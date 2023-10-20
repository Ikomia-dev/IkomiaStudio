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

#include "CProjectManager.h"
#include <iostream>
#include <QtConcurrent>
#include <boost/filesystem.hpp>
#include <unordered_map>
#include "Main/LogCategory.h"
#include "Data/CMat.hpp"
#include "Data/CDataDicomInfo.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Model/Render/CRenderManager.h"
#include "Model/Results/CResultManager.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/Data/Video/CVideoManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Data/CMainDataManager.h"
#include "CProjectGraphicsProxyModel.h"
#include "CProjectDataProxyModel.h"
#include "CProjectExportMgr.h"

CProjectManager::CProjectManager()
{
    initConnections();
}

CProjectManager::~CProjectManager()
{
    for(auto& it : m_projectList)
        delete it;
}

void CProjectManager::onNewProject()
{
    try
    {
        auto pProject = createProject();
        assert(pProject != nullptr);
        pProject->updateHash();
        m_multiProject.addModel(pProject);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }

    emit doSetModel(&m_multiProject);
    // Update index to first item
    emit doUpdateIndex(m_multiProject.index(0,0));
}

void CProjectManager::onLoadProject(QStringList fileNames)
{
    m_loadWatcherCount = 0;
    m_pProgressMgr->launchInfiniteProgress(tr("Loading project..."), true);

    for(auto& it : fileNames)
    {
        createProject(false);
        auto pProjectModel = m_projectList.back();
        pProjectModel->setPath(it);

        QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>;
        connect(pWatcher, &QFutureWatcher<void>::finished, [this, pWatcher, pProjectModel]
        {
            pProjectModel->updateHash();
            m_multiProject.addModel(pProjectModel);
            delete pWatcher;
            m_loadWatcherCount--;

            if(m_loadWatcherCount == 0)
            {
                m_pProgressMgr->endInfiniteProgress();
                emit doSetModel(&m_multiProject);
                // Update index to first item
                emit doUpdateIndex(m_multiProject.index(0,0));
            }
        });

        auto future = QtConcurrent::run([pProjectModel, it]
        {
            try
            {
                CProjectDbManager project(pProjectModel);
                project.loadProject(it);
            }
            catch(std::exception& e)
            {
                qCCritical(logProject).noquote() << QString::fromStdString(e.what());
            }
        });
        pWatcher->setFuture(future);
        m_loadWatcherCount++;
    }
}

void CProjectManager::onLoadImages(QStringList fileNames, const DatasetLoadPolicy &policy, const QModelIndex& currentIndex)
{
    try
    {
        // Test if a project already exists
        // if not create project and add image
        // if so add image to existing project
        QModelIndex index;
        if(currentIndex.isValid())
            index = findRootProjectIndex(currentIndex);
        else
        {
            auto pProject = createProject();
            assert(pProject != nullptr);
            pProject->updateHash();
            m_multiProject.addModel(pProject);
            index = m_multiProject.index(0,0);
            emit doSetModel(&m_multiProject);
        }
        auto imgIndex = addImages(index, fileNames, policy);
        emit doUpdateIndex(imgIndex);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onLoadVideos(QStringList fileNames, const QModelIndex& currentIndex)
{
    try
    {
        // Test if a project already exists
        // if not create project and add image
        // if so add image to existing project
        QModelIndex index;
        if(currentIndex.isValid())
            index = findRootProjectIndex(currentIndex);
        else
        {
            auto pProject = createProject();
            assert(pProject != nullptr);
            pProject->updateHash();
            m_multiProject.addModel(pProject);
            index = m_multiProject.index(0,0);
            emit doSetModel(&m_multiProject);
        }
        auto videoIndex = addVideos(index, fileNames);
        emit doUpdateIndex(videoIndex);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onLoadStream(const QString& cameraId, const QString& cameraName, const QModelIndex& currentIndex)
{
    try
    {
        // Test if a project already exists
        // if not create project and add image
        // if so add image to existing project
        QModelIndex index;
        if(currentIndex.isValid())
            index = findRootProjectIndex(currentIndex);
        else
        {
            auto pProject = createProject();
            assert(pProject != nullptr);
            pProject->updateHash();
            m_multiProject.addModel(pProject);
            index = m_multiProject.index(0,0);
            emit doSetModel(&m_multiProject);
        }
        addStream(index, cameraId, cameraName);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onLoadFolder(const QString &folder, const QModelIndex &currentIndex)
{
    try
    {
        QModelIndex index;
        if(currentIndex.isValid())
            index = findRootProjectIndex(currentIndex);
        else
        {
            auto pProject = createProject();
            assert(pProject);
            pProject->updateHash();
            m_multiProject.addModel(pProject);
            index = m_multiProject.index(0, 0);
            emit doSetModel(&m_multiProject);
        }
        addFolderContent(folder, index);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onSaveProjectAs(const QModelIndex& index, const QString& fileName)
{
    if(!index.isValid())
        return;

    QModelIndex projectIndex = findRootProjectIndex(index);

    int row = projectIndex.row();
    auto pProjectModel = m_projectList.at(row);

    //Suppression de l'étoile indiquant que le projet a été modifié
    QString name = projectIndex.data().toString();
    if(name.endsWith("*"))
        name.chop(1);

    try
    {
        //Copie du fichier source pour conserver les éléments non chargés en mémoire
        auto currentProjectPath = pProjectModel->getPath().toStdString();
        if(!currentProjectPath.empty())
            boost::filesystem::copy_file(currentProjectPath, fileName.toStdString(), boost::filesystem::copy_options::overwrite_existing);

        pProjectModel->setPath(fileName);
        pProjectModel->getRoot()->getChild(0)->setName(name.toStdString());
        pProjectModel->setExported(false);
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.code().message());
        return;
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
        return;
    }    

    //Preparation du watcher pour le thread de sauvegarde
    QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>;
    connect(pWatcher, &QFutureWatcher<void>::finished, [this, pWatcher, row]
    {
        delete pWatcher;
        m_pProgressMgr->endInfiniteProgress();
        m_projectList.at(row)->updateHash();

        if(m_pGraphicsMgr)
            m_pGraphicsMgr->notifyProjectSave(row);

        if(m_pResultsMgr)
            m_pResultsMgr->notifyProjectSaved(row);
    });

    //Sauvegarde du nouveau fichier dans un thread
    m_pProgressMgr->launchInfiniteProgress(tr("Saving project as..."), false);
    auto future = QtConcurrent::run([name, fileName, pProjectModel]
    {
        try
        {
            CProjectDbManager project(name, pProjectModel);
            project.saveProject(fileName);
        }
        catch(std::exception& e)
        {
            qCCritical(logProject).noquote() << QString::fromStdString(e.what());
        }
    });
    pWatcher->setFuture(future);
}

void CProjectManager::onSaveProject(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    QModelIndex projectIndex = findRootProjectIndex(index);


    int row = projectIndex.row();
    auto pProjectModel = m_projectList.at(row);

    //Suppression de l'étoile indiquant que le projet a été modifié
    QString name = projectIndex.data().toString();
    if(name.endsWith("*"))
        name.chop(1);

    pProjectModel->getRoot()->getChild(0)->setName(name.toStdString());
    auto fileName = m_projectList.at(row)->getPath();

    if(fileName.isEmpty())
        return;

    pProjectModel->setExported(false);

    //Preparation du watcher pour le thread de sauvegarde
    QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>;
    connect(pWatcher, &QFutureWatcher<void>::finished, [this, pWatcher, row]
    {
        delete pWatcher;
        m_pProgressMgr->endInfiniteProgress();
        m_projectList.at(row)->updateHash();

        if(m_pGraphicsMgr)
            m_pGraphicsMgr->notifyProjectSave(row);

        if(m_pResultsMgr)
            m_pResultsMgr->notifyProjectSaved(row);
    });

    //Sauvegarde dans un thread
    m_pProgressMgr->launchInfiniteProgress(tr("Saving project..."), false);
    auto future = QtConcurrent::run([name, fileName, pProjectModel]
    {
        try
        {
            CProjectDbManager project(name, pProjectModel);
            project.saveProject(fileName);
        }
        catch(std::exception& e)
        {
            qCCritical(logProject).noquote() << QString(e.what());
        }
    });
    pWatcher->setFuture(future);
}

void CProjectManager::onCloseProject(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    QModelIndex projectIndex = findRootProjectIndex(index);
    bool bCurrentImgClosed = false;

    try
    {
        bCurrentImgClosed = wrapIndex(projectIndex).model() == m_pCurrentModel;
        notifyBeforeProjectClose(projectIndex, bCurrentImgClosed);

        // Remove project from tree view
        m_multiProject.removeItem(projectIndex);
        // Remove and delete project model from list
        int row = projectIndex.row();
        delete m_projectList.at(row);
        m_projectList.erase(m_projectList.begin()+row);
        // Delete graphics proxy model
        delete m_pGraphicsProxyModel;
        m_pGraphicsProxyModel = nullptr;
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }

    //Notify view
    emit doCloseProject(bCurrentImgClosed);
}

void CProjectManager::onAddFolder(const QModelIndex &index)
{
    try
    {
        addFolder(index, tr("NewFolder").toStdString());
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onAddDataset(const QModelIndex &index, IODataType type)
{
    try
    {
        addDataset(index, tr("NewDataset").toStdString(), type);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onAddDimension(const QModelIndex &index, DataDimension dim)
{
    try
    {
        addDimension(index, dim);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onAddImage(const QModelIndex &index, QStringList &files, const DatasetLoadPolicy &policy)
{
    try
    {
        auto imgIndex = addImages(index, files, policy);
        emit doUpdateIndex(imgIndex);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onAddImageToDimension(const QModelIndex &index, const QStringList &files)
{
    try
    {
        auto imgIndex = addImagesToDimension(index, files);
        emit doUpdateIndex(imgIndex);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onAddDicomFolder(const QModelIndex &index, const QString &folder)
{
    SubsetBounds bounds = {{DataDimension::STUDY, {0, 0}},
                           {DataDimension::SERIE, {0, 0}},
                           {DataDimension::VOLUME, {0, 0}},
                           {DataDimension::IMAGE, {0, 0}}};

    CImageDataIO imageIO(folder.toStdString());

    QModelIndex firstImgIndex;

    //Get metadata for the first image to retrieve patient name
    auto pInfo = std::dynamic_pointer_cast<CDataDicomInfo>(imageIO.dataInfo());
    assert(pInfo != nullptr);

    QModelIndex patientIndex;
    auto it = pInfo->metadata().find("PatientName");
    if(it != pInfo->metadata().end() && !it->second.empty())
        patientIndex = addFolder(index, it->second);
    else
        patientIndex = addFolder(index, tr("Anonym").toStdString());

    //Create folder for studies
    for(int i=0; i<pInfo->m_nbStudy; ++i)
    {
        QModelIndex studyIndex;
        bounds[0].second = std::make_pair(i, i);
        bounds[1].second = std::make_pair(0, 0);
        pInfo = std::dynamic_pointer_cast<CDataDicomInfo>(imageIO.dataInfo(bounds));
        assert(pInfo != nullptr);

        it = pInfo->metadata().find("StudyDescription");
        if(it != pInfo->metadata().end() && !it->second.empty())
            studyIndex = addFolder(patientIndex, it->second);
        else
            studyIndex = addFolder(patientIndex, tr("Study").toStdString());

        //Create dataset for series
        for(int j=0; j<pInfo->m_nbSerie; ++j)
        {
            QModelIndex serieIndex;
            bounds[1].second = std::make_pair(j, j);
            pInfo = std::dynamic_pointer_cast<CDataDicomInfo>(imageIO.dataInfo(bounds));
            assert(pInfo != nullptr);

            it = pInfo->metadata().find("SeriesDescription");
            if(it != pInfo->metadata().end() && !it->second.empty())
                serieIndex = addDataset(studyIndex, it->second, IODataType::IMAGE);
            else
                serieIndex = addDataset(studyIndex, tr("Serie").toStdString(), IODataType::IMAGE);

            //Add Volume dimension
            QModelIndex dimensionIndex = addDimension(serieIndex, DataDimension::VOLUME);

            //Add images of the serie
            QStringList qfiles;
            auto files = imageIO.getFileNames(bounds);
            for(size_t k=0; k<files.size(); ++k)
                qfiles.append(QString::fromStdString(files[k]));

            auto imgIndex = addImagesToDimension(dimensionIndex, qfiles);
            if(i==0 && j==0)
                firstImgIndex = imgIndex;
        }
    }
    emit doUpdateIndex(firstImgIndex);
}

void CProjectManager::onDeleteItem(const QModelIndex &index)
{
    deleteItem(index);
}

void CProjectManager::onDisplayData(const QModelIndex &index)
{
    try
    {
        auto wrapInd = wrapIndex(index);
        ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());

        if(itemPtr->getTypeId() == TreeItemType::IMAGE)
            displayImage(index);
        else if(itemPtr->getTypeId() == TreeItemType::VIDEO || itemPtr->getTypeId() == TreeItemType::LIVE_STREAM)
            displayVideo(index);
        else if(itemPtr->getTypeId() == TreeItemType::RESULT)
        {
            assert(m_pResultsMgr);
            m_pResultsMgr->displayResult(index);
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::onDisplayVideoImage(const QModelIndex& modelIndex, int index, QImage image, QString name)
{
    if(!modelIndex.isValid())
    {
        qCDebug(logProject()) << "Video image index is not valid.";
        return;
    }

    // Manage graphics scene display
    auto wrapInd = wrapIndex(modelIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());
    if(!pItem)
        return;

    auto type = pItem->getTypeId();
    CImageScene* pScene = nullptr;

    if(type == TreeItemType::IMAGE)
    {
        // If image sequence, get image item and display new image in image item scene (one different scene per image)
        auto imgItemPtr = CProjectUtils::getItem<CImageItem>(wrapInd, TreeItemType::IMAGE);
        if(!imgItemPtr)
        {
            qCCritical(logProject) << QString("Invalid model index for time sequence at position %1 for image %2").arg(index).arg(name);
            return;
        }

        pScene = imgItemPtr->getScene();

        // Update image index to next index in case of playing image sequence
        if(m_pDataMgr->getVideoMgr()->isPlaying(getCurrentVideoItemIndex()))
            updateImageSequenceIndex();
    }
    else if(type == TreeItemType::VIDEO)
    {
        // If real video, get video item and display new image in video item scene (one unique scene for all images)
        auto videoItemPtr = CProjectUtils::getItem<CVideoItem>(wrapInd, TreeItemType::VIDEO);
        if(!videoItemPtr)
        {
            qCCritical(logProject) << QString("Invalid model index for video %1").arg(name);
            return;
        }
        pScene = videoItemPtr->getScene();
    }
    else if(type == TreeItemType::LIVE_STREAM)
    {
        // If real video, get video item and display new image in video item scene (one unique scene for all images)
        auto streamItemPtr = CProjectUtils::getItem<CLiveStreamItem>(wrapInd, TreeItemType::LIVE_STREAM);
        if(!streamItemPtr)
        {
            qCCritical(logProject) << QString("Invalid model index for camera %1").arg(name);
            return;
        }
        pScene = streamItemPtr->getScene();
    }

    //Load associated graphics
    // ATTENTION: item scene pointer must be created!
    m_pGraphicsMgr->loadAllGraphics(modelIndex);

    // Notify view to display image.
    emit doDisplayVideo(modelIndex, index, pScene, image, name, m_bVideoChanged, nullptr);

    m_bVideoChanged = false;
}

void CProjectManager::onCurrentImgChangedInZ(int index)
{
    if(!m_currentDataItemIndex.isValid())
        return;

    if(m_currentDataItemIndex.row() == index)
        return;

    auto parentIndex = m_currentDataItemIndex.parent();
    emit doUpdateImgZ(parentIndex.model()->index(index, 0, parentIndex));
}

void CProjectManager::onInsertDimension(const QModelIndex &parentIndex, const DataDimension dim)
{
    try
    {
        int childCount = m_multiProject.rowCount(parentIndex);
        auto newDimensionIndex = addDimension(parentIndex, dim);
        moveChildData(parentIndex, newDimensionIndex, childCount);
    }
    catch(CException& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.getMessage());
    }
}

void CProjectManager::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(bottomRight);

    if(roles.size() == 0)
        manageCheckStateChanged(topLeft);
    else
    {
        for(int i=0; i<roles.size(); ++i)
        {
            if(roles[i] == Qt::CheckStateRole)
                manageCheckStateChanged(topLeft);
        }
    }
}

void CProjectManager::onUpdateVideoPos(const QModelIndex &modelIndex, int index, size_t pos)
{
    auto pVideoMgr = m_pDataMgr->getVideoMgr();
    if(pVideoMgr->getCurrentPos(modelIndex) == pos)
        return;

    // Get current item in model
    auto wrapInd = wrapIndex(modelIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());

    if(!pItem)
        return;

    auto type = pItem->getTypeId();
    if(type == TreeItemType::IMAGE) // If image sequence
    {
        // Get parent index
        auto parentIndex = m_currentDataItemIndex.parent();
        // Set current index according to pos if image sequence
        m_currentDataItemIndex = parentIndex.model()->index(pos, 0, parentIndex);
        // Update wrap index
        wrapInd = wrapIndex(m_currentDataItemIndex);
        // Notify view to update index in treeview in case of image sequence
        emit doUpdateSelectedIndex(m_currentDataItemIndex);
    }

    // Notify video manager to update video position
    pVideoMgr->updateVideoPos(modelIndex, index, pos);
}

void CProjectManager::onRecordVideo(const QModelIndex &modelIndex, bool bRecord)
{
    m_pDataMgr->getVideoMgr()->setVideoRecord(modelIndex, bRecord);
}

void CProjectManager::onVideoStopped()
{
    auto wrapInd = wrapIndex(m_currentDataItemIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());
    if(!pItem)
        return;

    if(pItem->getTypeId() == TreeItemType::IMAGE)
    {
        // For image sequence, we have to reset current data model index
        auto parentIndex = m_currentDataItemIndex.parent();
        m_currentDataItemIndex = parentIndex.model()->index(0, 0, parentIndex);
    }
}

void CProjectManager::onAddRecordVideo(const QString& path)
{
    //Création d'un dataset dans le projet
    auto currentDatasetIndex = getDatasetIndex(m_currentDataItemIndex);

    //Ajout de la video résultat
    QStringList files;
    files.push_back(path);
    addVideos(currentDatasetIndex, files);
}

void CProjectManager::onUpdateIndex(const QPersistentModelIndex& index)
{
    // Manage closing camera when we change item in project tree
    if(index.isValid() == false)
        return;

    if(m_currentDataItemIndex == index)
        return;

    auto wrapInd = wrapIndex(index);
    m_pCurrentModel = wrapInd.model();

    if(isContainerItem(wrapInd) == true)
    {
        // Notify view to stop video
        m_pDataMgr->closeData(m_currentDataItemIndex);
        m_currentDataItemIndex = index;
        m_pWorkflowMgr->onInputDataChanged(index, 0, true);
    }
    else
    {
        QModelIndex localIndex = index;
        if(isImageSubItem(wrapInd) == true)
            localIndex = getImageIndexFromSubItem(localIndex);

        bool bVideoHasChanged = sequenceHasChanged(localIndex);
        if(bVideoHasChanged)
            m_pDataMgr->closeData(m_currentDataItemIndex);

        m_currentDataItemIndex = localIndex;
    }
}

void CProjectManager::onExportProject(const QModelIndex &index, const QString &folder)
{
    assert(index.isValid());

    m_pProgressMgr->launchInfiniteProgress(tr("Exporting project..."), false);
    QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>;

    connect(pWatcher, &QFutureWatcher<void>::finished, [this, pWatcher]
    {
        delete pWatcher;
        m_pProgressMgr->endInfiniteProgress();
    });

    auto future = QtConcurrent::run([this, index, folder]
    {
        try
        {
            int row = index.row();
            CProjectExportMgr exportMgr(m_projectList[row], folder);
            exportMgr.run();
        }
        catch(std::exception& e)
        {
            qCCritical(logProject).noquote() << QString::fromStdString(e.what());
        }
    });
    pWatcher->setFuture(future);
}

void CProjectManager::onShowLocation(const QModelIndex &index)
{
    assert(m_pDataMgr);
    QString path = m_pDataMgr->getDataPath(wrapIndex(index));
    if(path.isEmpty())
    {
        qCCritical(logProject).noquote() << tr("Error while retrieving data path");
        return;
    }
    Utils::File::showLocation(path);
}

void CProjectManager::initConnections()
{
    connect(&m_multiProject, &CMultiProjectModel::dataChanged, this, &CProjectManager::onDataChanged);
}

QModelIndex CProjectManager::getImageIndexFromSubItem(const QModelIndex &index)
{
    QModelIndex parentIndex = index.parent();
    QModelIndex wrapInd = wrapIndex(parentIndex);

    while(isImageItem(wrapInd) == false)
    {
        parentIndex = parentIndex.parent();
        wrapInd = wrapIndex(parentIndex);
    }
    return parentIndex;
}

QModelIndex CProjectManager::addImagesToDataset(const QModelIndex &datasetItemIndex, const QStringList &files)
{
    assert(m_pProgressMgr);

    auto index = wrapIndex(datasetItemIndex);
    auto pDatasetItem = CProjectUtils::getDatasetItem<CMat>(index);
    if(pDatasetItem)
    {
        auto pDataset = pDatasetItem->getDataset();
        if(pDataset)
        {
            //Init progress bar
            if(files.size() > 1)
                m_pProgressMgr->launchProgress(&m_progressSignal, files.size(), tr("Add images to dataset"), true);

            size_t nbImages = pDataset->getDataInfo().size(DataDimension::IMAGE);
            pDataset->setDimension(DataDimension::IMAGE, nbImages + files.size());

            QModelIndex firstImgIndex;
            for(int i=0; i<files.size(); ++i)
            {
                boost::filesystem::path path(files[i].toStdString());
                auto index = m_multiProject.addItem(datasetItemIndex, std::make_shared<CImageItem>(path.stem().string(), path.string()));
                if(i == 0)
                    firstImgIndex = index;

                pDataset->appendFile(files[i].toStdString());
                emit m_progressSignal.doProgress();
            }
            return firstImgIndex;
        }
    }
    return datasetItemIndex;
}

QModelIndex CProjectManager::addImagesToDimension(const QModelIndex &itemIndex, const QStringList &files)
{
    auto pSet = CProjectUtils::getDataset<CMat>(wrapIndex(itemIndex));
    if(!pSet)
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapIndex(itemIndex).internalPointer());
    assert(pItem);
    size_t nbImagesIntoDim = pItem->getChildCount();
    size_t nbImages = pSet->getDataInfo().size(DataDimension::IMAGE);

    if(nbImagesIntoDim + files.size() > nbImages)
        pSet->setDimension(DataDimension::IMAGE, nbImagesIntoDim + files.size());

    size_t insertIndex = CProjectUtils::getIndexInDataset(wrapIndex(itemIndex));
    if(insertIndex == SIZE_MAX)
    {
        qCCritical(logProject).noquote() << tr("Dataset index out of bounds") + QString::fromStdString(__func__) + QString::fromStdString(__FILE__) + QString::number(__LINE__);
        return QModelIndex();
    }

    if(m_multiProject.rowCount(itemIndex) > 0)
        insertIndex++;

    pSet->setCurrentIndex(insertIndex);
    QModelIndex firstImgIndex;

    //Init progress bar
    if(files.size() > 1)
        m_pProgressMgr->launchProgress(&m_progressSignal, files.size(), tr("Add images to dimension"), true);

    for(int i=0; i<files.size(); ++i)
    {
        boost::filesystem::path path(files[i].toStdString());
        auto index = m_multiProject.addItem(itemIndex, std::make_shared<CImageItem>(path.stem().string(), path.string()));
        if(i == 0)
            firstImgIndex = index;

        pSet->appendFile(files[i].toStdString());
        emit m_progressSignal.doProgress();
    }

    return firstImgIndex;
}

void CProjectManager::displayImage(const QModelIndex &itemIndex)
{
    assert(m_pGraphicsMgr);
    assert(m_pResultsMgr);
    assert(m_pRenderMgr);
    assert(m_pWorkflowMgr);

    auto indexWrapped = wrapIndex(itemIndex);
    //Get the corresponding image item
    auto imgItemPtr = CProjectUtils::getImageItem(indexWrapped);
    assert(imgItemPtr);

    // Test if dataset containing image is already loaded before doing getImage()
    bool bStackChanged = sequenceHasChanged(itemIndex);

    //Update current states
    m_pCurrentModel = indexWrapped.model();
    m_currentDataItemIndex = QPersistentModelIndex(itemIndex);
    m_currentIndices = CProjectUtils::getIndicesInDataset(indexWrapped);
    m_currentDatasetId = CProjectUtils::getDatasetItem<CMat>(indexWrapped)->getId();
    auto pDataset = CProjectUtils::getDataset<CMat>(indexWrapped);

    if(pDataset->hasDimension(DataDimension::VOLUME)) // 3D image
    {
        // Notify data manager to display volume
        m_pDataMgr->displayVolumeImage(imgItemPtr->getScene(), itemIndex, indexWrapped, bStackChanged);        
    }
    else if(pDataset->hasDimension(DataDimension::TIME)) // Video image sequence
    {
        m_bVideoChanged = bStackChanged;
        // Display new image
        m_pDataMgr->displayImageSequence(itemIndex, 0, m_bVideoChanged);
    }
    else if(pDataset->hasDimension(DataDimension::IMAGE)) // Simple image
    {
        // Notify data manager to display image
        m_pDataMgr->displaySimpleImage(imgItemPtr->getScene(), itemIndex, indexWrapped, 0, bStackChanged);
    }
    m_pWorkflowMgr->loadImageWorkflows(itemIndex);
}

void CProjectManager::displayVideo(const QModelIndex& itemIndex)
{
    assert(itemIndex.isValid());

    try
    {
        // Test if dataset containing image is already loaded before doing getImage()
        m_bVideoChanged = sequenceHasChanged(itemIndex);

        // Set current item index
        auto indexWrapped = wrapIndex(itemIndex);
        m_pCurrentModel = indexWrapped.model();
        m_currentDataItemIndex = QPersistentModelIndex(itemIndex);
        m_currentIndices = CProjectUtils::getIndicesInDataset(indexWrapped);
        m_currentDatasetId = CProjectUtils::getDatasetItem<CMat>(indexWrapped)->getId();

        // Display new image
        m_pDataMgr->displayVideoImage(itemIndex, 0, m_bVideoChanged);
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString::fromStdString(e.what());
    }
}

void CProjectManager::removeImage(const QModelIndex &itemIndex)
{
    //Notify others managers that image will be removed from project
    notifyBeforeDataDeleted(itemIndex);

    //Remove image from project -> to do before removing from dataset
    m_multiProject.removeItem(itemIndex);

    //Remove from dataset
    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex(itemIndex));
    if(pDataset)
    {
        size_t index = CProjectUtils::getIndexInDataset(wrapIndex(itemIndex));
        // Clear subset bounds to avoid displaying the removed image
        pDataset->subset().bounds().clear();
        if(index != SIZE_MAX)
            pDataset->removeAt(index);
        else
            qCWarning(logProject()).noquote() << tr("Invalid image index");
    }
    else
        qCWarning(logProject()).noquote() << tr("No dataset found from model index");
}

void CProjectManager::removeVideo(const QModelIndex &itemIndex)
{
    notifyBeforeDataDeleted(itemIndex);
    m_multiProject.removeItem(itemIndex);
}

void CProjectManager::removeDimension(const QModelIndex &itemIndex)
{
    std::vector<QModelIndex> indexes;
    fillChildDataItem(itemIndex, indexes);
    notifyBeforeDataDeleted(indexes);

    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex(itemIndex));
    if(pDataset)
    {
        auto dimItem = CProjectUtils::getDimensionItem(wrapIndex(itemIndex));
        DimensionIndices indices = CProjectUtils::getIndicesInDataset(wrapIndex(itemIndex));
        pDataset->removeDimension(dimItem->getDimension(), indices);
    }
    else
        qCWarning(logProject()).noquote() << tr("No dataset found from model index");

    m_multiProject.removeItem(itemIndex);
}

void CProjectManager::removeDataset(const QModelIndex &itemIndex)
{
    std::vector<QModelIndex> indexes;
    fillChildDataItem(itemIndex, indexes);
    notifyBeforeDataDeleted(indexes);
    m_multiProject.removeItem(itemIndex);
}

void CProjectManager::removeFolder(const QModelIndex &itemIndex)
{
    std::vector<QModelIndex> indexes;
    fillChildDataItem(itemIndex, indexes);
    notifyBeforeDataDeleted(indexes);
    m_multiProject.removeItem(itemIndex);
}

void CProjectManager::deleteItem(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    try
    {
        auto itemIndex = wrapIndex(index);
        ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(itemIndex.internalPointer());

        if(pItem->getTypeId() == TreeItemType::IMAGE)
            removeImage(index);
        else if(pItem->getTypeId() == TreeItemType::VIDEO || pItem->getTypeId() == TreeItemType::LIVE_STREAM)
            removeVideo(index);
        else if(pItem->getTypeId() == TreeItemType::DIMENSION)
            removeDimension(index);
        else if(pItem->getTypeId() == TreeItemType::DATASET)
            removeDataset(index);
        else if(pItem->getTypeId() == TreeItemType::FOLDER)
            removeFolder(index);
        else if(pItem->getTypeId() == TreeItemType::GRAPHICS_LAYER && m_pGraphicsMgr)
            m_pGraphicsMgr->removeLayer(index);
        else if(pItem->getTypeId() == TreeItemType::RESULT && m_pResultsMgr)
            m_pResultsMgr->removeResult(index);
        else if(pItem->getTypeId() == TreeItemType::PROJECT)
            onCloseProject(index);
        else
            m_multiProject.removeItem(index);

        if(m_currentDataItemIndex.isValid() == false)
        {
            //Current image has been deleted
            //If image has never been loaded -> not really well handled, we just go up in the tree
            emit doUpdateIndex(index.parent());
            emit doAfterCurrentImageDeleted();
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logProject).noquote() << QString(e.what());
    }
}

void CProjectManager::fillChildDataItem(const QModelIndex& parentIndex, std::vector<QModelIndex>& indexes) const
{
    int childCount = m_multiProject.rowCount(parentIndex);
    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = m_multiProject.index(i, 0, parentIndex);
        if(isImageItem(wrapIndex(childIndex)))
            indexes.push_back(childIndex);
        else
            fillChildDataItem(childIndex, indexes);
    }
}

bool CProjectManager::isAlreadyLoadedInMemory(const QModelIndex& itemIndex) const
{
    CImageDataManager imgMgr;
    auto pDataset = CProjectUtils::getDataset<CMat>(itemIndex);
    DimensionIndices ind = CProjectUtils::getIndicesInDataset(itemIndex);
    auto currentImgInd = Utils::Data::getDimensionSize(ind, DataDimension::IMAGE);
    auto bounds = pDataset->subsetBounds(currentImgInd);
    return imgMgr.isInMemory(*pDataset, bounds);
}

bool CProjectManager::isContainerItem(const QModelIndex &wrapIndex) const
{
    if(!wrapIndex.isValid())
        return false;

    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    auto type = static_cast<TreeItemType>(pItem->getTypeId());
    return (type == TreeItemType::PROJECT || type == TreeItemType::FOLDER ||
            type == TreeItemType::DATASET || type == TreeItemType::DIMENSION);
}

bool CProjectManager::isImageItem(const QModelIndex &wrapIndex) const
{
    if(!wrapIndex.isValid())
        return false;

    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    auto type = static_cast<TreeItemType>(pItem->getTypeId());
    return (type == TreeItemType::IMAGE || type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM);
}

bool CProjectManager::isImageSubItem(const QModelIndex &wrapIndex) const
{
    if(!wrapIndex.isValid())
        return false;

    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    auto type = static_cast<TreeItemType>(pItem->getTypeId());
    return (type == TreeItemType::GRAPHICS_LAYER || type == TreeItemType::RESULT);
}

bool CProjectManager::sequenceHasChanged(const QModelIndex& itemIndex) const
{
    bool bChanged = false;
    auto indexWrap = wrapIndex(itemIndex);
    auto pDatasetItem = CProjectUtils::getDatasetItem<CMat>(indexWrap);

    if(pDatasetItem)
    {
        if(m_currentIndices.empty())
            return true;

        DimensionIndices currentIndices = m_currentIndices;
        DimensionIndices newIndices = CProjectUtils::getIndicesInDataset(indexWrap);

        currentIndices.pop_back();
        newIndices.pop_back();
        bChanged = (pDatasetItem->getId() != m_currentDatasetId || currentIndices.size() == 0 || newIndices != currentIndices);
    }
    return bChanged;
}

void CProjectManager::moveChildData(const QModelIndex &source, const QModelIndex &target, const int childCount)
{
    int j = 0;

    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = m_multiProject.index(0, 0, source);
        if(isImageItem(wrapIndex(childIndex)))
            m_multiProject.moveItem<std::shared_ptr<CImageItem>>(childIndex, target, j++);
    }
}

void CProjectManager::manageCheckStateChanged(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    if(itemPtr)
    {
        if(itemPtr->getTypeId() == TreeItemType::GRAPHICS_LAYER && m_pGraphicsMgr)
            m_pGraphicsMgr->setLayerVisible(index, itemPtr->isChecked());
    }
}

void CProjectManager::notifyBeforeProjectClose(const QModelIndex& projectIndex, bool bWithCurrentImg)
{
    assert(m_pGraphicsMgr);
    assert(m_pWorkflowMgr);
    assert(m_pResultsMgr);
    int row = projectIndex.row();
    m_pGraphicsMgr->beforeProjectClose(row);
    m_pWorkflowMgr->beforeProjectClose(bWithCurrentImg);
    m_pResultsMgr->notifyBeforeProjectClosed(row, bWithCurrentImg);
    m_pDataMgr->beforeProjectClose(row, bWithCurrentImg);
}

void CProjectManager::notifyBeforeDataDeleted(const QModelIndex &index)
{
    assert(m_pWorkflowMgr);
    m_pWorkflowMgr->beforeDataDeleted(index);
}

void CProjectManager::notifyBeforeDataDeleted(const std::vector<QModelIndex> &indexes)
{
    assert(m_pWorkflowMgr);
    m_pWorkflowMgr->beforeDataDeleted(indexes);
}

void CProjectManager::checkSequenceFileNames(DataDimension dim, QStringList &files)
{
    if(dim == DataDimension::TIME)
        checkTimeSequenceFileName(files);
}

void CProjectManager::checkTimeSequenceFileName(QStringList &files)
{
    //Check file names conformity with cv::VideoCapture contraints
    if(files.size() >= 2)
    {
        //Find the number of digits requires
        int nb = files.size();
        int digitsRequired = QString::number(nb).length() + 1;

        boost::filesystem::path file1(files.first().toStdString());
        boost::filesystem::path file2(files.last().toStdString());
        //Get extension
        QString extension = QString::fromStdString(file1.extension().string());
        //Get parent directory
        QString directory = QString::fromStdString(file1.parent_path().string());
        //Get two opposite filenames in the sequence (only name without extension)
        std::string name1 = file1.stem().string();
        std::string name2 = file2.stem().string();
        //Find common pattern in the names
        QString pattern = QString::fromStdString(Utils::String::longestCommonSubstring(name1, name2));
        //Extract potential digits part
        QString digits = QString::fromStdString(name1).remove(pattern);
        auto index = digits.toStdString().find_first_not_of("0123456789");

        if(index != std::string::npos)
        {
            //Not a valid digits part -> rename all files
            qCInfo(logProject()).noquote() << tr("Original names of time sequence were not valid, they have been renamed automatically.");
            files.sort();

            try
            {
                for(int i=0; i<nb; ++i)
                {
                    QString number = QString("%1").arg(i, digitsRequired, 10, QChar('0'));
                    QString newFile = directory + "/" + pattern + number + extension;
                    boost::filesystem::path oldPath(files[i].toStdString());
                    boost::filesystem::path newPath(newFile.toStdString());
                    boost::filesystem::rename(oldPath, newPath);
                    files[i] = newFile;
                }
            }
            catch (const boost::filesystem::filesystem_error& e)
            {
                qCCritical(logProject).noquote() << QString::fromStdString(e.code().message());
                return;
            }
        }
    }
}

QModelIndex CProjectManager::findRootProjectIndex(const QModelIndex& index)
{
    auto projectIndex = index;
    while(projectIndex.parent() != QModelIndex())
        projectIndex = projectIndex.parent();

    return projectIndex;
}

void CProjectManager::updateImageSequenceIndex()
{
    // Get parent index and parent item
    int nextRow;
    auto parentIndex = m_currentDataItemIndex.parent();    
    int rowCount = parentIndex.model()->rowCount(parentIndex);

    // Loop if current index reaches end of sequence
    if(m_currentDataItemIndex.row()+1 < rowCount)
        nextRow = m_currentDataItemIndex.row() + 1;
    else
        nextRow = 0;

    // QModelIndex of next image
    emit doUpdateSelectedIndex(m_currentDataItemIndex);
    m_currentDataItemIndex = parentIndex.model()->index(nextRow, 0, parentIndex);
}

QModelIndex CProjectManager::wrapIndex(const QModelIndex& index) const
{
    return m_multiProject.getWrappedIndex(index);
}

void CProjectManager::notifyDataChanged()
{
    m_multiProject.dataChanged(m_currentDataItemIndex, m_currentDataItemIndex);
}

CProjectModel* CProjectManager::createProject(bool bWithRoot)
{
    auto pProject = new CProjectModel;
    pProject->insertIcon(TreeItemType::PROJECT, QIcon(":/Images/project.png"));
    pProject->insertIcon(TreeItemType::FOLDER, QIcon(":/Images/folder.png"));
    pProject->insertIcon(TreeItemType::DATASET, QIcon(":/Images/dataset.png"));
    pProject->insertIcon(TreeItemType::DIMENSION, QIcon(":/Images/dimension.png"));
    pProject->insertIcon(TreeItemType::IMAGE, QIcon(":/Images/image.png"));
    pProject->insertIcon(TreeItemType::GRAPHICS_LAYER, QIcon(":/Images/draw-layers.png"));
    pProject->insertIcon(TreeItemType::RESULT, QIcon(":/Images/tool-table-color.png"));
    pProject->insertIcon(TreeItemType::VIDEO, QIcon(":/Images/video.png"));
    pProject->insertIcon(TreeItemType::LIVE_STREAM, QIcon(":/Images/cam.png"));

    if(bWithRoot)
        pProject->getRoot()->emplace_back(std::make_shared<CProjectItem>(tr("New Project").toStdString()));

    m_projectList.push_back(pProject);
    return pProject;
}

CProjectGraphicsProxyModel *CProjectManager::getGraphicsProxyModel(const std::vector<QModelIndex> &indicesFrom)
{    
    if(m_pGraphicsProxyModel)
        m_pGraphicsProxyModel->setReferenceIndices(indicesFrom);
    else
    {
        m_pGraphicsProxyModel = new CProjectGraphicsProxyModel(indicesFrom);
        m_pGraphicsProxyModel->setSourceModel(&m_multiProject);
    }
    return m_pGraphicsProxyModel;
}

CProjectDataProxyModel *CProjectManager::getDataProxyModel(const QModelIndex &from, const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters)
{
    //Get root item of the project
    size_t index = getProjectIndex(from);
    auto projectRootIndex = m_multiProject.index((int)index, 0);

    if(m_pDataProxyModel)
        m_pDataProxyModel->setProxyParameters(projectRootIndex, dataTypes, filters);
    else
    {
        m_pDataProxyModel = new CProjectDataProxyModel(projectRootIndex, dataTypes, filters);
        m_pDataProxyModel->setSourceModel(&m_multiProject);
    }
    return m_pDataProxyModel;
}

std::string CProjectManager::getItemPath(const QModelIndex &index) const
{
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type == TreeItemType::FOLDER)
    {
        auto folderItemPtr = pItem->getNode<std::shared_ptr<CFolderItem>>();
        return folderItemPtr->getPath();
    }
    if(type == TreeItemType::IMAGE)
    {
        auto imageItemPtr = pItem->getNode<std::shared_ptr<CImageItem>>();
        return imageItemPtr->getFullPath();
    }
    else if(type == TreeItemType::VIDEO)
    {
        auto videoItemPtr = pItem->getNode<std::shared_ptr<CVideoItem>>();
        return videoItemPtr->getFullPath();
    }
    else
        return std::string();
}

TreeItemType CProjectManager::getItemType(const QModelIndex &index) const
{
    if(!index.isValid())
        return TreeItemType::NONE;

    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    return static_cast<TreeItemType>(pItem->getTypeId());
}

ProjectTreeItem *CProjectManager::getItem(const QModelIndex &index) const
{
    if(!index.isValid())
        return nullptr;

    auto wrapindex = wrapIndex(index);
    return static_cast<ProjectTreeItem*>(wrapindex.internalPointer());
}

size_t CProjectManager::getFolderDataItemCount(const QModelIndex &index) const
{
    size_t count = 0;
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type != TreeItemType::FOLDER)
        return 0;

    int childCount = m_multiProject.rowCount(index);
    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = m_multiProject.index(i, 0, index);
        auto wrapChildIndex = wrapIndex(childIndex);
        auto pChild = static_cast<ProjectTreeItem*>(wrapChildIndex.internalPointer());
        auto childType = static_cast<TreeItemType>(pChild->getTypeId());

        if(childType == TreeItemType::FOLDER)
            count += getFolderDataItemCount(childIndex);
        else if(childType == TreeItemType::DATASET)
        {
            auto pDataset = CProjectUtils::getDataset<CMat>(wrapChildIndex);
            if(pDataset->hasDimension(DataDimension::VOLUME) || pDataset->hasDimension(DataDimension::TIME))
                count += 1;
            else
                count += pDataset->size();
        }
    }
    return count;
}

QModelIndex CProjectManager::getDatasetDataIndex(const QModelIndex& datasetIndex, size_t dataIndex) const
{
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(datasetIndex).internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type != TreeItemType::DATASET)
        return QModelIndex();

    QModelIndex parent = datasetIndex;
    QModelIndex firstChild = m_multiProject.index(0, 0, parent);

    while(!isImageItem(wrapIndex(firstChild)))
    {
        parent = firstChild;
        firstChild = m_multiProject.index(0, 0, parent);
    }
    return m_multiProject.index(dataIndex, 0, parent);
}

QModelIndex CProjectManager::getFolderDataIndex(const QModelIndex &folderIndex, size_t& dataIndex) const
{
    QModelIndex dataModelIndex;
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(folderIndex).internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type != TreeItemType::FOLDER)
        return QModelIndex();

    int childCount = m_multiProject.rowCount(folderIndex);
    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = m_multiProject.index(i, 0, folderIndex);
        auto wrapChildIndex = wrapIndex(childIndex);
        auto pChild = static_cast<ProjectTreeItem*>(wrapChildIndex.internalPointer());
        auto childType = static_cast<TreeItemType>(pChild->getTypeId());

        if(childType == TreeItemType::FOLDER)
        {
            dataModelIndex = getFolderDataIndex(childIndex, dataIndex);
            if(dataModelIndex.isValid())
                return dataModelIndex;
        }
        else if(childType == TreeItemType::DATASET)
        {
            auto pDataset = CProjectUtils::getDataset<CMat>(wrapChildIndex);
            if(pDataset->hasDimension(DataDimension::VOLUME) || pDataset->hasDimension(DataDimension::TIME))
            {
                if(dataIndex == 0)
                    return getDatasetDataIndex(childIndex, dataIndex);

                dataIndex--;
            }
            else
            {
                if(dataIndex < pDataset->size())
                    return getDatasetDataIndex(childIndex, dataIndex);

                dataIndex -= pDataset->size();
            }
        }
    }
    return QModelIndex();
}

bool CProjectManager::isTimeDataItem(const QModelIndex &index) const
{
    // Check if item is stream, video or image sequence
    if(!index.isValid())
        return false;

    auto wrapInd = wrapIndex(index);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM)
        return true;
    else if(type == TreeItemType::IMAGE || type == TreeItemType::DATASET)
    {
        auto pDataset = CProjectUtils::getDataset<CMat>(wrapInd);
        return pDataset->hasDimension(DataDimension::TIME);
    }
    else
        return false;
}

QModelIndex CProjectManager::addFolder(const QModelIndex &itemIndex, const std::string& name, const std::string& path)
{
    return m_multiProject.addItem(itemIndex, std::make_shared<CFolderItem>(name, path));
}

void CProjectManager::addFolderContent(const QString &folder, const QModelIndex &index)
{
    QDir qdir(folder);
    QModelIndex folderIndex = addFolder(index, qdir.dirName().toStdString(), folder.toStdString());
    QFileInfoList filesInfo = qdir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoSymLinks|QDir::NoDotAndDotDot);
    QStringList imageFiles, videoFiles;

    for(int i=0; i<filesInfo.size(); ++i)
    {
        if(filesInfo[i].isDir())
            addFolderContent(filesInfo[i].absoluteFilePath(), folderIndex);
        else if(filesInfo[i].isFile())
        {
            QString dataFile = filesInfo[i].absoluteFilePath();
            std::string ext = Utils::File::extension(dataFile.toStdString());

            if(CDataImageIO::isImageFormat(ext))
                imageFiles.append(dataFile);
            else if(CDataVideoIO::isVideoFormat(ext, true))
                videoFiles.append(dataFile);
        }
    }

    if(!imageFiles.empty())
        addImages(folderIndex, imageFiles, std::make_pair(Relationship::MANY_TO_ONE, DataDimension::NONE));

    if(!videoFiles.empty())
        addVideos(folderIndex, videoFiles);
}

QModelIndex CProjectManager::addDataset(const QModelIndex &itemIndex, std::string name, IODataType type)
{
    QModelIndex addedItemIndex = QModelIndex();
    switch(type)
    {
        case IODataType::IMAGE:
        case IODataType::VIDEO:
            addedItemIndex = m_multiProject.addItem(itemIndex, std::make_shared<CDatasetItem<CMat>>(name, type));
            break;
        default: break;
    }
    return addedItemIndex;
}

QModelIndex CProjectManager::addDimension(const QModelIndex &itemIndex, const DataDimension dim)
{
    auto pSet = CProjectUtils::getDataset<CMat>(wrapIndex(itemIndex));
    if(!pSet)
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

    if(!pSet->hasDimension(dim))
    {
        //Find the dimension level
        size_t level = 0;
        QModelIndex tmpItemIndex = wrapIndex(itemIndex);
        auto pItem = static_cast<ProjectTreeItem*>(tmpItemIndex.internalPointer());

        while(pItem != nullptr && pItem->getTypeId() != TreeItemType::DATASET)
        {
            tmpItemIndex = tmpItemIndex.parent();
            pItem = static_cast<ProjectTreeItem*>(tmpItemIndex.internalPointer());
            level++;
        }
        pSet->insertDimension(dim, level);
        pSet->subset().clear();
    }
    else
        pSet->upDimension(dim, 1);

    return m_multiProject.addItem(itemIndex, std::make_shared<CDimensionItem>(dim, Utils::Data::getDimensionName(dim)));
}

QModelIndex CProjectManager::addImages(const QModelIndex &itemIndex, QStringList &files, const DatasetLoadPolicy &policy)
{
    QModelIndex imgIndex;
    if(itemIndex.isValid())
    {
        ProjectTreeItem* itemPtr = static_cast<ProjectTreeItem*>(wrapIndex(itemIndex).internalPointer());
        if(itemPtr->getTypeId() == TreeItemType::DATASET)
        {
            if(policy.first == Relationship::MANY_TO_ONE && policy.second != DataDimension::NONE)
            {
                auto dimItemIndex = addDimension(itemIndex, policy.second);
                imgIndex = addImagesToDimension(dimItemIndex, files);
            }
            else
                imgIndex = addImagesToDataset(itemIndex, files);
        }
        else
        {
            if(policy.first == Relationship::ONE_TO_ONE)
            {
                for(int i=0; i<files.size(); ++i)
                {
                    boost::filesystem::path path(files[i].toStdString());
                    auto datasetIndex = addDataset(itemIndex, path.parent_path().stem().string(), IODataType::IMAGE);
                    imgIndex = addImagesToDataset(datasetIndex, QStringList(files[i]));
                }
            }
            else if(policy.first == Relationship::MANY_TO_ONE)
            {                
                boost::filesystem::path path(files[0].toStdString());
                auto datasetIndex = addDataset(itemIndex, path.parent_path().stem().string(), IODataType::IMAGE);

                if(policy.second == DataDimension::NONE)
                    imgIndex = addImagesToDataset(datasetIndex, files);
                else
                {
                    auto dimItemIndex = addDimension(datasetIndex, policy.second);
                    checkSequenceFileNames(policy.second, files);
                    imgIndex = addImagesToDimension(dimItemIndex, files);
                }
            }
        }
    }
    return imgIndex;
}

QModelIndex CProjectManager::addVideos(const QModelIndex& itemIndex, const QStringList& files)
{
    QModelIndex index;
    if(itemIndex.isValid())
    {
        for(int i=0; i<files.size(); ++i)
        {
            boost::filesystem::path path(files[i].toStdString());
            // Add dataset de type video
            auto datasetIndex = addDataset(itemIndex, path.stem().string(), IODataType::VIDEO);
            // Add dimension TIME to dataset
            auto dimIndex = addDimension(datasetIndex, DataDimension::TIME);

            auto pSet = CProjectUtils::getDataset<CMat>(wrapIndex(dimIndex));
            if(!pSet)
                throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

            // Set dimension IMAGE to 1 element for managing display
            pSet->setDimension(DataDimension::IMAGE, 1);
            pSet->appendFile(files[i].toStdString());

            index = m_multiProject.addItem(dimIndex, std::make_shared<CVideoItem>(path.stem().string(), path.string()));
        }
    }
    return index;
}

QModelIndex CProjectManager::addStream(const QModelIndex& itemIndex, const QString& cameraId, const QString& cameraName)
{
    if(itemIndex.isValid())
    {
        // Add dataset de type video
        auto datasetIndex = addDataset(itemIndex, "Live", IODataType::VIDEO);
        // Add dimension TIME to dataset
        auto dimIndex = addDimension(datasetIndex, DataDimension::TIME);

        auto pSet = CProjectUtils::getDataset<CMat>(wrapIndex(dimIndex));
        if(!pSet)
            throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);
        // Set dimension IMAGE to 1 element for managing display
        pSet->setDimension(DataDimension::IMAGE, 1);
        pSet->appendFile(cameraId.toStdString());

        auto index = m_multiProject.addItem(dimIndex, std::make_shared<CLiveStreamItem>(cameraName.toStdString(), cameraId.toStdString()));
        emit doUpdateIndex(index);
    }
    return QModelIndex();
}

void CProjectManager::setManagers(CGraphicsManager *pGraphicsMgr, CRenderManager *pRenderMgr, CResultManager *pResultsMgr,
                                  CWorkflowManager *pWorkflowMgr, CProgressBarManager* pProgressMgr,
                                  CMainDataManager* pDataMgr)
{
    m_pGraphicsMgr = pGraphicsMgr;
    m_pRenderMgr = pRenderMgr;
    m_pResultsMgr = pResultsMgr;
    m_pWorkflowMgr = pWorkflowMgr;
    m_pDataMgr = pDataMgr;
    m_pDataMgr->setProgressSignalHandler(&m_progressSignal);
    m_pProgressMgr = pProgressMgr;
}

QModelIndex CProjectManager::getDatasetIndex(const QModelIndex& index) const
{
    if(!index.isValid())
        return QModelIndex();

    auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    if(itemPtr->getTypeId() == TreeItemType::DATASET)
        return index;
    else
    {
        QModelIndex searchIndex = m_multiProject.parent(index);
        itemPtr = static_cast<ProjectTreeItem*>(wrapIndex(searchIndex).internalPointer());

        while(itemPtr != nullptr && itemPtr->getTypeId() != TreeItemType::DATASET)
        {
            searchIndex = searchIndex.parent();
            itemPtr = static_cast<ProjectTreeItem*>(wrapIndex(searchIndex).internalPointer());
        }

        if(itemPtr)
            return searchIndex;
    }
    return QModelIndex();
}

QModelIndex CProjectManager::getCurrentDataItemIndex() const
{
    return m_currentDataItemIndex;
}

QModelIndex CProjectManager::getCurrentVideoItemIndex() const
{
    // In case of image sequence, we must have a single video player,
    // so we need to ensure a single mapping between model index and video player.
    // Thus, this method returns the current data model index for videos
    // and the first image model index for image sequence.
    auto wrapInd = wrapIndex(m_currentDataItemIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());

    if(pItem->getTypeId() == TreeItemType::IMAGE)
    {
        auto datasetIndex = getDatasetIndex(m_currentDataItemIndex);
        return getDatasetDataIndex(datasetIndex, 0);
    }
    else
        return m_currentDataItemIndex;
}

CProjectModel *CProjectManager::getModel(const QModelIndex &itemIndex)
{
    if(!itemIndex.isValid())
        return nullptr;

    QModelIndex index = wrapIndex(itemIndex);
    for(size_t i=0; i<m_projectList.size(); ++i)
    {
        if(m_projectList[i] == index.model())
            return m_projectList[i];
    }
    return nullptr;
}

CProjectModel *CProjectManager::getModel(size_t index)
{
    if(index < m_projectList.size())
        return m_projectList[index];
    else
        return nullptr;
}

CMultiProjectModel *CProjectManager::getMultiModel()
{
    return &m_multiProject;
}

size_t CProjectManager::getProjectIndex(const QModelIndex &itemIndex)
{
    if(!itemIndex.isValid())
        return 0;

    QModelIndex index = wrapIndex(itemIndex);
    for(size_t i=0; i<m_projectList.size(); ++i)
    {
        if(m_projectList[i] == index.model())
            return i;
    }
    return 0;
}

QModelIndex CProjectManager::getImageSubTreeRootIndex(const TreeItemType& type) const
{
    if(!m_currentDataItemIndex.isValid())
        return QModelIndex();

    //Iterate through all child items of current image
    int childCount = m_multiProject.rowCount(m_currentDataItemIndex);
    for(int i=0; i<childCount; ++i)
    {
        auto childIndex = m_multiProject.index(i, 0, m_currentDataItemIndex);
        auto pChild = static_cast<ProjectTreeItem*>(wrapIndex(childIndex).internalPointer());
        assert(pChild);

        if(pChild->getTypeId() == static_cast<size_t>(type))
            return childIndex;
    }
    return QModelIndex();
}

#include "moc_CProjectManager.cpp"
