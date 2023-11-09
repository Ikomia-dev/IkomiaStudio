// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CResultManager.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "DataProcessTools.hpp"
#include "IO/CImageIO.h"
#include "IO/CGraphicsOutput.h"
#include "IO/CBlobMeasureIO.h"
#include "IO/CNumericIO.h"
#include "IO/CVideoIO.h"
#include "IO/CWidgetOutput.h"
#include "IO/CDatasetIO.h"
#include "IO/CObjectDetectionIO.h"
#include "IO/CInstanceSegIO.h"
#include "IO/CSemanticSegIO.h"
#include "IO/CKeypointsIO.h"
#include "IO/CTextIO.h"
#include "IO/CJsonIO.h"
#include "IO/CScene3dIO.h"
#include "Model/Project/CProjectManager.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Model/Results/CResultItem.hpp"
#include "Model/Render/CRenderManager.h"
#include "Model/Data/CMainDataManager.h"
#include "Model/Data/CFeaturesTableModel.h"
#include "Model/Data/CMultiImageModel.h"
#include <QMessageBox>
#include "Graphics/CPoint.hpp"

CResultManager::CResultManager()
{
    connect(this, &CResultManager::doVideoSaveIsFinished, this, &CResultManager::onVideoSaveIsFinished);
}

CResultManager::~CResultManager()
{
    clearTableModels();
}

void CResultManager::init()
{
    try
    {
        createCustomMeasureTable();
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::setManagers(CProjectManager *pProjectMgr, CWorkflowManager* pWorkflowMgr, CGraphicsManager* pGraphicsMgr,
                                 CRenderManager *pRenderMgr, CMainDataManager *pDataMgr, CProgressBarManager *pProgressMgr)
{
    m_pProjectMgr = pProjectMgr;
    m_pWorkflowMgr = pWorkflowMgr;
    m_pGraphicsMgr = pGraphicsMgr;
    m_pRenderMgr = pRenderMgr;
    m_pProgressMgr = pProgressMgr;
    m_pDataMgr = pDataMgr;
}

void CResultManager::setCurrentResult(const QModelIndex &index)
{
    if(index.isValid() == false)
    {
        qCCritical(logResults).noquote() << tr("Current result not set: invalid index");
        return;
    }

    setResultHighlighted(m_currentResultIndex, false);
    setResultHighlighted(index, true);
    m_currentResultIndex = index;
}

//Display 2D image output from 3D volume
void CResultManager::setCurrentOutputImage(const QModelIndex& index)
{
    assert(m_pWorkflowMgr);

    if(m_bWorkflowInProgress == false)
        return;

    DimensionIndices indices = CProjectUtils::getIndicesInDataset(m_pProjectMgr->wrapIndex(index));
    auto currentImgIndex = Utils::Data::getDimensionSize(indices, DataDimension::IMAGE);
    auto pTask = m_pWorkflowMgr->getActiveTask();
    int volumeIndex = 0;

    for(size_t i=0; i<pTask->getOutputCount(); ++i)
    {
        auto dataType = pTask->getOutputDataType(i);
        if((dataType == IODataType::VOLUME || 
            dataType == IODataType::VOLUME_BINARY ||
            dataType == IODataType::POSITION) &&
            pTask->getOutput(i)->isDataAvailable())
        {
            auto pOut = std::dynamic_pointer_cast<CImageIO>(pTask->getOutput(i));
            if(pOut)
            {
                pOut->setCurrentImage(currentImgIndex);
                //Emit signal to display result
                emit doDisplayImage(volumeIndex++, CDataConversion::CMatToQImage(pOut->getImage()), QString::fromStdString(pTask->getName()), pTask->getOutputViewProperty(i));
            }
        }
    }
}

QModelIndex CResultManager::getRootIndex() const
{
    assert(m_pProjectMgr);
    return m_pProjectMgr->getImageSubTreeRootIndex(TreeItemType::RESULT);
}

void CResultManager::manageOutputs(const WorkflowTaskPtr &taskPtr, const WorkflowVertex& taskId, const QModelIndex& itemIndex)
{
    CPyEnsureGIL gil;
    m_bWorkflowInProgress = true;
    size_t outputCount = taskPtr->getOutputCount();
    clearPreviousOutputs();

    if(m_pCurrentTask != taskPtr || m_currentInputIndex != itemIndex || m_currentOutputCount != outputCount)
    {
        m_pCurrentTask = taskPtr;
        m_currentInputIndex = itemIndex;
        m_currentOutputCount = (int)outputCount;
        //Prepare view according to output data types
        auto outDisplays = getOutputDisplays(taskPtr);
        emit doInitDisplay(outDisplays);
    }

    try
    {
        int imageIndex = 0;
        int videoIndex = 0;
        int volumeIndex = 0;
        int widgetIndex = 0;
        int tableIndex = 0;
        int plotIndex = 0;
        int textIndex = 0;
        clearTableModels();
        auto globalViewMode = getViewMode(taskPtr);

        //Handle each output
        for(size_t i=0; i<outputCount; ++i)
        {
            auto outputPtr = taskPtr->getOutput(i);
            auto pOutputViewProp = taskPtr->getOutputViewProperty(i);
            pOutputViewProp->setViewMode(globalViewMode);

            if(outputPtr->isDataAvailable() && outputPtr->isDisplayable())
            {
                switch(outputPtr->getDataType())
                {
                    case IODataType::IMAGE:
                    case IODataType::IMAGE_BINARY:
                    case IODataType::IMAGE_LABEL:
                    case IODataType::POSITION:
                        manageImageOutput(outputPtr, taskPtr->getName(), imageIndex++, pOutputViewProp);
                        break;

                    case IODataType::VIDEO:
                    case IODataType::VIDEO_BINARY:
                    case IODataType::VIDEO_LABEL:
                    case IODataType::LIVE_STREAM:
                    case IODataType::LIVE_STREAM_BINARY:
                    case IODataType::LIVE_STREAM_LABEL:
                    {
                        //Get video inputs index to synchronize views
                        std::set<IODataType> types = {IODataType::VIDEO, IODataType::VIDEO_BINARY, IODataType::VIDEO_LABEL,
                                                      IODataType::LIVE_STREAM, IODataType::LIVE_STREAM_BINARY, IODataType::LIVE_STREAM_LABEL};
                        std::vector<int> videoInputIndices = m_pWorkflowMgr->getDisplayedInputIndices(taskPtr, types);
                        manageVideoOutput(taskPtr, outputPtr, videoIndex++, videoInputIndices, pOutputViewProp);
                        break;
                    }

                    case IODataType::VOLUME:
                    case IODataType::VOLUME_BINARY:
                    case IODataType::VOLUME_LABEL:
                        manageVolumeOutput(outputPtr, taskPtr->getName(), volumeIndex++, pOutputViewProp);
                        break;

                    case IODataType::BLOB_VALUES:
                        manageBlobOutput(outputPtr, taskPtr->getName(), tableIndex++, pOutputViewProp);
                        break;

                    case IODataType::NUMERIC_VALUES:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CNumericIOBase>(outputPtr);
                        assert(outPtr);
                        auto outType = outPtr->getOutputType();

                        if(outType == NumericOutputType::TABLE)
                            manageTableOutput(outputPtr, taskPtr->getName(), tableIndex++, pOutputViewProp);
                        else if (outType == NumericOutputType::PLOT)
                            managePlotOutput(outputPtr, taskPtr->getName(), plotIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::OUTPUT_GRAPHICS:
                        if(m_pWorkflowMgr->isRoot(taskId) == false)
                            manageGraphicsOutput(taskPtr, outputPtr);
                        break;

                    case IODataType::WIDGET:
                        manageWidgetOutput(outputPtr, taskPtr->getName(), widgetIndex++, pOutputViewProp);
                        break;

                    case IODataType::DNN_DATASET:
                        manageDatasetOutput(outputPtr, taskPtr->getName(), pOutputViewProp);
                        break;

                    case IODataType::DATA_DICT:
                        manageTextOutput(outputPtr, taskPtr->getName(), textIndex++, pOutputViewProp);
                        break;

                    case IODataType::OBJECT_DETECTION:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CObjectDetectionIO>(outputPtr);
                        assert(outPtr);
                        manageGraphicsOutput(taskPtr, outPtr->getGraphicsIO());
                        manageBlobOutput(outPtr->getBlobMeasureIO(), taskPtr->getName(), tableIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::INSTANCE_SEGMENTATION:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CInstanceSegIO>(outputPtr);
                        assert(outPtr);
                        manageImageOutput(outPtr->getMaskImageIO(), taskPtr->getName(), imageIndex++, pOutputViewProp);
                        manageGraphicsOutput(taskPtr, outPtr->getGraphicsIO());
                        manageBlobOutput(outPtr->getBlobMeasureIO(), taskPtr->getName(), tableIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::SEMANTIC_SEGMENTATION:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CSemanticSegIO>(outputPtr);
                        assert(outPtr);
                        manageImageOutput(outPtr->getMaskImageIO(), taskPtr->getName(), imageIndex++, pOutputViewProp);
                        manageImageOutput(outPtr->getLegendImageIO(), taskPtr->getName(), imageIndex++, pOutputViewProp);
                        manageGraphicsOutput(taskPtr, outPtr->getGraphicsIO());
                        break;
                    }

                    case IODataType::KEYPOINTS:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CKeypointsIO>(outputPtr);
                        assert(outPtr);
                        manageGraphicsOutput(taskPtr, outPtr->getGraphicsIO());
                        manageBlobOutput(outPtr->getBlobMeasureIO(), taskPtr->getName(), tableIndex++, pOutputViewProp);
                        manageTableOutput(outPtr->getDataStringIO(), taskPtr->getName(), tableIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::TEXT:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CTextIO>(outputPtr);
                        assert(outPtr);
                        manageGraphicsOutput(taskPtr, outPtr->getGraphicsIO());
                        manageTableOutput(outPtr->getDataStringIO(), taskPtr->getName(), tableIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::JSON:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CJsonIO>(outputPtr);
                        assert(outPtr);
                        manageJsonOutput(outputPtr, taskPtr->getName(), textIndex++, pOutputViewProp);
                        break;
                    }

                    case IODataType::SCENE_3D:
                    {
                        auto outPtr = std::dynamic_pointer_cast<CScene3dIO>(outputPtr);
                        assert(outPtr);
                        manageScene3dOutput(outputPtr, taskPtr->getName(), imageIndex++, pOutputViewProp);
                        break;
                    }

                    default: break;
                }
            }
        }
        //Apply the view properties for all outputs (maximized status, zoom...)
        emit doApplyViewProperty();
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::loadImageResults(const QModelIndex &index)
{
    if(index != m_currentImgIndex)
    {
        try
        {
            m_currentImgIndex = index;
            m_currentResultIndex = QModelIndex();
            auto wrapIndex = m_pProjectMgr->wrapIndex(index);
            auto imgItemPtr = CProjectUtils::getImageItem(wrapIndex);

            if(imgItemPtr)
            {
                //Get results root
                QModelIndex rootResultsIndex;
                auto pMultiModel = m_pProjectMgr->getMultiModel();
                int childCount = pMultiModel->rowCount(index);

                for(int i=0; i<childCount && !rootResultsIndex.isValid(); ++i)
                {
                    auto indexTmp = pMultiModel->index(i, 0, index);
                    auto pChild = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(indexTmp).internalPointer());

                    if(pChild->getTypeId() == TreeItemType::RESULT)
                        rootResultsIndex = indexTmp;
                }

                if(rootResultsIndex.isValid())
                {
                    //Get first leaf
                    QModelIndex resIndex = rootResultsIndex;
                    //Force multimodel initialization of non visible childs due to lazy evaluation
                    childCount = pMultiModel->rowCount(resIndex);
                    QModelIndex childIndex = pMultiModel->index(0, 0, resIndex);

                    while(childIndex.isValid())
                    {
                        resIndex = childIndex;
                        //Force multimodel initialization of non visible childs due to lazy evaluation
                        childCount = pMultiModel->rowCount(resIndex);
                        childIndex = pMultiModel->index(0, 0, resIndex);
                    }
                    loadResults(resIndex);
                }
                else
                    emit doHideResultsView();
            }
        }
        catch(std::exception& e)
        {
            qCCritical(logResults).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CResultManager::notifyProjectSaved(int projectIndex)
{
    auto it = m_removedResults.find(projectIndex);
    if(it != m_removedResults.end())
    {
        try
        {
            //Remove orphans results items directly in database
            CProjectDbManager projectDB(m_pProjectMgr->getModel(projectIndex));
            CResultDbManager resultDB(projectDB.getPath(), projectDB.getConnectionName());
            resultDB.remove(it->second);
        }
        catch(std::exception& e)
        {
            qCCritical(logResults).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CResultManager::notifyBeforeProjectClosed(int projectIndex, bool bWithCurrentImage)
{
    auto it = m_removedResults.find(projectIndex);
    if(it != m_removedResults.end())
        m_removedResults.erase(it);

    if(bWithCurrentImage == true)
        clearPreviousOutputs();
}

void CResultManager::notifyBeforeWorkflowCleared()
{
    clearPreviousOutputs();
}

void CResultManager::notifyDisplaySelected(int index)
{
    if(m_pCurrentTask == nullptr)
        return;

    WorkflowTaskIOPtr outputPtr = m_pCurrentTask->getOutput(index);
    if(outputPtr == nullptr)
        return;

    if(index == m_selectedIndex)
        return;

    switch(outputPtr->getDataType())
    {
        case IODataType::VOLUME:
        case IODataType::VOLUME_BINARY:
        case IODataType::VOLUME_LABEL:
            updateVolumeRender(outputPtr);
            break;

        default: break;
    }
    m_selectedIndex = index;
}

void CResultManager::onWorkflowClosed()
{
    CPyEnsureGIL gil;
    m_pCurrentTask = nullptr;
    m_bWorkflowInProgress = false;
    clearPreviousOutputs();
    emit doClearResultsView();
}

void CResultManager::onRecordResultVideo(size_t index, bool bRecord)
{
    try
    {
        setRecordVideoState(index, bRecord);

        if(bRecord)
            startRecordVideo();
        else
            stopRecordVideo();
    }
    catch(std::exception& e)
    {
        // Set current video record name as empty to avoid item creation in tree
        m_currentVideoRecord = "";
        // Unset record video state
        setRecordVideoState(index, !bRecord);
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
        emit doStopRecording(index);
    }
}

void CResultManager::onVideoSaveIsFinished(QStringList &paths, CDataVideoBuffer::Type sourceType)
{
    //Récupération du chemin de l'image source
    QModelIndex imgIndex = m_pWorkflowMgr->getCurrentVideoInputModelIndex();
    if(!imgIndex.isValid())
    {
        qCCritical(logResults).noquote() << tr("Error while saving result video: invalid index");
        return;
    }

    //Création d'un dataset dans le projet
    auto currentDatasetIndex = m_pProjectMgr->getDatasetIndex(imgIndex);

    //Ajout de l'image résultat
    if(sourceType == CDataVideoBuffer::IMAGE_SEQUENCE)
    {
        DatasetLoadPolicy policy(Relationship::MANY_TO_ONE, DataDimension::TIME);
        m_pProjectMgr->addImages(currentDatasetIndex.parent(), paths, policy);
    }
    else
        m_pProjectMgr->addVideos(currentDatasetIndex.parent(), paths);
}

void CResultManager::removeResult(const QModelIndex &index)
{
    //Clear result view if current result item or its parent will be removed
    if(index == m_currentResultIndex || isParentIndex(m_currentResultIndex, index))
        emit doClearResultsView();

    //We need to store ids of removed results to really remove them if project is saved
    std::vector<int> resultIds;
    fillResultTreeIds(index, resultIds);

    auto it = m_removedResults.find((int)m_pProjectMgr->getProjectIndex(index));
    if(it != m_removedResults.end())
        it->second.insert(it->second.end(), resultIds.begin(), resultIds.end());
    else
        m_removedResults.insert(std::make_pair((int)m_pProjectMgr->getProjectIndex(index), resultIds));

    m_pProjectMgr->getMultiModel()->removeItem(index);
}

void CResultManager::onSaveCurrentImage(int index)
{
    assert(m_pProjectMgr && m_pWorkflowMgr);

    //Sauvegarde de l'image résultat
    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving result image: invalid current task");
        return;
    }

    try
    {
        auto outputPtr = pTask->getOutput(index);
        if(!outputPtr)
        {
            qCCritical(logResults).noquote() << tr("Error while saving result image: invalid output #%1").arg(index + 1);
            return;
        }

        outputPtr->setSaveInfo(pTask->getOutputFolder(), pTask->getName());
        std::string path = outputPtr->getSavePath();
        Utils::File::createDirectory(Utils::File::getParentPath(path));
        saveOutputImage(index, path, false);

        //Création d'un dataset dans le projet
        QModelIndex currentIndex = m_pProjectMgr->getCurrentDataItemIndex();
        std::string newItemName = currentIndex.data(Qt::DisplayRole).toString().toStdString();
        auto currentDatasetIndex = m_pProjectMgr->getDatasetIndex(currentIndex);
        auto datasetIndex = m_pProjectMgr->addDataset(currentDatasetIndex.parent(), newItemName, IODataType::IMAGE);

        //Ajout de l'image résultat
        QStringList files;
        files.push_back(QString::fromStdString(path));
        m_pProjectMgr->addImagesToDataset(datasetIndex, files);

        emit doNewResultNotification(QString("Result image has been saved and added to the current project."), Notification::INFO);
    }
    catch(std::exception& e)
    {
       qCCritical(logResults).noquote() << QString::fromStdString(e.what());
       return;
    }
}

void CResultManager::onExportCurrentImage(int index, const QString &path, bool bWithGraphics)
{
    try
    {
        saveOutputImage(index, path.toStdString(), bWithGraphics);
        emit doNewResultNotification(QString("Result image has been exported."), Notification::INFO);
    }
    catch(std::exception& e)
    {
       qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::onSaveCurrentVideo(size_t index)
{
    assert(m_pProjectMgr && m_pWorkflowMgr);

    // Save video after launching workflow
    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving result video: invalid current task");
        return;
    }

    try
    {
        auto outputPtr = pTask->getOutput(index);
        if(!outputPtr)
        {
            qCCritical(logResults).noquote() << tr("Error while saving result image: invalid output #%1").arg(index + 1);
            return;
        }

        outputPtr->setSaveInfo(pTask->getOutputFolder(), pTask->getName());
        std::string path = outputPtr->getSavePath();
        Utils::File::createDirectory(Utils::File::getParentPath(path));
        runWorkflowAndSaveVideo(index, path, false);
    }
    catch(std::exception& e)
    {
       qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::onExportCurrentVideo(size_t id, const QString& path, bool bWithGraphics)
{
    try
    {
        runWorkflowAndSaveVideo(id, path.toStdString(), bWithGraphics);
    }
    catch(std::exception& e)
    {
       qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::onSaveTableData(int index)
{
    if(index < 0 || index >= (int)m_tableModels.size())
    {
        qCCritical(logResults).noquote() << "Table data cannot be saved: invalid index";
        return;
    }

    auto pModel = m_tableModels[index];
    if(typeid(*pModel) == typeid(CMeasuresTableModel))
    {
        saveOutputMeasures(index);
        saveOutputGraphics();
        emit doNewResultNotification(QString("Result table has been saved."), Notification::INFO);
    }
}

void CResultManager::onExportTableData(int index, const QString &path)
{
    if(index < 0 || index >= (int)m_tableModels.size())
    {
        qCCritical(logResults).noquote() << "Table data cannot be exported: invalid index";
        return;
    }

    try
    {
        auto pModel = m_tableModels[index];
        if(typeid(*pModel) == typeid(CMeasuresTableModel))
            saveOutputMeasures(index, path.toStdString());
        else if(typeid(*pModel) == typeid(CFeaturesTableModel))
            saveOutputFeatures(index, path.toStdString());
        emit doNewResultNotification(QString("Result table has been exported."), Notification::INFO);
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::onExportDatasetImage(const QString &path, CMat &img, CGraphicsLayer *pLayer)
{
    if(pLayer)
        m_pGraphicsMgr->burnLayerToImage(pLayer, img);

    CImageDataIO io(path.toStdString());
    io.write(img);
}

void CResultManager::displayResult(const QModelIndex &index)
{
    assert(m_pProjectMgr);

    if(index.isValid() == false)
    {
        qCCritical(logResults).noquote() << tr("Error while loading results: image index not valid");
        return;
    }

    try
    {
        if(isResultFromCurrentImage(index))
        {
            //Display only task results (=leaf)
            QModelIndex tmpIndex = index;
            auto pMultiModel = m_pProjectMgr->getMultiModel();

            while(pMultiModel->rowCount(tmpIndex) != 0)
                tmpIndex = pMultiModel->index(0, 0, tmpIndex);

            if(tmpIndex != m_currentResultIndex)
                loadResults(tmpIndex);
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << e.what();
    }
}

void CResultManager::createCustomMeasureTable()
{
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid())
    {
        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        //Table MeasuresInfo
        if(tables.contains("customMeasureInfo") == false)
        {
            if(!q.exec("CREATE TABLE customMeasureInfo (id INTEGER PRIMARY KEY, measureName TEXT NOT NULL, measureFormula TEXT);"))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }
}

QModelIndex CResultManager::createRootResult()
{
    assert(m_currentImgIndex.isValid());
    auto pMultiProject = m_pProjectMgr->getMultiModel();
    assert(pMultiProject);
    auto pResultItem = std::make_shared<CResultItem>(tr("Results").toStdString(), CResultItem::NodeType::ROOT);
    return pMultiProject->addItem(m_currentImgIndex, pResultItem);
}

void CResultManager::setResultHighlighted(const QModelIndex &index, bool bHighlighted)
{
    if(index.isValid())
    {
        auto itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(index).internalPointer());
        if(itemPtr && itemPtr->getTypeId() == TreeItemType::RESULT)
            itemPtr->setHighlighted(bHighlighted);
    }
}

void CResultManager::setRecordVideoState(size_t id, bool bRecord)
{
    auto it = m_recordVideoMap.find(id);
    if(it != m_recordVideoMap.end())
        it->second = bRecord;
    else
        m_recordVideoMap.insert(std::make_pair(id, bRecord));
}

ResultItemPtr CResultManager::getResultItem(const QModelIndex &index) const
{
    assert(m_pProjectMgr);

    if(index.isValid() == false)
        return nullptr;

    auto pItem = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(index).internalPointer());
    if(!pItem)
        return nullptr;

    if(pItem->getTypeId() != TreeItemType::RESULT)
        return nullptr;

    return pItem->getNode<ResultItemPtr>();
}

OutputDisplays CResultManager::getOutputDisplays(const WorkflowTaskPtr &pTask) const
{
    OutputDisplays outDisplays;
    try
    {
        for(size_t i=0; i<pTask->getOutputCount(); ++i)
        {
            auto outputPtr = pTask->getOutput(i);
            if(outputPtr->isDataAvailable() == true)
            {
                DisplayType viewType;
                auto type = outputPtr->getDataType();
                auto pViewProp = pTask->getOutputViewProperty(i);

                if(type == IODataType::IMAGE &&
                        (pTask->hasInput(IODataType::VIDEO) ||
                         pTask->hasInput(IODataType::VIDEO_BINARY) ||
                         pTask->hasInput(IODataType::VIDEO_LABEL)))
                {
                    viewType = getResultViewType(IODataType::VIDEO);
                }
                else
                    viewType = getResultViewType(type);

                auto it = outDisplays.find(viewType);
                if(it == outDisplays.end())
                    outDisplays.insert(std::make_pair(viewType, std::vector<CViewPropertyIO*>(1, pViewProp)));
                else
                    it->second.push_back(pViewProp);
            }
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
    return outDisplays;
}

DisplayType CResultManager::getResultViewType(IODataType type) const
{
    DisplayType viewType = DisplayType::EMPTY_DISPLAY;

    switch(type)
    {
        case IODataType::INPUT_GRAPHICS:
        case IODataType::OUTPUT_GRAPHICS: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::IMAGE: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::IMAGE_BINARY: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::IMAGE_LABEL: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::NONE: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::BLOB_VALUES: viewType = DisplayType::TABLE_DISPLAY; break;
        case IODataType::NUMERIC_VALUES: viewType = DisplayType::PLOT_DISPLAY; break;
        case IODataType::VOLUME: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::VOLUME_BINARY: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::VOLUME_LABEL: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::POSITION: viewType = DisplayType::IMAGE_DISPLAY; break;
        case IODataType::VIDEO: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::VIDEO_BINARY: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::VIDEO_LABEL: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::LIVE_STREAM: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::LIVE_STREAM_BINARY: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::LIVE_STREAM_LABEL: viewType = DisplayType::VIDEO_DISPLAY; break;
        case IODataType::WIDGET: viewType = DisplayType::WIDGET_DISPLAY; break;
        case IODataType::DESCRIPTORS: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::PROJECT_FOLDER: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::FOLDER_PATH: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::FILE_PATH: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::DNN_DATASET: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::ARRAY: viewType = DisplayType::EMPTY_DISPLAY; break;
        case IODataType::DATA_DICT: viewType = DisplayType::TEXT_DISPLAY; break;
        case IODataType::OBJECT_DETECTION: viewType = DisplayType::EMPTY_DISPLAY; break;        //Composite
        case IODataType::INSTANCE_SEGMENTATION: viewType = DisplayType::EMPTY_DISPLAY; break;   //Composite
        case IODataType::SEMANTIC_SEGMENTATION: viewType = DisplayType::EMPTY_DISPLAY; break;   //Composite
        case IODataType::KEYPOINTS: viewType = DisplayType::EMPTY_DISPLAY; break;               //Composite
        case IODataType::TEXT: viewType = DisplayType::EMPTY_DISPLAY; break;                    //Composite
        case IODataType::JSON: viewType = DisplayType::JSON_DISPLAY; break;
        case IODataType::SCENE_3D: viewType = DisplayType::SCENE_3D_DISPLAY; break;
    }
    return viewType;
}

std::set<IODataType> CResultManager::getImageBasedDataTypes() const
{
    std::set<IODataType> dataTypes = {
        IODataType::IMAGE,
        IODataType::IMAGE_BINARY,
        IODataType::IMAGE_LABEL,
        IODataType::VOLUME,
        IODataType::VOLUME_BINARY,
        IODataType::VOLUME_LABEL,
        IODataType::POSITION,
        IODataType::VIDEO,
        IODataType::VIDEO_BINARY,
        IODataType::VIDEO_LABEL,
        IODataType::LIVE_STREAM,
        IODataType::LIVE_STREAM_BINARY,
        IODataType::LIVE_STREAM_LABEL,
        IODataType::INSTANCE_SEGMENTATION,
        IODataType::SEMANTIC_SEGMENTATION
    };
    return dataTypes;
}

CViewPropertyIO::ViewMode CResultManager::getViewMode(const WorkflowTaskPtr &taskPtr)
{
    CViewPropertyIO::ViewMode mode;
    if(taskPtr->isSelfInput())
        mode = CViewPropertyIO::ViewMode::OUTPUT_ONLY;
    else if(m_currentViewMode == CViewPropertyIO::ViewMode::OUTPUT_ONLY)
        mode = CViewPropertyIO::ViewMode::INPUT_OUTPUT;
    else
        mode = CViewPropertyIO::ViewMode::GUI_DRIVEN;

    m_currentViewMode = mode;
    return mode;
}

bool CResultManager::isResultFromCurrentImage(const QModelIndex &index) const
{
    //Retrieve image QModelIndex
    QModelIndex imageIndex = index.parent();
    auto itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(index).parent().internalPointer());

    while(itemPtr->getTypeId() != TreeItemType::IMAGE)
    {
        imageIndex = imageIndex.parent();
        itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(imageIndex).internalPointer());
    }
    return imageIndex == m_currentImgIndex;
}

bool CResultManager::isParentIndex(const QModelIndex &index, const QModelIndex &parent) const
{
    assert(m_pProjectMgr);

    if(index.isValid() == false || parent.isValid() == false)
        return false;

    bool bSearch = true;
    QModelIndex indexTmp = index;

    while(bSearch == true)
    {
        auto parentTmp =  indexTmp.parent();
        auto itemPtr = static_cast<ProjectTreeItem*>(m_pProjectMgr->wrapIndex(indexTmp).internalPointer());

        if(parentTmp == parent)
            return true;
        else
        {
            indexTmp = parentTmp;
            bSearch = (itemPtr->getTypeId() == TreeItemType::RESULT);
        }
    }
    return false;
}

void CResultManager::clearOverlay()
{
    if(m_bImageOverlay == true)
    {
        m_bImageOverlay = false;
        emit doClearOverlay();
    }
}

void CResultManager::clearGraphics()
{
    assert(m_pGraphicsMgr);

    if(m_tempGraphicsLayerInfo.m_pLayer)
    {
        m_pGraphicsMgr->removeTemporaryLayer(m_tempGraphicsLayerInfo);
        m_tempGraphicsLayerInfo.deleteLayer();
    }
}

void CResultManager::clearPreviousOutputs()
{
    clearOverlay();
    clearGraphics();

    if(m_bBinaryVolume == true)
    {
        m_bBinaryVolume = false;
        m_pRenderMgr->clearBinary();
    }
}

void CResultManager::clearTableModels()
{
    for(size_t i=0; i<m_tableModels.size(); ++i)
        m_tableModels[i]->deleteLater();

    m_tableModels.clear();
}

void CResultManager::manageImageOutput(const WorkflowTaskIOPtr &pOutput, const std::string& taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(pOutput);

    auto pOut = std::dynamic_pointer_cast<CImageIO>(pOutput);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid image");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid image buffer");
        return;
    }


    CMat image;
    if(pOut->getDataType() == IODataType::IMAGE_LABEL)
    {
        CDataConversion::grayscaleToAlpha(pOut->getImage(), image);
    }
    else if(pOut->getDataType() == IODataType::POSITION)
    {
        image = pOut->getData().getPlane(index);
    }
    else
    {
        image = pOut->getImage();
    }

    //Emit signal to display result
    emit doDisplayImage(index, CDataConversion::CMatToQImage(image), QString::fromStdString(taskName), pViewProp);

    //Emit signal to display overlay binary
    if(pOut->isOverlayAvailable() == true)
    {
        emit doDisplayOverlay(CDataConversion::CMatToQImage(pOut->getOverlayMask()), index, DisplayType::IMAGE_DISPLAY);
        m_bImageOverlay = true;
    }
}

void CResultManager::manageVolumeOutput(const WorkflowTaskIOPtr &outputPtr, const std::string &taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(m_pRenderMgr);
    assert(outputPtr);

    auto pOut = std::dynamic_pointer_cast<CImageIO>(outputPtr);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid volume");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid volume buffer");
        return;
    }

    try
    {
        DimensionIndices indices = CProjectUtils::getIndicesInDataset(m_pProjectMgr->wrapIndex(m_currentInputIndex));
        auto currentImgIndex = Utils::Data::getDimensionSize(indices, DataDimension::IMAGE);
        pOut->setCurrentImage(currentImgIndex);

        //Emit signal to display result
        emit doDisplayImage(index, CDataConversion::CMatToQImage(pOut->getImage()), QString::fromStdString(taskName), pViewProp);

        //Update 3D scene rendering, only for first 3D output
        if(index == 0)
        {
            updateVolumeRender(outputPtr);
            m_selectedIndex = index;
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::manageGraphicsOutput(const WorkflowTaskPtr &taskPtr, const WorkflowTaskIOPtr &pOutput)
{
    assert(m_pGraphicsMgr && pOutput);

    auto pOut = std::dynamic_pointer_cast<CGraphicsOutput>(pOutput);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid graphics");
        return;
    }

    //Clear previous graphics
    clearGraphics();
    assert(m_tempGraphicsLayerInfo.m_pLayer == nullptr);

    //Display graphics layer
    m_tempGraphicsLayerInfo.m_pLayer = pOut->createLayer(m_pGraphicsMgr->getContext());
    m_tempGraphicsLayerInfo.m_refImageIndex = pOut->getImageIndex();
    m_tempGraphicsLayerInfo.m_refImageType = getResultViewType(taskPtr->getOutputDataType(pOut->getImageIndex()));
    m_tempGraphicsLayerInfo.m_displayTarget = CGraphicsLayerInfo::RESULT;
    m_tempGraphicsLayerInfo.m_bTopMost = false;
    m_pGraphicsMgr->addTemporaryLayer(m_tempGraphicsLayerInfo);
}

void CResultManager::manageBlobOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(pOutput);

    auto pOut = std::dynamic_pointer_cast<CBlobMeasureIO>(pOutput);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid measures");
        return;
    }

    try
    {
        CResultDbManager resultDb;
        resultDb.setMeasures(pOut->getMeasures());
        auto pModel = resultDb.createMeasureModel();
        m_tableModels.push_back(pModel);
        emit doDisplayMeasuresTable(index, QString::fromStdString(taskName), pModel, pViewProp);
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::manageTableOutput(const WorkflowTaskIOPtr& pOutput, const std::string& taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(pOutput);
    auto pOut = std::dynamic_pointer_cast<CNumericIOBase>(pOutput);
    auto pModel = new CFeaturesTableModel(this);
    m_tableModels.push_back(pModel);
    pModel->insertData(pOut->getAllValuesAsString(), pOut->getAllValueLabels(), pOut->getAllHeaderLabels());
    emit doDisplayFeaturesTable(index, QString::fromStdString(taskName), pModel, pViewProp);
}

void CResultManager::managePlotOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp)
{
    assert(pOutput);
    auto pOut = std::dynamic_pointer_cast<CNumericIO<double>>(pOutput);

    if(pOut == nullptr)
    {
        qCritical(logResults).noquote() << tr("Wrong data format for plot output, type double expected");
        return;
    }

    CDataPlot::Type plotType;
    auto type = pOut->getPlotType();
    switch(type)
    {
        case PlotType::CURVE: plotType = CDataPlot::Type::Curve; break;
        case PlotType::BAR: plotType = CDataPlot::Type::Bar; break;
        case PlotType::MULTIBAR: plotType = CDataPlot::Type::MultiBar; break;
        case PlotType::HISTOGRAM: plotType = CDataPlot::Type::Histogram; break;
        case PlotType::PIE: plotType = CDataPlot::Type::Pie; break;
        default: plotType = CDataPlot::Type::Curve; break;
    }
    CDataPlot* pPlot = new CDataPlot;
    pPlot->setType(plotType);
    pPlot->setValueList(pOut->getAllValues());
    pPlot->setStringList(pOut->getAllValueLabels());
    emit doDisplayPlot(index, QString::fromStdString(taskName), pPlot, pViewProp);
}

void CResultManager::manageVideoOutput(const WorkflowTaskPtr &taskPtr, const WorkflowTaskIOPtr& pOutput, int index, const std::vector<int>& videoInputIndices, CViewPropertyIO* pViewProp)
{
    assert(pOutput);

    auto pOut = std::dynamic_pointer_cast<CVideoIO>(pOutput);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid image");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid image buffer");
        return;
    }

    CMat image;
    if(pOut->getDataType() == IODataType::VIDEO_LABEL || pOut->getDataType() == IODataType::LIVE_STREAM_LABEL)
        CDataConversion::grayscaleToAlpha(pOut->getImage(), image);
    else
        image = pOut->getImage();

    try
    {
        manageVideoRecord(taskPtr, index, image);
    }
    catch (std::exception& e)
    {
        qCCritical(logVideo).noquote() << QString::fromStdString(e.what());
    }

    //Emit signal to display result
    emit doDisplayVideo(index, CDataConversion::CMatToQImage(image), QString::fromStdString(taskPtr->getName()), videoInputIndices, pViewProp);
    //Get source video info
    auto videoInfoPtr = m_pDataMgr->getVideoMgr()->getVideoInfo(m_pWorkflowMgr->getCurrentVideoInputModelIndex());
    auto srcType = m_pDataMgr->getVideoMgr()->getSourceType(m_pWorkflowMgr->getCurrentVideoInputModelIndex());

    //Emit signals to initialize video info (fps...)
    if(videoInfoPtr)
    {
        emit doSetVideoSourceType(index, srcType);
        emit doSetVideoFPS(index, videoInfoPtr->m_fps);
        emit doSetVideoLength(index, videoInfoPtr->m_frameCount);
        emit doSetVideoPos(index, videoInfoPtr->m_currentPos);

        if(videoInfoPtr->m_fps != 0)
        {
            emit doSetVideoTotalTime(index, videoInfoPtr->m_frameCount / videoInfoPtr->m_fps);
            emit doSetVideoCurrentTime(index, videoInfoPtr->m_currentPos / videoInfoPtr->m_fps);
        }
    }

    //Emit signal to display overlay binary
    if(pOut->isOverlayAvailable() == true)
    {
        emit doDisplayOverlay(CDataConversion::CMatToQImage(pOut->getOverlayMask()), index, DisplayType::VIDEO_DISPLAY);
        m_bImageOverlay = true;
    }
}

void CResultManager::manageWidgetOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(pOutput);
    Q_UNUSED(taskName);

    auto pOut = std::dynamic_pointer_cast<CWidgetOutput>(pOutput);
    if(pOut == nullptr)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid output type");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid widget");
        return;
    }
    emit doAddResultWidget(index, pOut->getWidget(), false, pViewProp);
}

void CResultManager::manageVideoRecord(const WorkflowTaskPtr &taskPtr, size_t index, const CMat& image)
{
    if(m_recordVideoMap.empty() || index >= m_recordVideoMap.size())
        return;

    if(m_recordVideoMap[index])
    {
        CMat tmp;
        if(image.channels() == 1)
            cv::cvtColor(image, tmp, cv::COLOR_GRAY2BGR);
        else
            cv::cvtColor(image, tmp, cv::COLOR_RGB2BGR);

        auto graphicsOutputs = taskPtr->getOutputs({IODataType::OUTPUT_GRAPHICS});
        if(graphicsOutputs.size() > 0)
        {
            for(size_t i=0; i<graphicsOutputs.size(); ++i)
            {
                auto graphicsOutPtr = std::static_pointer_cast<CGraphicsOutput>(graphicsOutputs[i]);
                if(graphicsOutPtr->getImageIndex() == (int)index)
                {
                    CGraphicsConversion graphicsConv((int)tmp.getNbCols(), (int)tmp.getNbRows());
                    for(auto it : graphicsOutPtr->getItems())
                        it->insertToImage(tmp, graphicsConv, false, false, true);
                }
            }
        }
        m_pVideoMgr->write(tmp);
    }
}

void CResultManager::manageDatasetOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, CViewPropertyIO *pViewProp)
{
    assert(pOutput);

    auto pOut = std::dynamic_pointer_cast<CDatasetIO>(pOutput);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid dataset");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: no data available");
        return;
    }

    if(m_pMultiImgModel)
        delete m_pMultiImgModel;

    m_pMultiImgModel = new CMultiImageModel(this);
    m_pMultiImgModel->setGraphicsContext(m_pGraphicsMgr->getContext());
    auto paths = pOut->getImagePaths();

    for(size_t i=0; i<paths.size(); ++i)
    {
        CPyEnsureGIL gil;
        auto graphics = pOut->getGraphicsAnnotations(paths[i]);
        auto maskPath = pOut->getMaskPath(paths[i]);
        m_pMultiImgModel->addImage(QString::fromStdString(paths[i]), QString::fromStdString(maskPath), graphics);
    }
    emit doDisplayDnnDataset(m_pMultiImgModel, QString::fromStdString(taskName), pViewProp);
}

void CResultManager::manageTextOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp)
{
    if(pOutput->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: no data available");
        return;
    }
    std::vector<std::string> options =   {"json_format", "indented"};
    emit doDisplayText(index, QString::fromStdString(pOutput->toJson(options)), QString::fromStdString(taskName), pViewProp);
}

void CResultManager::manageJsonOutput(const WorkflowTaskIOPtr &pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp)
{
    auto pOut = std::dynamic_pointer_cast<CJsonIO>(pOutput);

    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid JSON data");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: no JSON data available");
        return;
    }

    try
    {
        // WARNING: don't use 'toJson()' methods because they return 'std::string' instead of 'QJsonDocument'
        emit doDisplayJson(index, pOut->getData(), QString::fromStdString(taskName), pViewProp);
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::manageScene3dOutput(const WorkflowTaskIOPtr &outputPtr, const std::string &taskName, int index, CViewPropertyIO* pViewProp)
{
    assert(m_pRenderMgr);
    assert(outputPtr);

    auto pOut = std::dynamic_pointer_cast<CScene3dIO>(outputPtr);
    if(!pOut)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid scene");
        return;
    }

    if(pOut->isDataAvailable() == false)
    {
        qCCritical(logResults).noquote() << tr("Process output management: invalid scene content");
        return;
    }

    try
    {
        emit doDisplayScene3d(pOut->getScene3d(), index, QString::fromStdString(taskName), pViewProp);
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

QModelIndex CResultManager::findResultFromName(const QString &name, QModelIndex startIndex) const
{
    if(!startIndex.isValid())
        startIndex = getRootIndex();

    auto pModel = static_cast<const CMultiProjectModel*>(startIndex.model());
    return pModel->findItemFromName(name.toStdString(), startIndex);
}

void CResultManager::fillResultTreeIds(const QModelIndex &index, std::vector<int> &ids)
{
    auto pResultItem = getResultItem(index);
    if(pResultItem)
    {
        ids.push_back(pResultItem->getDbId());

        //Iterate through all child layers
        auto pMultiProject = m_pProjectMgr->getMultiModel();
        int childCount = pMultiProject->rowCount(index);

        for(int i=0; i<childCount; ++i)
        {
            auto childIndex = pMultiProject->index(i, 0, index);
            fillResultTreeIds(childIndex, ids);
        }
    }
}

void CResultManager::runWorkflowAndSaveVideo(size_t id, const std::string& path, bool bWithGraphics)
{
    // Message box non blocking
    QMessageBox* pDlg = new QMessageBox;
    pDlg->setText(tr("The workflow is running in order to generate your video, please wait until it finishes."));
    auto pAbortBtn = pDlg->addButton(tr("Abort exporting"), QMessageBox::RejectRole);

    connect(pAbortBtn, &QPushButton::clicked, [&]
    {
        m_pWorkflowMgr->stopWorkflow();
    });
    pDlg->show();

    // Create a single shot connection between workflow manager finish and video saving
    QMetaObject::Connection* pConnSuccess = new QMetaObject::Connection();
    *pConnSuccess = connect(m_pWorkflowMgr, &CWorkflowManager::doWorkflowFinished, [this, path, pDlg, id, pConnSuccess]
    {
        try
        {
            // Reset process on whole video to go back on live processing
            m_pWorkflowMgr->setWorkflowConfig("GraphicsEmbedded", std::to_string(false));
            m_pWorkflowMgr->enableWholeVideo(false);
            m_pWorkflowMgr->setCurrentTaskAutoSave(id, false);
            // Move file to appropriate place
            saveOutputVideo(id, path);
        }
        catch(std::exception& e)
        {
            qCCritical(logResults).noquote() << QString::fromStdString(e.what());
        }
        // automatic close message box
        pDlg->close();

        //Disconnect one shot connection
        QObject::disconnect(*pConnSuccess);
        delete pConnSuccess;
    });

    // Create a single shot connection to handle workflow failures
    QMetaObject::Connection* pConnFailure = new QMetaObject::Connection();
    *pConnFailure = connect(m_pWorkflowMgr, &CWorkflowManager::doWorkflowFailed, [this, pDlg, id, pConnFailure, pConnSuccess]
    {
        // Reset process on whole video to go back on live processing
        m_pWorkflowMgr->setWorkflowConfig("GraphicsEmbedded", std::to_string(false));
        m_pWorkflowMgr->enableWholeVideo(false);
        m_pWorkflowMgr->setCurrentTaskAutoSave(id, false);
        // automatic close message box
        pDlg->close();
        //Disconnect one shot connections
        QObject::disconnect(*pConnSuccess);
        QObject::disconnect(*pConnFailure);
        delete pConnSuccess;
        delete pConnFailure;
    });

    //Get save format
    std::string ext = Utils::File::extension(path);
    auto format = Utils::Data::getFileFormatFromExtension(ext);

    // Set processing on whole video
    m_pWorkflowMgr->setWorkflowConfig("GraphicsEmbedded", std::to_string(bWithGraphics));
    m_pWorkflowMgr->setCurrentTaskAutoSave(id, true);
    m_pWorkflowMgr->setCurrentTaskSaveFormat(id, format);
    m_pWorkflowMgr->enableWholeVideo(true);
    // Run protocol to current active task
    m_pWorkflowMgr->runWorkflowToActiveTask();
}

void CResultManager::updateVolumeRender(const WorkflowTaskIOPtr &outputPtr)
{
    assert(m_pRenderMgr);

    auto outPtr = std::dynamic_pointer_cast<CImageIO>(outputPtr);
    if(outPtr == nullptr)
        return;

    if(outPtr->getDataType() == IODataType::VOLUME_BINARY)
    {
        m_bBinaryVolume = true;
        m_pRenderMgr->addBinary(outPtr->getData());
    }
    else
        m_pRenderMgr->updateVolumeRenderInput(outPtr->getData());
}

void CResultManager::saveOutputImage(int index, const std::string &path, bool bWithGraphics)
{
    assert(m_pWorkflowMgr);
    assert(index >= 0);

    auto taskPtr = m_pWorkflowMgr->getActiveTask();
    if(!taskPtr)
        throw CException(CoreExCode::NULL_POINTER, tr("Invalid current task").toStdString(), __func__, __FILE__, __LINE__);

    // Get image from possible outputs: composite outputs may have more than 1 CImageIO
    auto outputs = taskPtr->getOutputs(getImageBasedDataTypes());
    if(index >= (int)outputs.size())
        throw CException(CoreExCode::INVALID_PARAMETER, tr("Invalid output index").toStdString(), __func__, __FILE__, __LINE__);

    auto pImageIO = std::static_pointer_cast<CImageIO>(outputs[index]);
    CMat srcImg = pImageIO->getImage();

    CMat img;
    if(bWithGraphics)
    {
        img = srcImg.clone();
        auto graphicsOutputs =  taskPtr->getOutputs({IODataType::OUTPUT_GRAPHICS});
        for(size_t i=0; i<graphicsOutputs.size(); ++i)
        {
            auto graphicsOutPtr = std::static_pointer_cast<CGraphicsOutput>(graphicsOutputs[i]);
            if(graphicsOutPtr->getImageIndex() == index)
                Utils::Image::burnGraphics(img, graphicsOutPtr->getItems());
        }

        if (pImageIO->isOverlayAvailable())
            img = Utils::Image::mergeColorMask(img, pImageIO->getOverlayMask(), 0.7, true);
    }
    else
        img = srcImg;

    CImageDataIO io(Utils::File::getAvailablePath(path));
    io.write(img);
}

void CResultManager::saveOutputVideo(size_t id, const std::string& path)
{
    assert(m_pWorkflowMgr);
    WorkflowTaskPtr pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
        throw CException(CoreExCode::NULL_POINTER, "Invalid current task", __func__, __FILE__, __LINE__);

    QStringList paths;
    auto output = pTask->getOutput(id);
    auto dataType = output->getDataType();

    if(dataType == IODataType::VIDEO || dataType == IODataType::VIDEO_BINARY || dataType == IODataType::VIDEO_LABEL)
    {
        // Move to user-defined path
        auto pVideoIO = std::static_pointer_cast<CVideoIO>(output);
        auto infoPtr = std::static_pointer_cast<CDataVideoInfo>(pVideoIO->getDataInfo());
        auto resultPath = pVideoIO->getVideoPath();

        if(infoPtr && infoPtr->m_sourceType == CDataVideoBuffer::IMAGE_SEQUENCE)
        {
            for(size_t i=0; i<infoPtr->m_frameCount; ++i)
            {
                auto pathFrom = Utils::File::getPathFromPattern(resultPath, (int)i);
                auto imgPath = path + "/" + Utils::File::getFileName(pathFrom);
                Utils::File::moveFile(pathFrom, imgPath);
                paths.push_back(QString::fromStdString(imgPath));
            }
        }
        else
        {
            Utils::File::moveFile(resultPath, path);
            paths.push_back(QString::fromStdString(path));
        }
        emit doVideoSaveIsFinished(paths, static_cast<CDataVideoBuffer::Type>(infoPtr->m_sourceType));
    }

    //Create Workflow <-> Image association
    //mapWorkflowToImage();
}

void CResultManager::saveOutputMeasures(int index)
{
    auto pMultiProject = m_pProjectMgr->getMultiModel();
    assert(pMultiProject);

    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid current task");
        return;
    }

    //Get all outputs displayed as table
    auto outputs = pTask->getOutputs({IODataType::BLOB_VALUES, IODataType::NUMERIC_VALUES});
    if(index >= (int)outputs.size())
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid index");
        return;
    }

    auto pOutput = std::dynamic_pointer_cast<CBlobMeasureIO>(outputs[index]);
    if(!pOutput)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid output type");
        return;
    }

    //Check existence of root item
    auto rootIndex = getRootIndex();
    if(!rootIndex.isValid())
        rootIndex = createRootResult();

    //Set the structure of the result tree for protocol: WorkflowResults -> TaskResults
    auto protocolName = m_pWorkflowMgr->getWorkflowName();
    auto protocolResultIndex = findResultFromName(QString::fromStdString(protocolName));

    if(!protocolResultIndex.isValid())
        protocolResultIndex = pMultiProject->addItem(rootIndex, std::make_shared<CResultItem>(protocolName, CResultItem::NodeType::PROTOCOL));

    //Add task result item
    auto pResultItem = std::make_shared<CResultItem>(pTask->getName(), CResultItem::NodeType::TASK);
    pResultItem->setMeasures(pOutput->getMeasures());
    pMultiProject->addItem(protocolResultIndex, pResultItem);
}

void CResultManager::saveOutputMeasures(int index, const std::string &path)
{
    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid current task");
        return;
    }

    //Get all outputs displayed as table
    auto outputs = pTask->getOutputs({IODataType::BLOB_VALUES, IODataType::NUMERIC_VALUES});
    if(index >= (int)outputs.size())
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid index");
        return;
    }

    auto pOutput = std::dynamic_pointer_cast<CBlobMeasureIO>(outputs[index]);
    if(!pOutput)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid output type");
        return;
    }
    pOutput->save(path);
}

void CResultManager::saveOutputFeatures(int index, const std::string &path)
{
    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid current task");
        return;
    }

    //Get all outputs displayed as table
    auto outputs = pTask->getOutputs({IODataType::BLOB_VALUES, IODataType::NUMERIC_VALUES});
    if(index >= (int)outputs.size())
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid index");
        return;
    }

    auto pOutput = std::dynamic_pointer_cast<CNumericIOBase>(outputs[index]);
    if(!pOutput)
    {
        qCCritical(logResults).noquote() << tr("Error while saving measures: invalid output type");
        return;
    }
    pOutput->save(path);
}

void CResultManager::saveOutputGraphics()
{
    auto pTask = m_pWorkflowMgr->getActiveTask();
    if(!pTask)
    {
        qCCritical(logResults).noquote() << tr("Error while saving graphics: invalid current task");
        return;
    }

    //Set the structure of the graphics tree for protocol: WorkflowLayer -> TaskLayer
    auto protocolName = QString::fromStdString(m_pWorkflowMgr->getWorkflowName());
    auto protocolLayerIndex = m_pGraphicsMgr->findLayerFromName(protocolName);
    auto currentLayerIndex = m_pGraphicsMgr->getCurrentLayerIndex();

    if(!protocolLayerIndex.isValid())
    {
        //Create protocol layer if it does not exist
        auto pWorkflowLayer = new CGraphicsLayer(protocolName);
        auto rootLayerIndex = m_pGraphicsMgr->getRootLayerIndex();

        if(rootLayerIndex.isValid())
            m_pGraphicsMgr->setCurrentLayer(m_pGraphicsMgr->getRootLayerIndex(), true);

        protocolLayerIndex = m_pGraphicsMgr->addLayer(pWorkflowLayer);
    }
    else if(protocolLayerIndex != currentLayerIndex && !m_pGraphicsMgr->isChildLayer(currentLayerIndex, protocolLayerIndex))
    {
        //If current layer is not child of the protocol layer, protocol layer becomes the current
        m_pGraphicsMgr->setCurrentLayer(protocolLayerIndex, true);
    }

    //Add to project and image scene
    auto pOut = std::dynamic_pointer_cast<CGraphicsOutput>(pTask->getOutputFromType(IODataType::OUTPUT_GRAPHICS));
    if(pOut)
    {
        //We have to create new layer from output objects
        CGraphicsLayer* pSavedLayer = pOut->createLayer(m_pGraphicsMgr->getContext());
        m_pGraphicsMgr->addLayer(pSavedLayer);
        m_pGraphicsMgr->setCurrentLayer(protocolLayerIndex, true);
    }
}

void CResultManager::loadResults(const QModelIndex &index)
{
    if(index.isValid() == false)
        qCCritical(logResults).noquote() << tr("Error while loading results: invalid item index");

    auto pResultItem = getResultItem(index);
    assert(pResultItem);

    if(pResultItem->getNodeType() != CResultItem::NodeType::TASK)
        return;

    clearTableModels();

    try
    {
        if(pResultItem->isLoaded())
        {
            CResultDbManager resultDB;
            resultDB.setMeasures(pResultItem->getMeasures());
            m_tableModels.push_back(resultDB.createMeasureModel());
        }
        else
        {
            CProjectDbManager projectDB(m_pProjectMgr->getModel(index));
            CResultDbManager resultDB(projectDB.getPath(), projectDB.getConnectionName());
            pResultItem->setMeasures(resultDB.getMeasures(pResultItem->getDbId()));
            m_tableModels.push_back(resultDB.createMeasureModel(pResultItem->getDbId()));
        }

        //Prepare view according to output data types
        OutputDisplays outDisplay = {{DisplayType::TABLE_DISPLAY, {nullptr}}};
        emit doInitDisplay(outDisplay);
        emit doDisplayMeasuresTable(0, QString::fromStdString(pResultItem->getName()), static_cast<CMeasuresTableModel*>(m_tableModels[0]), nullptr);
        setCurrentResult(index);
    }
    catch(std::exception& e)
    {
        qCCritical(logResults).noquote() << QString::fromStdString(e.what());
    }
}

void CResultManager::startRecordVideo()
{
    assert(m_pWorkflowMgr);

    // Get active task for naming
    auto name = m_pWorkflowMgr->getWorkflowName();
    m_currentVideoRecord = Utils::IkomiaApp::getIkomiaFolder() + "/VideoRecord";

    try
    {
        Utils::File::createDirectory(m_currentVideoRecord);
    }
    catch (const CException& e)
    {
        qCCritical(logResults).noquote() << tr("Error while recording video:") << QString::fromStdString(e.what());
        return;
    }

    auto datasetIndex = m_pProjectMgr->getDatasetIndex(m_pProjectMgr->getCurrentDataItemIndex());
    int number = datasetIndex.model()->rowCount(datasetIndex);
    m_currentVideoRecord += "/liveProcess_" + name + "_" + std::to_string(number) + ".avi";

    // Create a CDataVideoBuffer for writing video
    if(m_pVideoMgr == nullptr)
        m_pVideoMgr = new CDataVideoBuffer(m_currentVideoRecord);

    // Get info from source video manager and copy information to video manager
    size_t fps = 25;
    auto pInfo = m_pDataMgr->getVideoMgr()->getVideoInfo(m_pWorkflowMgr->getCurrentVideoInputModelIndex());

    if(pInfo)
        fps = m_pWorkflowMgr->getCurrentFPS();

    m_pVideoMgr->startStreamWrite(pInfo->m_width, pInfo->m_height, fps);
}

void CResultManager::stopRecordVideo()
{
    // Test si on vient d'arrêter un enregistrement ou si on est juste dans le cas normal
    if(!m_currentVideoRecord.empty())
    {
        emit doAddRecordVideo(QString::fromStdString(m_currentVideoRecord));
        emit doNewResultNotification(tr("Video record has been added to the current project."), Notification::INFO);
        m_currentVideoRecord = "";
    }

    if(m_pVideoMgr)
        delete m_pVideoMgr;
    m_pVideoMgr = nullptr;
}

#include "moc_CResultManager.cpp"
