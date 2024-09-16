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

 #include "CWorkflowManager.h"
#include <QtConcurrent/QtConcurrent>
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Model/Process/CProcessManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Model/Results/CResultManager.h"
#include "Model/Data/CMainDataManager.h"
#include "Model/Settings/CSettingsManager.h"
#include "IO/CGraphicsInput.h"
#include "IO/CImageIO.h"
#include "IO/CVideoIO.h"
#include "IO/CPathIO.h"

CWorkflowManager::CWorkflowManager() :
    m_runMgr(&m_inputs),
    m_inputViewMgr(&m_inputs, WorkflowInputViewMode::ORIGIN)
{
    initGlobalConnections();
}

CWorkflowManager::~CWorkflowManager()
{
}

std::string CWorkflowManager::getWorkflowName() const
{
    if(m_pWorkflow)
        return m_pWorkflow->getName();
    else
        return "";
}

std::string CWorkflowManager::getWorkflowDescription() const
{
    if(m_pWorkflow)
        return m_pWorkflow->getDescription();
    else
        return "";
}

std::string CWorkflowManager::getWorkflowKeywords() const
{
    if (m_pWorkflow)
        return m_pWorkflow->getKeywords();
    else
        return std::string();
}

WorkflowVertex CWorkflowManager::getRootId() const
{
    if(m_pWorkflow)
        return m_pWorkflow->getRootId();
    else
        return boost::graph_traits<WorkflowGraph>::null_vertex();
}

WorkflowTaskIOPtr CWorkflowManager::getInput(size_t index) const
{
    if(m_pWorkflow)
        return m_pWorkflow->getInput(index);
    else
        return nullptr;
}

IODataType CWorkflowManager::getInputDataType(size_t index) const
{
    if(m_pWorkflow)
        return m_pWorkflow->getInputDataType(index);
    else
        return IODataType::NONE;
}

WorkflowInputViewMode CWorkflowManager::getInputViewMode() const
{
    return m_inputViewMode;
}

void CWorkflowManager::setManagers(QNetworkAccessManager *pNetMgr, CProcessManager *pProcessMgr,
                                   CProjectManager *pProjectMgr, CGraphicsManager *pGraphicsMgr,
                                   CResultManager *pResultsMgr, CMainDataManager *pDataMgr,
                                   CProgressBarManager *pProgressMgr, CSettingsManager *pSettingsMgr)
{
    m_pProcessMgr = pProcessMgr;
    m_pProjectMgr = pProjectMgr;
    m_pGraphicsMgr = pGraphicsMgr;
    m_pResultsMgr = pResultsMgr;
    m_pProgressMgr = pProgressMgr;
    m_pDataMgr = pDataMgr;
    m_pSettingsMgr = pSettingsMgr;
    m_runMgr.setManagers(pProjectMgr, pDataMgr, pProgressMgr);
    m_dbMgr.setManagers(pSettingsMgr);
    m_inputViewMgr.setManagers(pProjectMgr);
    m_scaleMgr.setManagers(pNetMgr, pProgressMgr);

    if(m_pProgressMgr)
    {
        connect(this, &CWorkflowManager::doSetElapsedTime, m_pProgressMgr, &CProgressBarManager::onSetElapsedTime);
        connect(this, &CWorkflowManager::doSetMessage, m_pProgressMgr, &CProgressBarManager::onSetMessage);
    }
}

void CWorkflowManager::setWorkflowName(const std::string &name)
{
    if(m_pWorkflow)
        m_pWorkflow->setName(name);
}

void CWorkflowManager::setWorkflowKeywords(const std::string &keywords)
{
    if(m_pWorkflow)
        m_pWorkflow->setKeywords(keywords);
}

void CWorkflowManager::setWorkflowDescription(const std::string &description)
{
    if(m_pWorkflow)
        m_pWorkflow->setDescription(description);
}

void CWorkflowManager::setInputViewMode(WorkflowInputViewMode mode)
{
    if(m_pWorkflow == nullptr)
        return;

    m_inputViewMode = mode;
    m_inputViewMgr.setViewMode(mode);

    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
        m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getRootId()));
    else
        m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId()));
}

void CWorkflowManager::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
    m_scaleMgr.setCurrentUser(user);
}

void CWorkflowManager::setCurrentTaskAutoSave(size_t outputIndex, bool bAutoSave)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    auto outputPtr = taskPtr->getOutput(outputIndex);
    if(outputPtr)
    {
        outputPtr->setAutoSave(bAutoSave);
        if(m_pWorkflow->isBatchMode())
        {
            taskPtr->setActionFlag(CWorkflowTask::ActionFlag::OUTPUT_AUTO_EXPORT, taskPtr->isAutoSave());
            doUpdateTaskItemView(taskPtr, m_pWorkflow->getActiveTaskId());
        }
    }
}

void CWorkflowManager::setCurrentTaskSaveFolder(const std::string &path)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    taskPtr->setOutputFolder(path);
}

void CWorkflowManager::setCurrentTaskSaveFormat(size_t outputIndex, size_t formatIndex)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    auto outputPtr = taskPtr->getOutput(outputIndex);
    if(outputPtr)
    {
        auto formats = outputPtr->getPossibleSaveFormats();
        if(formatIndex < formats.size())
            outputPtr->setSaveFormat(formats[formatIndex]);
    }
}

void CWorkflowManager::setCurrentTaskSaveFormat(size_t outputIndex, DataFileFormat format)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    auto outputPtr = taskPtr->getOutput(outputIndex);
    if(outputPtr)
        outputPtr->setSaveFormat(format);
}

void CWorkflowManager::setCurrentTaskExposedOutputDescription(int index, const std::string &description)
{
    auto taskId = getActiveTaskId();
    m_pWorkflow->setExposedOutputDescription(taskId, index, description);
}

void CWorkflowManager::setWorkflowConfig(const std::string &key, const std::string &value)
{
    if(m_pWorkflow)
        m_pWorkflow->setCfgEntry(key, value);
}

int CWorkflowManager::getCurrentFPS() const
{
    return m_currentFPS;
}

QModelIndex CWorkflowManager::getCurrentVideoInputModelIndex() const
{
    if(m_currentVideoInputIndex < m_inputs.size())
        return m_inputs[m_currentVideoInputIndex].getModelIndex(0);
    else
        return QModelIndex();
}

std::vector<int> CWorkflowManager::getDisplayedInputIndices(const WorkflowTaskPtr &taskPtr, const std::set<IODataType>& types) const
{
    std::vector<int> inputIndices;

    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
        inputIndices.push_back(m_currentVideoInputIndex);
    else
    {
        size_t index = 0;
        for(size_t i=0; i<taskPtr->getInputCount(); ++i)
        {
            auto inputPtr = taskPtr->getInput(i);
            auto it = types.find(inputPtr->getDataType());

            if(it != types.end())
                inputIndices.push_back(index++);
        }
    }
    return inputIndices;
}

CWorkflow::ExposedParams CWorkflowManager::getWorflowExposedParams() const
{
    if (m_pWorkflow == nullptr)
    {
        qCCritical(logWorkflow()).noquote() << tr("No workflow instance found.");
        return CWorkflow::ExposedParams();
    }
    return m_pWorkflow->getExposedParameters();
}

CWorkflowScaleManager *CWorkflowManager::getScaleManager()
{
    return &m_scaleMgr;
}

void CWorkflowManager::notifyViewShow()
{
    loadWorkflows();
    emit doSetManager(this);
}

void CWorkflowManager::notifyGraphicsChanged()
{
    m_runMgr.notifyGraphicsChanged();
}

void CWorkflowManager::enableAutoLoadBatchResults(bool bEnable)
{
    m_bAutoLoadBatchResult = bEnable;
}

void CWorkflowManager::enableWholeVideo(bool bEnable)
{
    if (m_pWorkflow)
    {
        m_pWorkflow->setCfgEntry("WholeVideo", std::to_string(bEnable));
        if (bEnable)
            rootConnectionChanged();
    }
}

void CWorkflowManager::beforeProjectClose(bool bWithCurrentImage)
{
    if(bWithCurrentImage == true)
    {
        if(m_interactionLayerInfo.m_pLayer != nullptr)
        {
            assert(m_pGraphicsMgr);
            m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
            m_interactionLayerInfo.m_pLayer = nullptr;
        }

        if(m_pWorkflow)
        {
            for(size_t i=0; i<m_inputs.size(); ++i)
                m_inputs[i].clearModelIndex();

            clearAllTasks();
            m_pWorkflow->clearInputs();
            emit doInputsCleared();
        }
        else
            initInputs();
    }
}

void CWorkflowManager::beforeDataDeleted(const QModelIndex &index)
{
    for(size_t i=0; i<m_inputs.size(); ++i)
    {
        if(m_inputs[i].contains(index))
        {
            if(m_inputs[i].getMode() == WorkflowInputMode::CURRENT_DATA)
            {
                assert(m_pGraphicsMgr);
                m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
                m_interactionLayerInfo.m_pLayer = nullptr;
            }

            m_inputs[i].clearModelIndex();
            if(m_pWorkflow)
            {
                m_pWorkflow->clearInputData(i);
                emit doInputChanged(i);
            }
        }
    }
}

void CWorkflowManager::beforeDataDeleted(const std::vector<QModelIndex> &indexes)
{
    for(size_t i=0; i<indexes.size(); ++i)
        beforeDataDeleted(indexes[i]);
}

bool CWorkflowManager::isRoot(const WorkflowVertex &id) const
{
    if(m_pWorkflow)
        return m_pWorkflow->isRoot(id);
    else
        return false;
}

bool CWorkflowManager::isWorkflowExists() const
{
    return m_pWorkflow != nullptr;
}

bool CWorkflowManager::isWorkflowModified() const
{
    if(m_pWorkflow)
    {
        try
        {
            return m_pWorkflow->isModified();
        }
        catch(std::exception& e)
        {
            qCWarning(logWorkflow).noquote() << QString::fromStdString(e.what());
            return false;
        }
    }
    else
        return false;
}

bool CWorkflowManager::isWorkflowRunning() const
{
    return m_runMgr.isRunning();
}

bool CWorkflowManager::isBatchInput(size_t index) const
{
    if(index >= m_inputs.size())
        return false;

    auto type = m_inputs[index].getType();
    if(m_inputs[index].getModelIndicesCount() > 1)
        return true;
    else if(type == TreeItemType::DATASET)
        return true;
    else if(type == TreeItemType::FOLDER)
    {
        auto types = m_runMgr.getOriginTargetDataTypes(index);
        if(types.empty() || types.find(IODataType::PROJECT_FOLDER) != types.end() || types.find(IODataType::FOLDER_PATH) != types.end())
            return false;
        else
            return true;
    }
    else
        return false;
}

bool CWorkflowManager::isCurrentTaskOutputExposed(size_t index) const
{
    auto activeTaskId = getActiveTaskId();
    std::vector<CWorkflowOutput> outputs = m_pWorkflow->getExposedOutputs();

    for (size_t i=0; i<outputs.size(); ++i)
    {
        auto taskId = reinterpret_cast<WorkflowVertex>(outputs[i].getTaskId());
        if (taskId == activeTaskId && outputs[i].getTaskOutputIndex() == index)
            return true;
    }
    return false;
}

void CWorkflowManager::createWorkflow(const std::string &name, const std::string &keywords, const std::string &description)
{
    assert(m_pProcessMgr);
    assert(m_pGraphicsMgr);
    try
    {
        m_pWorkflow = std::make_unique<CWorkflow>(name, &m_pProcessMgr->m_registry, m_pGraphicsMgr->getContext());
        m_pWorkflow->setKeywords(keywords);
        m_pWorkflow->setDescription(description);
        m_pWorkflow->setOutputFolder(m_pSettingsMgr->getWorkflowSaveFolder() + name + "/");
        initWorkflow();
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

WorkflowTaskWidgetPtr CWorkflowManager::createTaskWidget(const WorkflowTaskPtr& pTask)
{
    assert(pTask);
    return m_pProcessMgr->createWidget(pTask->getName(), pTask->getParam());
}

void CWorkflowManager::runWorkflow()
{
    if(m_pWorkflow == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
        clearAllTasks();

    m_protocolTimer.start();
    m_runMgr.run();
}

void CWorkflowManager::runWorkflowFromActiveTask()
{
    if(m_pWorkflow == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
    {
        clearFrom(m_pWorkflow->getActiveTaskId());
        m_protocolTimer.start();
        m_runMgr.runFromActiveTask();
    }
}

void CWorkflowManager::runWorkflowToActiveTask()
{
    if(m_pWorkflow == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
    {
        clearTo(m_pWorkflow->getActiveTaskId());
        m_protocolTimer.start();
        m_runMgr.runToActiveTask();
    }
}

void CWorkflowManager::stopWorkflow()
{
    m_runMgr.stop();
}

void CWorkflowManager::beforeGraphicsLayerRemoved(CGraphicsLayer *pLayer)
{
    if(m_pWorkflow && pLayer)
    {
        std::set<const CGraphicsLayer*> layersToBeRemoved;
        auto childLayers = pLayer->getChildLayers();

        layersToBeRemoved.insert(pLayer);
        for(int i=0; i<childLayers.size(); ++i)
            layersToBeRemoved.insert(childLayers[i]);

        auto vertexRangeIt = m_pWorkflow->getVertices();
        for(auto it=vertexRangeIt.first; it!=vertexRangeIt.second; ++it)
        {
            auto pTask = m_pWorkflow->getTask(*it);
            if(pTask)
            {
                auto inputs = pTask->getInputs();
                for(size_t i=0; i<inputs.size(); ++i)
                {
                    if(inputs[i]->getDataType() == IODataType::INPUT_GRAPHICS)
                    {
                        auto pGraphicsInput = std::static_pointer_cast<CGraphicsInput>(inputs[i]);
                        auto itLayer = layersToBeRemoved.find(pGraphicsInput->getLayer());

                        if(itLayer != layersToBeRemoved.end())
                        {
                            pTask->clearInputData(i);
                            emit doInputAssigned(*it, (int)i, false);
                        }
                    }
                }
            }
        }
    }
}

void CWorkflowManager::saveWorkflow()
{
    if(m_pWorkflow)
    {
        try
        {
            QString protocolName = QString::fromStdString(m_pWorkflow->getName());
            auto it = m_mapWorkflowNameToId.find(protocolName);
            if(it != m_mapWorkflowNameToId.end())
            {
                //Overwrite existing protocol
                m_dbMgr.remove(it->second);
                m_dbMgr.save(m_pWorkflow);
            }
            else
            {
                //New internal protocol
                int protocolDbId = m_dbMgr.save(m_pWorkflow);
                if(m_pModel->insertRow(m_pModel->rowCount()))
                {
                    QModelIndex index = m_pModel->index(m_pModel->rowCount() - 1, 0);
                    m_pModel->setData(index, protocolName);
                    m_mapWorkflowNameToId.insert(std::make_pair(protocolName, protocolDbId));
                    m_mapWorkflowIdToName.insert(std::make_pair(protocolDbId, protocolName));
                }
            }
            m_pWorkflow->updateHash();
            emit doNotifyWorkflowSaved();
            emit doNewWorkflowNotification(tr("Workflow has been saved."), Notification::INFO);
        }
        catch(std::exception& e)
        {
            qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CWorkflowManager::saveWorkflow(const QString &path)
{
    if(m_pWorkflow)
    {
        try
        {
            auto ext = Utils::File::extension(path.toStdString());
            if(ext == ".pcl")
                m_dbMgr.save(m_pWorkflow, path);
            else
                m_pWorkflow->save(path.toStdString());

            emit doNewWorkflowNotification(tr("Workflow has been exported."), Notification::INFO);
        }
        catch(std::exception& e)
        {
            qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CWorkflowManager::loadWorkflow(const QString &path)
{
    assert(m_pProcessMgr);
    assert(m_pGraphicsMgr);

    try
    {
        CPyEnsureGIL gil;
        if(m_pWorkflow)
            emit doCloseWorkflow();

        auto ext = Utils::File::extension(path.toStdString());
        if(ext == ".pcl")
            m_pWorkflow = m_dbMgr.load(path, m_pProcessMgr, m_pGraphicsMgr->getContext());
        else
        {
            m_pWorkflow = std::make_unique<CWorkflow>("", &m_pProcessMgr->m_registry, m_pGraphicsMgr->getContext());
            m_pWorkflow->setOutputFolder(m_pSettingsMgr->getWorkflowSaveFolder() + m_pWorkflow->getName() + "/");
            m_pWorkflow->load(path.toStdString());
        }

        if(m_pWorkflow)
            initWorkflow();
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::loadImageWorkflows(const QModelIndex &imageIndex)
{
    assert(m_pProjectMgr);

    if(imageIndex.isValid() == false)
        qCCritical(logWorkflow).noquote() << tr("Invalid image index");

    auto imgItemPtr = CProjectUtils::getImageItem(m_pProjectMgr->wrapIndex(imageIndex));
    if(imgItemPtr == nullptr)
        qCCritical(logWorkflow).noquote() << tr("Invalid image item");

    if(m_pImageModel == nullptr)
        m_pImageModel = new QStringListModel(this);

    QStringList protocolNames;
    auto protocolIds = imgItemPtr->getWorkflowDbIds();

    for(size_t i=0; i<protocolIds.size(); ++i)
    {
        auto it = m_mapWorkflowIdToName.find(protocolIds[i]);
        if(it != m_mapWorkflowIdToName.end())
            protocolNames.push_back(it->second);
    }
    m_pImageModel->setStringList(protocolNames);
    emit doSetNamesFromImageModel(m_pImageModel);
}

void CWorkflowManager::requestScaleProjects()
{
    m_scaleMgr.requestProjects();
}

void CWorkflowManager::publishWorkflow(const QString& name, const QString& description, bool bNewProject,
                                       const QString& projectName, const QString &projectDescription, const QString& namespacePath)
{
    assert(m_pWorkflow);
    m_pWorkflow->setName(name.toStdString());
    m_pWorkflow->setDescription(description.toStdString());
    QString filename = Utils::File::conformName(name + "_" + QDateTime::currentDateTime().toString(Qt::ISODate));
    QString tmpPath = QString("%1/Workflows/%2.json")
                        .arg(Utils::IkomiaApp::getQIkomiaFolder())
                        .arg(filename);

    try
    {
        m_pWorkflow->save(tmpPath.toStdString());
        m_scaleMgr.publishWorkflow(tmpPath, bNewProject, projectName, projectDescription, namespacePath);
        emit doNewWorkflowNotification(tr("Workflow %1 has been published successfully.").arg(name), Notification::INFO);
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::onWorkflowClosed()
{
    if(m_pWorkflow == nullptr)
        return;

    // VIDEO Stop wait thread if running
    m_runMgr.stopWaitThread();

    // Wait for the end of live protocol before closing
    stopWorkflow();
    m_runMgr.waitForWorkflow();

    //Reset input view
    size_t inputNb = m_pWorkflow->getInputCount();
    if(inputNb > 1)
        m_inputViewMgr.clear(m_pWorkflow->getTask(m_pWorkflow->getRootId()));

    if(inputNb > 1 && m_inputViewMode == WorkflowInputViewMode::CURRENT)
        m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getRootId()));

    //Workflow must be set to null before reloading source data
    m_runMgr.setWorkflow(nullptr);
    m_pWorkflow = nullptr;
    m_pDataMgr->reloadCurrent();

    if(m_interactionLayerInfo.m_pLayer)
    {
        assert(m_pGraphicsMgr);
        m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
        m_interactionLayerInfo.m_pLayer = nullptr;
    }
}

void CWorkflowManager::onSearchWorkflow(const QString &text)
{
    QStringList protocolNames = m_dbMgr.searchWorkflows(text);
    m_pModel->setStringList(protocolNames);
}

void CWorkflowManager::onOutputChanged()
{
    assert(m_pResultsMgr);
    auto taskId = m_pWorkflow->getActiveTaskId();
    auto taskPtr = m_pWorkflow->getTask(taskId);
    QModelIndex inputIndex = m_inputs[0].getModelIndex(0);
    m_pResultsMgr->manageOutputs(taskPtr, taskId, inputIndex);
}

void CWorkflowManager::onUpdateTaskGraphicsInput(const GraphicsInputPtr &inputPtr)
{
    assert(m_pGraphicsMgr);
    assert(m_pProjectMgr);

    auto pLayer = m_pGraphicsMgr->getLayer(m_pGraphicsMgr->getCurrentLayerIndex());
    if(pLayer)
        inputPtr->setLayer(pLayer);
    else
        inputPtr->clearData();
}

void CWorkflowManager::onUpdateProcessInfo(bool bFullEdit, const CTaskInfo &info)
{
    assert(m_pProcessMgr);
    m_pProcessMgr->onUpdateProcessInfo(bFullEdit, info);
    auto pTask = m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId());
    emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
}

void CWorkflowManager::onNotifyVideoStart(int frameCount)
{
    if(m_pWorkflow)
    {
        m_pWorkflow->notifyVideoStart(frameCount);
        m_protocolTimer.start();
    }
}

void CWorkflowManager::onSendProcessAction(const WorkflowTaskPtr& pTask, int flags)
{
    try
    {
        pTask->executeActions(flags);
    }
    catch (std::exception& e)
    {
        m_runMgr.workflowErrorHandling(e);
    }
}

void CWorkflowManager::onStopThread()
{
    m_runMgr.stopWaitThread();
}

void CWorkflowManager::onAllProcessReloaded()
{
    if(m_pWorkflow == nullptr)
        return;

    assert(m_pProcessMgr);
    CPyEnsureGIL gil;
    auto rangeIt = m_pWorkflow->getVertices();

    for(auto it=rangeIt.first; it!=rangeIt.second; ++it)
    {
        auto taskPtr = m_pWorkflow->getTask(*it);
        auto processInfo = m_pProcessMgr->getProcessInfo(taskPtr->getName());

        if(processInfo.getName() != taskPtr->getName())
            continue;

        if(processInfo.isInternal() == false)
        {
            //Current task is a plugin so create new instance
            //We pass nullptr as parameter because we want to take into account possible modifications
            //in implementation side -> in this case new instance must be created
            auto newTaskPtr = m_pProcessMgr->createObject(taskPtr->getName(), nullptr);
            if(newTaskPtr)
            {
                m_pWorkflow->replaceTask(newTaskPtr, *it);
                if(m_pWorkflow->getActiveTaskId() == *it)
                    emit doUpdateTaskInfo(newTaskPtr, processInfo);
            }
        }
    }
}

void CWorkflowManager::onProcessReloaded(const QString &name)
{
    assert(m_pProcessMgr);
    if(m_pWorkflow == nullptr)
        return;

    auto taskIds = m_pWorkflow->getTaskIdList(name.toStdString());
    for (size_t i=0; i<taskIds.size(); ++i)
    {
        auto taskPtr = m_pWorkflow->getTask(taskIds[i]);
        auto processInfo = m_pProcessMgr->getProcessInfo(taskPtr->getName());
        //Create new instance
        //We pass nullptr as parameter because we want to take into account possible modifications
        //in implementation side -> in this case new instance must be created
        auto newTaskPtr = m_pProcessMgr->createObject(taskPtr->getName(), nullptr);
        if (newTaskPtr)
        {
            m_pWorkflow->replaceTask(newTaskPtr, taskIds[i]);
            if (m_pWorkflow->getActiveTaskId() == taskIds[i])
                emit doUpdateTaskInfo(newTaskPtr, processInfo);
        }
    }
}

void CWorkflowManager::onCloseApp()
{
    m_runMgr.stop();
    m_runMgr.stopWaitThread();
}

void CWorkflowManager::onWorkflowLive(int inputIndex, bool bNewSequence)
{
    if(m_pWorkflow == nullptr)
        return;

    try
    {
        setWorkflowInput(inputIndex, bNewSequence);
        m_runMgr.runLive(inputIndex);
    }
    catch(std::exception& e)
    {
        emit doStopVideo();
        qCritical(logWorkflow).noquote() << e.what();
    }
}

std::vector<WorkflowVertex> CWorkflowManager::getAllChilds(const WorkflowVertex& id) const
{
    if(m_pWorkflow)
        return m_pWorkflow->getAllChilds(id);
    else
        return std::vector<WorkflowVertex>();
}

QStringList CWorkflowManager::getWorkflowNames() const
{
    return m_pModel->stringList();
}

WorkflowTaskPtr CWorkflowManager::getActiveTask() const
{
    if(m_pWorkflow == nullptr)
        return nullptr;
    else
    {
        auto id = m_pWorkflow->getActiveTaskId();
        auto pCurrentTask = m_pWorkflow->getTask(id);
        return pCurrentTask;
    }
}

WorkflowVertex CWorkflowManager::getActiveTaskId() const
{
    if(m_pWorkflow)
        return m_pWorkflow->getActiveTaskId();
    else
        return boost::graph_traits<WorkflowGraph>::null_vertex();
}

int CWorkflowManager::getWorkflowDbId() const
{
    if(m_pWorkflow == nullptr)
        return -1;

    auto it = m_mapWorkflowNameToId.find(QString::fromStdString(m_pWorkflow->getName()));
    if(it == m_mapWorkflowNameToId.end())
        return -1;
    else
        return it->second;
}

CWorkflowInputViewManager *CWorkflowManager::getInputViewManager()
{
    return &m_inputViewMgr;
}

void CWorkflowManager::onApplyProcess(const QModelIndex &itemIndex, const std::string &processName, const WorkflowTaskParamPtr &pParam)
{
    assert(m_pProcessMgr && m_pProjectMgr);

    try
    {
        if(m_pWorkflow == nullptr)
            createWorkflow(tr("New workflow").toStdString());

        // Store current item index
        if(itemIndex.isValid() && !m_inputs.empty())
            m_inputs[0].setModelIndex(itemIndex, 0);

        WorkflowVertex newTaskId = addProcess(processName, pParam);
        //Select the added task to ease next connection
        m_pWorkflow->setActiveTask(newTaskId);
        emit doSetActiveTask(newTaskId);
        //Update task info
        auto pTask = m_pWorkflow->getTask(newTaskId);
        emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
        //Display the next possible task to add to the protocol (empty informative item)
        emit doUpdateCandidateTask();
        //Update protocol module icon to notify change
        emit doSetWorkflowChangedIcon();

        m_runMgr.addSequentialRun(newTaskId);
        m_runMgr.runSequentialTask(newTaskId);
    }
    catch(std::exception& e)
    {
        //Connection failed
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
        m_pWorkflow->deleteTask(m_pWorkflow->getLastTaskId());
    }
}

void CWorkflowManager::onActiveTaskChanged(const WorkflowVertex &id)
{    
    if(m_pWorkflow == nullptr)
        return;

    //Test if new task
    auto previousId = m_pWorkflow->getActiveTaskId();
    if(previousId == id)
        return;

    m_pWorkflow->setActiveTask(id);
    auto pTask = m_pWorkflow->getTask(id);
    assert(pTask);

    if( m_inputViewMode == WorkflowInputViewMode::CURRENT)
        m_inputViewMgr.manageInputs(pTask);

    //Get output
    if(m_pResultsMgr)
    {
        assert(m_pProjectMgr);
        auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
        m_pResultsMgr->manageOutputs(pTask, id, currentModelIndex);
    }

    //Display the task information
    updateDataInfo();
    updateExecTimeInfo(id);
    emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));

    //Display the next possible task to add to the protocol (empty informative item)
    emit doUpdateCandidateTask();
}

void CWorkflowManager::onDeleteTask(const WorkflowVertex &id)
{
    assert(m_pWorkflow && m_pProjectMgr);

    //Select first parent
    WorkflowVertex parentId = m_pWorkflow->getRootId();
    auto parents = m_pWorkflow->getParents(id);

    if(parents.size() > 0)
        parentId = parents.front();

    auto pTask = m_pWorkflow->getTask(parentId);
    if(pTask != nullptr)
    {
        //Set the parent task as active
        m_pWorkflow->setActiveTask(parentId);
        emit doSetActiveTask(parentId);
        //Display the task information
        updateExecTimeInfo(parentId);
        emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
        //Display the next possible task to add to the protocol (empty informative item)
        emit doUpdateCandidateTask();
        //Update results
        auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
        m_pResultsMgr->manageOutputs(pTask, parentId, currentModelIndex);
    }

    bool bRootChanged = false;
    if(m_pWorkflow->isConnectedToRoot(id))
        bRootChanged = true;

    //Delete task
    m_pWorkflow->deleteTask(id);

    if(bRootChanged)
        rootConnectionChanged();
}

void CWorkflowManager::onDeleteEdge(const WorkflowEdge &id)
{
    assert(m_pWorkflow);
    bool bRootChanged = false;
    auto srcTaskId = m_pWorkflow->getEdgeSource(id);

    if(srcTaskId == m_pWorkflow->getRootId())
        bRootChanged = true;

    m_pWorkflow->deleteEdge(id);

    if(bRootChanged)
        rootConnectionChanged();
}

void CWorkflowManager::onConnectTask(const WorkflowVertex &srcId, int srcIndex, const WorkflowVertex &dstId, int dstIndex)
{
    assert(m_pWorkflow);

    try
    {
        auto edgeId = m_pWorkflow->connect(srcId, srcIndex, dstId, dstIndex);
        emit doAddConnection(edgeId, srcId, srcIndex, dstId, dstIndex);

        if(srcId == m_pWorkflow->getRootId())
            rootConnectionChanged();
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::onRunFromActiveTask(const WorkflowTaskParamPtr &pParam)
{
    assert(m_pWorkflow);
    auto id = m_pWorkflow->getActiveTaskId();
    auto pTask = m_pWorkflow->getTask(id);

    if(pTask == nullptr)
        qCCritical(logWorkflow).noquote() << tr("Invalid workflow current task");
    else
    {
        pTask->setParam(pParam);
        // Update task item view in order to take into account #port changes
        emit doUpdateTaskItemView(pTask, id);
        runWorkflowFromActiveTask();
    }
}

void CWorkflowManager::onRunWorkflowFinished()
{
    assert(m_pWorkflow && m_pProjectMgr);

    emit doSetElapsedTime(m_runMgr.getTotalElapsedTime());
    updateProcessingFPS();
    emit m_pWorkflow->getSignalRawPtr()->doFinish();
    auto taskId = m_pWorkflow->getActiveTaskId();
    auto pTask = m_pWorkflow->getTask(taskId);
    auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();

    if(m_pWorkflow->isBatchMode())
    {
        if(m_bAutoLoadBatchResult)
            m_pProjectMgr->onLoadFolder(QString::fromStdString(m_pWorkflow->getLastRunFolder()), currentModelIndex);
    }
    else if(pTask && m_pResultsMgr)
    {
        if(m_inputViewMode == WorkflowInputViewMode::CURRENT)
            m_inputViewMgr.manageInputs(pTask);
        else
        {
            // Video only -> update input video frame and result frame at the same time
            m_inputViewMgr.manageOriginVideoInput(m_currentVideoInputIndex);
        }
        updateDataInfo();
        m_pResultsMgr->manageOutputs(pTask, taskId, currentModelIndex);
    }
    emit doWorkflowFinished();
}

void CWorkflowManager::onRunWorkflowFailed()
{
    emit doWorkflowFailed();
}

void CWorkflowManager::onInputDataChanged(const QModelIndex& itemIndex, int inputIndex, bool bNewSequence)
{
    assert(m_pProjectMgr);
    try
    {
        if(m_pWorkflow == nullptr)
            return;

        if(m_runMgr.isRunning())
            return;

        if(m_inputs.empty())
            initInputs();

        auto wrapInd = m_pProjectMgr->wrapIndex(itemIndex);
        auto pItem = static_cast<ProjectTreeItem*>(wrapInd.internalPointer());
        auto type = static_cast<TreeItemType>(pItem->getTypeId());

        if(isValidInputItemType(type) == false)
            return;

        // Check if image is with TIME dimension (image sequence)
        auto pDataset = CProjectUtils::getDataset<CMat>(wrapInd);
        bool bTimeImageSequence = pDataset != nullptr && pDataset->hasDimension(DataDimension::TIME) && type == TreeItemType::IMAGE;

        // Check if video or stream or image sequence
        if(type == TreeItemType::LIVE_STREAM || type == TreeItemType::VIDEO || bTimeImageSequence)
        {
            // Store current item index
            m_inputs[m_currentVideoInputIndex].setType(type);
            m_inputs[m_currentVideoInputIndex].setModelIndex(itemIndex, 0);

            if(bNewSequence == false)
                onWorkflowLive(m_currentVideoInputIndex, false);
            else
            {
                clearAllTasks();
                setWorkflowInput(m_currentVideoInputIndex, bNewSequence);
            }
        }
        else
        {
            // Store current item index
            m_inputs[inputIndex].setType(type);
            m_inputs[inputIndex].setModelIndex(itemIndex, 0);

            // We don't have VIDEO so we stop wait thread if running
            m_runMgr.stopWaitThread();

            // Maybe should ask if user wants to effectively stop
            if(m_runMgr.isRunning())
                m_pWorkflow->stop();

            if(bNewSequence == true)
                clearAllTasks();

            setWorkflowInput(inputIndex, bNewSequence);
        }

        //Reload source data if necessary
        if(bNewSequence == true && m_inputViewMode == WorkflowInputViewMode::CURRENT)
            m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getRootId()));
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::onLoadWorkflow(const QModelIndex &itemIndex)
{
    assert(m_pProcessMgr);
    assert(m_pGraphicsMgr);

    if(!itemIndex.isValid())
    {
        qCCritical(logWorkflow).noquote() << tr("Load workflow error: invalid item index");
        return;
    }

    QString workflowlName = itemIndex.data(Qt::DisplayRole).toString();
    auto it = m_mapWorkflowNameToId.find(workflowlName);

    if(it != m_mapWorkflowNameToId.end())
    {
        try
        {
            if(m_pWorkflow != nullptr)
                emit doCloseWorkflow();

            m_pWorkflow = m_dbMgr.load(it->second, m_pProcessMgr, m_pGraphicsMgr->getContext());
            if(m_pWorkflow)
                initWorkflow();
        }
        catch(std::exception& e)
        {
            qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CWorkflowManager::onDeleteWorkflow(const QModelIndex &itemIndex)
{
    if(!itemIndex.isValid())
    {
        qCCritical(logWorkflow).noquote() << tr("Delete workflow error: invalid item index");
        return;
    }

    QString protocolName = itemIndex.data(Qt::DisplayRole).toString();
    auto it = m_mapWorkflowNameToId.find(protocolName);

    if(it != m_mapWorkflowNameToId.end())
    {
        if(m_pWorkflow && m_pWorkflow->getName() == protocolName.toStdString())
        {
            emit doCloseWorkflow();                
            m_pWorkflow = nullptr;
        }
        m_dbMgr.remove(it->second);
        m_pModel->removeRow(itemIndex.row(), itemIndex.parent());

        auto itTmp = m_mapWorkflowIdToName.find(it->second);
        m_mapWorkflowIdToName.erase(itTmp);
        m_mapWorkflowNameToId.erase(it);
    }
}

void CWorkflowManager::onDeleteInput(int index)
{
    assert(m_pWorkflow);

    if(index < (int)m_inputs.size())
        m_inputs.erase(m_inputs.begin() + index);

    auto rootId = m_pWorkflow->getRootId();
    m_pWorkflow->deleteOutEdges(rootId, index);
    m_pWorkflow->removeInput(index);

    //Update inputs vizualisation if view mode is ORIGIN or active task is connected to root
    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
        m_inputViewMgr.manageInputs(m_pWorkflow->getTask(rootId));
    else if(m_pWorkflow->isConnectedToRoot(m_pWorkflow->getActiveTaskId()))
        m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId()));

    //Notify CWorkflowScene
    emit doInputRemoved(index);
}

void CWorkflowManager::onSetTaskActionFlag(const WorkflowVertex &id, CWorkflowTask::ActionFlag action, bool bEnable)
{
    assert(m_pWorkflow);
    m_pWorkflow->setTaskActionFlag(id, action, bEnable);

    if(action == CWorkflowTask::ActionFlag::OUTPUT_AUTO_EXPORT)
        emit doUpdateTaskIOInfo(m_pWorkflow->getTask(id));
}

void CWorkflowManager::onSetGraphicsLayerInput(const WorkflowVertex &id, int portIndex, const QModelIndex &layerIndex)
{
    assert(m_pGraphicsMgr);
    assert(m_pWorkflow);

    auto pTask = m_pWorkflow->getTask(id);
    if(pTask == nullptr)
    {
        qCCritical(logWorkflow).noquote() << tr("Invalid task: graphics layer input can't be set.");
        return;
    }

    if(layerIndex.isValid() == false)
    {
        pTask->clearInputData(portIndex);
        emit doInputAssigned(id, portIndex, false);
    }
    else
    {
        auto pLayer = m_pGraphicsMgr->getLayer(layerIndex);
        if(pLayer == nullptr)
        {
            qCCritical(logWorkflow).noquote() << tr("Invalid graphics layer: input can't be set.");
            return;
        }

        try
        {
            m_pGraphicsMgr->setCurrentLayer(layerIndex, true);
            pTask->setInput(std::make_shared<CGraphicsInput>(pLayer), portIndex);
            emit doInputAssigned(id, portIndex, true);
        }
        catch(std::exception& e)
        {
            qCritical(logWorkflow).noquote() << e.what();
        }
    }
}

void CWorkflowManager::onSetFolderPathInput(const WorkflowVertex &id, int index, const QString &path)
{
    assert(m_pWorkflow);

    auto taskPtr = m_pWorkflow->getTask(id);
    if(taskPtr == nullptr)
    {
        qCCritical(logWorkflow).noquote() << tr("Invalid task: folder path can't be set.");
        return;
    }

    try
    {
        taskPtr->setInput(std::make_shared<CPathIO>(IODataType::FOLDER_PATH, path.toStdString()), index);
        emit doInputAssigned(id, index, true);
    }
    catch(std::exception& e)
    {
        qCritical(logWorkflow).noquote() << e.what();
    }
}

void CWorkflowManager::onSetInteractionGraphicsLayer(CGraphicsLayer *pLayer)
{
    assert(m_pGraphicsMgr);
    size_t refInputIndex = 0;
    if (refInputIndex >= m_inputs.size())
        return;

    DisplayType refImgType = Utils::Data::treeItemTypeToDisplayType(m_inputs[refInputIndex].getType());
    m_interactionLayerInfo = CGraphicsLayerInfo(pLayer, refInputIndex, refImgType, CGraphicsLayerInfo::SOURCE, true);
    m_pGraphicsMgr->addTemporaryLayer(m_interactionLayerInfo);
    m_pGraphicsMgr->setCurrentLayer(pLayer);
}

void CWorkflowManager::onSetGraphicsTool(GraphicsShape tool)
{
    assert(m_pGraphicsMgr);
    m_pGraphicsMgr->setCurrentTool(tool);
}

void CWorkflowManager::onSetGraphicsCategory(const QString &category)
{
    assert(m_pGraphicsMgr);
    m_pGraphicsMgr->setCurrentCategory(category);
}

void CWorkflowManager::onSetInput(int inputIndex, const CWorkflowInput &input)
{
    if(inputIndex >= (int)m_inputs.size())
        return;

    m_inputs[inputIndex] = input;

    try
    {        
        setWorkflowInput(inputIndex, true);

        //Update origin inputs vizualisation
        if(m_inputViewMode == WorkflowInputViewMode::ORIGIN || m_pWorkflow->getActiveTaskId() == m_pWorkflow->getRootId())
            m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getRootId()));
    }
    catch(std::exception& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::onIODisplaysSelected(DisplayCategory category, int index)
{
    if(isWorkflowExists())
    {
        if(category ==  DisplayCategory::OUTPUT)
            m_pResultsMgr->notifyDisplaySelected(index);

        updateDataInfo();
    }
}

void CWorkflowManager::onGetWorkflowInfo(const QModelIndex &index)
{
    if(!index.isValid())
    {
        qCDebug(logWorkflow) << tr("Retrieve workflow information failed: invalid item index");
        return;
    }

    QString name = index.data(Qt::DisplayRole).toString();
    auto it = m_mapWorkflowNameToId.find(name);

    if(it != m_mapWorkflowNameToId.end())
    {
        try
        {
            QString description, keywords;
            m_dbMgr.getWorkflowInfo(it->second, description, keywords);
            emit doSetWorkflowInfo(description, keywords);
        }
        catch(std::exception& e)
        {
            qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CWorkflowManager::onClearInteractionGraphicsLayer(CGraphicsLayer *pLayer)
{
    assert(m_pGraphicsMgr);
    assert(m_interactionLayerInfo.m_pLayer == pLayer);
    m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
    m_interactionLayerInfo.m_pLayer = nullptr;
}

void CWorkflowManager::onAddProcess(const std::string& processName, const WorkflowTaskParamPtr& pParam)
{
    assert(m_pProcessMgr && m_pProjectMgr);

    try
    {
        if(m_pWorkflow == nullptr)
            createWorkflow(tr("New workflow").toStdString());

        WorkflowVertex newTaskId = addProcess(processName, pParam);
        //Set the added task as active to ease next connection
        m_pWorkflow->setActiveTask(newTaskId);
        emit doSetActiveTask(newTaskId);
        //Update task info
        auto pTask = m_pWorkflow->getTask(newTaskId);
        updateExecTimeInfo(newTaskId);
        emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
        //Display the next possible task to add to the protocol (empty informative item)
        emit doUpdateCandidateTask();
    }
    catch(CException& e)
    {
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());

        //Check if connection failed so we have to delete task from protocol
        if(e.getCode() == CoreExCode::INVALID_CONNECTION)
            m_pWorkflow->deleteTask(m_pWorkflow->getLastTaskId());
    }
}

void CWorkflowManager::onAddTaskToWorkflowView(const WorkflowVertex &id)
{
    if(isRoot(id) == false)
    {
        //Get first parent
        auto inRangeIt = m_pWorkflow->getInEdges(id);
        auto parentId = m_pWorkflow->getEdgeSource(*inRangeIt.first);
        //Get task
        auto pProcess = m_pWorkflow->getTask(id);
        //Add task to view
        emit doAddTask(pProcess, id, parentId);
    }
}

void CWorkflowManager::onAddInput(const CWorkflowInput &input)
{
    assert(m_pWorkflow);
    m_inputs.push_back(input);

    try
    {
        setWorkflowInput((int)m_pWorkflow->getInputCount(), true);

        //Update origin inputs vizualisation if view mode is ORIGIN
        if(m_inputViewMode == WorkflowInputViewMode::ORIGIN || m_pWorkflow->getActiveTaskId() == m_pWorkflow->getRootId())
            m_inputViewMgr.manageInputs(m_pWorkflow->getTask(m_pWorkflow->getRootId()));
    }
    catch(std::exception& e)
    {
        m_inputs.pop_back();
        qCCritical(logWorkflow).noquote() << QString::fromStdString(e.what());
    }
}

void CWorkflowManager::onFinishTask(const WorkflowVertex& id, CWorkflowTask::State status, const QString& msg)
{
    if(m_pWorkflow == nullptr)
        return;

    // Update info for active task
    updateExecTimeInfo(m_pWorkflow->getActiveTaskId());

    if(m_pWorkflow->isValid(id))
        emit doSetTaskState(id, status, msg);
}

void CWorkflowManager::onQueryGraphicsProxyModel()
{
    assert(m_pProjectMgr);
    assert(m_inputs.size() > 0);

    // Case where there are no data loaded + a protocol has been loaded + someone clicked on graphics port
    auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentModelIndex.isValid() == false)
        return;

    std::vector<QModelIndex> modelIndices;
    for(size_t i=0; i<m_inputs.size(); ++i)
    {
        for(size_t j=0; j<m_inputs[i].getModelIndicesCount(); ++j)
            modelIndices.push_back(m_inputs[i].getModelIndex(j));
    }
    auto pGraphicsProxyModel = m_pProjectMgr->getGraphicsProxyModel(modelIndices);
    emit doSetGraphicsProxyModel(pGraphicsProxyModel);
}

void CWorkflowManager::onQueryProjectDataProxyModel(const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters)
{
    assert(m_pProjectMgr);

    if (m_inputs.size() == 0)
        return;

    // Case where there are no data loaded + a protocol has been loaded + someone clicked on graphics port
    auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentModelIndex.isValid() == false)
        return;

    auto pDataProxyModel = m_pProjectMgr->getDataProxyModel(currentModelIndex, dataTypes, filters);
    emit doSetProjectDataProxyModel(pDataProxyModel);
}

void CWorkflowManager::initGlobalConnections()
{
    //Run manager -> workflow manager
    connect(&m_runMgr, &CWorkflowRunManager::doSetElapsedTime, [&](double time){ emit doSetElapsedTime(time); });
    connect(&m_runMgr, &CWorkflowRunManager::doWorkflowLive, this, &CWorkflowManager::onWorkflowLive, Qt::BlockingQueuedConnection);

    //Scale manager -> workflow manager
    connect(&m_scaleMgr, &CWorkflowScaleManager::doSetProjects, [&](const QJsonArray& projects)
    {
        emit doSetScaleProjects(projects, m_currentUser);
    });
}

void CWorkflowManager::onQueryIOInfo(const WorkflowVertex &taskId, int index, bool bInput)
{
    if(m_pWorkflow)
    {
        auto info = m_pWorkflow->getIOInfo(taskId, index, bInput);
        emit doSetIOInfo(info, taskId, index, bInput);
    }
}

void CWorkflowManager::initConnections()
{
    //Workflow run manager -> workflow manager
    connect(&m_runMgr, &CWorkflowRunManager::doWorkflowFinished, this, &CWorkflowManager::onRunWorkflowFinished);
    connect(&m_runMgr, &CWorkflowRunManager::doWorkflowFailed, this, &CWorkflowManager::onRunWorkflowFailed);

    //Workflow -> Workflow manager
    auto pHandler = static_cast<CWorkflowSignalHandler*>(m_pWorkflow->getSignalRawPtr());
    connect(pHandler, &CWorkflowSignalHandler::doFinishTask, this, &CWorkflowManager::onFinishTask);
    connect(pHandler, &CWorkflowSignalHandler::doAddGraphicsLayer, this, &CWorkflowManager::onSetInteractionGraphicsLayer);
    connect(pHandler, &CWorkflowSignalHandler::doRemoveGraphicsLayer, this, &CWorkflowManager::onClearInteractionGraphicsLayer);
    connect(pHandler, &CWorkflowSignalHandler::doOutputChanged, this, &CWorkflowManager::onOutputChanged);
    connect(pHandler, &CWorkflowSignalHandler::doUpdateTaskGraphicsInput, this, &CWorkflowManager::onUpdateTaskGraphicsInput);

    connect(pHandler, &CWorkflowSignalHandler::doUpdateTaskItemView, [&](const WorkflowTaskPtr& taskPtr, const WorkflowVertex& id)
    {
        emit doUpdateTaskItemView(taskPtr, id);
    });
    connect(pHandler, &CWorkflowSignalHandler::doSetTaskState, [&](const WorkflowVertex& id, CWorkflowTask::State state, const QString& msg)
    {
        emit doSetTaskState(id, state, msg);
    });
    connect(pHandler, &CWorkflowSignalHandler::doDeleteConnection, [&](const WorkflowEdge& id)
    {
        emit doDeleteConnection(id);
    });
    connect(pHandler, &CWorkflowSignalHandler::doGraphicsContextChanged, [&]
    {
        assert(m_pGraphicsMgr);
        emit m_pGraphicsMgr->doGraphicsContextChanged();
    });
    connect(pHandler, &CWorkflowSignalHandler::doLog, [&](const QString& msg)
    {
        qCInfo(logWorkflow).noquote() << msg;
    });

    //Workflow -> ProgressBar manager
    connect(m_pWorkflow->getSignalRawPtr(), &CSignalHandler::doSetMessage, m_pProgressMgr, &CProgressBarManager::onSetMessage);
}

void CWorkflowManager::initWorkflow()
{
    assert(m_pWorkflow);

    m_runMgr.setWorkflow(m_pWorkflow);
    emit doWorkflowCreated();

    //Set inputs
    initInputs();

    for(size_t i=0; i<m_inputs.size(); ++i)
    {
        if (m_inputs[i].getType() != TreeItemType::NONE)
            setWorkflowInput(i, true);
    }

    //Create signals/slots connections
    initConnections();

    //Create protocol view
    buildWorkflowView();
    m_pWorkflow->updateHash();
    emit doNewWorkflowNotification(QString("Workflow %1 has been created.").arg(QString::fromStdString(m_pWorkflow->getName())), Notification::INFO);
}

void CWorkflowManager::initInputs()
{
    assert(m_pProjectMgr);
    m_inputs.clear();

    QModelIndex currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentModelIndex.isValid())
    {
        TreeItemType type = m_pProjectMgr->getItemType(currentModelIndex);
        CWorkflowInput input(WorkflowInputMode::CURRENT_DATA, type);
        input.appendModelIndex(currentModelIndex);
        m_inputs.push_back(input);
    }

    if(m_pWorkflow)
    {        
        // Get other possible inputs different from current data (ie index = 0)
        std::set<size_t> connectedIndices;
        WorkflowVertex root = m_pWorkflow->getRootId();
        auto outEdges = m_pWorkflow->getOutEdges(root);

        for(auto it=outEdges.first; it!=outEdges.second; ++it)
        {
            WorkflowEdgePtr edge = m_pWorkflow->getEdge(*it);
            size_t index = edge->getSourceIndex();

            if(index > 0)
                connectedIndices.insert(edge->getSourceIndex());
        }

        for(auto it=connectedIndices.begin(); it!=connectedIndices.end(); ++it)
            m_inputs.push_back(CWorkflowInput());
    }
}

void CWorkflowManager::loadWorkflows()
{
    QStringList protocolNames;

    if(m_pModel == nullptr)
        m_pModel = new QStringListModel(this);

    m_mapWorkflowNameToId = m_dbMgr.getWorkflows();
    for(auto it=m_mapWorkflowNameToId.begin(); it!=m_mapWorkflowNameToId.end(); ++it)
    {
        m_mapWorkflowIdToName.insert(std::make_pair(it->second, it->first));
        protocolNames.push_back(it->first);
    }
    m_pModel->setStringList(protocolNames);
    emit doSetNamesModel(m_pModel);
}

void CWorkflowManager::buildWorkflowView()
{
    assert(m_pWorkflow);

    //Depth first search to build the protocol view in the right order
    auto pHandler = static_cast<CWorkflowSignalHandler*>(m_pWorkflow->getSignalRawPtr());
    connect(pHandler, &CWorkflowSignalHandler::doDfsTaskVisited, this, &CWorkflowManager::onAddTaskToWorkflowView);
    m_pWorkflow->startDepthFirstSearch();
    disconnect(pHandler, &CWorkflowSignalHandler::doDfsTaskVisited, this, &CWorkflowManager::onAddTaskToWorkflowView);

    //Add connections
    auto edgeRangeIt = m_pWorkflow->getEdges();
    for(auto it=edgeRangeIt.first; it!=edgeRangeIt.second; ++it)
    {
        auto srcId = m_pWorkflow->getEdgeSource(*it);
        auto targetId = m_pWorkflow->getEdgeTarget(*it);
        auto pEdge = m_pWorkflow->getEdge(*it);
        emit doAddConnection(*it, srcId, pEdge->getSourceIndex(), targetId, pEdge->getTargetIndex());
    }

    //Select last added task
    m_pWorkflow->setActiveTask(m_pWorkflow->getLastTaskId());
    emit doSetActiveTask(m_pWorkflow->getLastTaskId());
    //Update task info
    auto lastId = m_pWorkflow->getLastTaskId();
    auto pLastTask = m_pWorkflow->getTask(lastId);
    updateExecTimeInfo(lastId);
    emit doUpdateTaskInfo(pLastTask, m_pProcessMgr->getProcessInfo(pLastTask->getName()));
    //Update next candidate task position
    emit doUpdateCandidateTask();
}

void CWorkflowManager::checkInput(size_t index) const
{
    if(index >= m_inputs.size())
        throw CException(CoreExCode::INDEX_OVERFLOW, tr("Input index overflow.").toStdString(), __func__, __FILE__, __LINE__);

    auto type = m_inputs[index].getType();
    if(!isValidInputItemType(type))
        throw CException(CoreExCode::INVALID_USAGE, tr("Invalid input type.").toStdString(), __func__, __FILE__, __LINE__);
}

bool CWorkflowManager::isValidInputItemType(TreeItemType type) const
{
    return type == TreeItemType::FOLDER || type == TreeItemType::DATASET ||
            type == TreeItemType::IMAGE || type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM;
}

void CWorkflowManager::setWorkflowInput(size_t inputIndex, bool bNewSequence)
{
    assert(m_pWorkflow);

    if(bNewSequence)
    {
        // May throw
        checkInput(inputIndex);
        // Set batch state
        m_pWorkflow->setInputBatchState(inputIndex, isBatchInput(inputIndex));
    }
    //Set first workflow input data
    auto taskIOPtr = m_runMgr.createTaskIO(inputIndex, 0, bNewSequence);
    if(!taskIOPtr)
    {
        qCritical(logWorkflow()).noquote() << tr("Workflow input cannot be set: invalid item.");
        return;
    }
    m_pWorkflow->setInput(taskIOPtr, inputIndex, bNewSequence);
    // Notify protocol view
    emit doInputChanged(inputIndex);
}

WorkflowVertex CWorkflowManager::addProcess(const std::string &name, const WorkflowTaskParamPtr &pParam)
{
    //Get process object
    auto taskPtr = m_pProcessMgr->createObject(name, pParam);
    if(taskPtr == nullptr)
    {
        std::string errorMsg = "Instanciation failed for object of type: " + name;
        throw CException(CoreExCode::CREATE_FAILED, errorMsg, __func__, __FILE__, __LINE__);
    }

    //Add process to protocol
    auto parentTaskId = m_pWorkflow->getActiveTaskId();
    auto newTaskId = m_pWorkflow->addTask(taskPtr);

    //Connection
    std::vector<WorkflowEdge> newEdgeIds;
    if(taskPtr->getInputCount() > 0)
    {
        try
        {
            newEdgeIds = m_pWorkflow->connect(parentTaskId, newTaskId);
        }
        catch(std::exception& e)
        {
            qCritical(logWorkflow()).noquote() << QString::fromStdString(e.what());
        }
    }

    //Notify view
    emit doAddCandidateTask(taskPtr, newTaskId);
    emit doNewWorkflowNotification(QString("A new process (%1) has been added to the workflow.").arg(QString::fromStdString(name)), Notification::INFO);

    for(size_t i=0; i<newEdgeIds.size(); ++i)
    {
        auto edgePtr = m_pWorkflow->getEdge(newEdgeIds[i]);
        emit doAddConnection(newEdgeIds[i], parentTaskId, edgePtr->getSourceIndex(), newTaskId, edgePtr->getTargetIndex());
    }

    if(parentTaskId == m_pWorkflow->getRootId() && newEdgeIds.size() > 0)
        rootConnectionChanged();

    return newTaskId;
}

void CWorkflowManager::updateProcessingFPS()
{
    auto time = m_runMgr.getTotalElapsedTime();
    if(time != 0)
        m_currentFPS = std::min(m_maxFps, (int)(1000.0/time));
    else
        m_currentFPS = 25;
}

void CWorkflowManager::updateExecTimeInfo(const WorkflowVertex& taskId)
{
    m_execTimeInfo.clear();

    auto pTask = m_pWorkflow->getTask(taskId);

    // Add here all values that you want to display in protocol module

    // Process elapsed time
    double elapsedTime = pTask->getElapsedTime();
    QString timeStr = timeToString(elapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Process elapsed time", timeStr.toStdString()));

    // Elapsed time to current task
    double currentElapsedTime = m_pWorkflow->getElapsedTimeTo(taskId);
    QString currentTimeStr = timeToString(currentElapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Intermediate elapsed time", currentTimeStr.toStdString()));

    // Workflow elapsed time
    double totalElapsedTime = m_pWorkflow->getTotalElapsedTime();
    QString totalTimeStr = timeToString(totalElapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Total elapsed time", totalTimeStr.toStdString()));

    emit doUpdateTaskStateInfo(m_execTimeInfo, pTask->getCustomInfo());
}

void CWorkflowManager::updateVideoInputIndex(size_t index)
{
    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
    {
        m_currentVideoInputIndex = index;
        m_inputViewMgr.setLiveInputIndex(index);
    }
    else
    {
        // In this case, active task can have multiple input so that index is
        // greater than origin inputs count. Since we can only connect
        // one video or stream as source, we have to find its index.
        for(size_t i=0; i<m_inputs.size(); ++i)
        {
            if(m_pProjectMgr->isTimeDataItem(m_inputs[i].getModelIndex(0)) && m_pWorkflow->isInputConnected(i))
            {
                m_currentVideoInputIndex = i;
                m_inputViewMgr.setLiveInputIndex(i);
                break;
            }
        }
    }
}

QString CWorkflowManager::timeToString(double time)
{
    int t = int(time);
    auto ms = int(t%1000);
    auto sec = int((t/1000)%60);
    auto min = int((t/60000)%60);

    QString timeStr = QString(tr("%1 min %2 sec %3 ms")).arg(min).arg(sec).arg(ms);
    return timeStr;
}

void CWorkflowManager::clearAllTasks()
{
    if(m_pWorkflow == nullptr)
        return;

    //Notify results manager that the protocol will be cleared
    m_pResultsMgr->notifyBeforeWorkflowCleared();

    auto vertexList = m_pWorkflow->getAllChilds(m_pWorkflow->getRootId());
    for(const auto& it : vertexList)
        clearTask(it);
}

void CWorkflowManager::clearFrom(const WorkflowVertex& taskId)
{
    if(m_pWorkflow == nullptr)
        return;

    //Notify results manager that the task will be cleared
    m_pResultsMgr->notifyBeforeWorkflowCleared();

    auto vertexList = m_pWorkflow->getAllChilds(taskId);
    vertexList.insert(vertexList.begin(), taskId);

    for(const auto& it : vertexList)
        clearTask(it);
}

void CWorkflowManager::clearTo(const WorkflowVertex &taskId)
{
    if(m_pWorkflow == nullptr)
        return;

    //Notify results manager that the task will be cleared
    m_pResultsMgr->notifyBeforeWorkflowCleared();

    std::vector<WorkflowVertex> vertexList;
    m_pWorkflow->getAllParents(taskId, vertexList);
    vertexList.push_back(taskId);

    for(const auto& it : vertexList)
        clearTask(it);
}

void CWorkflowManager::clearTask(const WorkflowVertex& taskId)
{
    assert(m_pWorkflow);

    // Check if root and if so, avoid clearing data
    if(m_pWorkflow->isRoot(taskId))
        return;

    auto pTask = m_pWorkflow->getTask(taskId);
    // Clear all output data
    pTask->clearOutputData();
    // Reset status
    emit doSetTaskState(taskId, CWorkflowTask::State::UNDONE);
}

void CWorkflowManager::rootConnectionChanged()
{
    for(size_t i=0; i<m_inputs.size(); ++i)
        setWorkflowInput(i, true);
}

void CWorkflowManager::updateDataInfo()
{
    assert(m_pDataMgr);
    WorkflowTaskPtr taskPtr;
    WorkflowTaskIOPtr ioPtr;
    IODataType dataType;

    if(m_pDataMgr->getSelectedDisplayCategory() == DisplayCategory::INPUT)
    {
        if(m_inputViewMode == WorkflowInputViewMode::CURRENT)
            taskPtr = m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId());
        else
            taskPtr = m_pWorkflow->getTask(m_pWorkflow->getRootId());

        if(taskPtr == nullptr)
            return;

        ioPtr = taskPtr->getInput(m_pDataMgr->getSelectedDisplayIndex());
        if(ioPtr == nullptr)
            return;

        dataType = ioPtr->getDataType();
    }
    else
    {
        taskPtr = m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId());
        if(taskPtr == nullptr)
            return;

        ioPtr = taskPtr->getOutput(m_pDataMgr->getSelectedDisplayIndex());
        if(ioPtr == nullptr)
            return;

        dataType = ioPtr->getDataType();
    }

    if( dataType == IODataType::IMAGE || dataType == IODataType::IMAGE_LABEL || dataType == IODataType::IMAGE_BINARY ||
        dataType == IODataType::VOLUME || dataType == IODataType::VOLUME_LABEL || dataType == IODataType::VOLUME_BINARY)
    {
        auto infoPtr = ioPtr->getDataInfo();
        if(infoPtr)
            emit doUpdateImageInfo(infoPtr->getStringList());
    }
    else if(dataType == IODataType::VIDEO || dataType == IODataType::VIDEO_LABEL || dataType == IODataType::VIDEO_BINARY)
    {
        //Update current selected image -> updating info is done when displaying each frame
        auto image = std::static_pointer_cast<CVideoIO>(ioPtr)->getImage();
        m_pDataMgr->getVideoMgr()->setSelectedImageFromWorkflow(image);
    }
}

void CWorkflowManager::reloadCurrentPlugins()
{
    assert(m_pProcessMgr);

    CPyEnsureGIL gil;
    if (m_pWorkflow == nullptr)
        return;

    try
    {
        auto rangeIt = m_pWorkflow->getVertices();
        for (auto it=rangeIt.first; it!=rangeIt.second; ++it)
        {
            WorkflowVertex taskId = *it;
            WorkflowTaskPtr taskPtr = m_pWorkflow->getTask(taskId);
            auto processInfo = m_pProcessMgr->getProcessInfo(taskPtr->getName());

            if (processInfo.isInternal() == false)
            {
                // Save parameters
                UMapString paramValues = taskPtr->getParamValues();
                // Reload plugin
                m_pProcessMgr->onReloadPlugin(QString::fromStdString(taskPtr->getName()), processInfo.getLanguage());
                // Get fresh task instance
                taskPtr = m_pWorkflow->getTask(taskId);
                if (paramValues != taskPtr->getParamValues())
                {
                    // Reset saved parameters
                    taskPtr->setParamValues(paramValues);
                    if (m_pWorkflow->getActiveTaskId() == taskId)
                        emit doUpdateTaskInfo(taskPtr, m_pProcessMgr->getProcessInfo(taskPtr->getName()));
                }
            }
        }
    }
    catch (std::exception& e)
    {
        Utils::print(e.what(), QtMsgType::QtCriticalMsg);
    }
}

void CWorkflowManager::saveCurrentInputImage(size_t index)
{
    WorkflowTaskPtr taskPtr;
    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
        taskPtr = m_pWorkflow->getTask(m_pWorkflow->getRootId());
    else
        taskPtr = m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId());

    auto inputPtr = std::dynamic_pointer_cast<CImageIO>(taskPtr->getInput(index));
    if(!inputPtr)
    {
        qCCritical(logWorkflow).noquote() << tr("No valid image data for input #%1 of task %2.").arg(index+1).arg(QString::fromStdString(taskPtr->getName()));
        return;
    }
    std::string path = taskPtr->getOutputFolder() + taskPtr->getName() + Utils::Data::getFileFormatExtension(DataFileFormat::PNG);
    m_pDataMgr->getImgMgr()->exportImage(inputPtr->getImage(), std::vector<ProxyGraphicsItemPtr>(), QString::fromStdString(path));
}

void CWorkflowManager::exportCurrentInputImage(size_t index, const QString &path, bool bWithGraphics)
{
    WorkflowTaskPtr taskPtr;
    if(m_inputViewMode == WorkflowInputViewMode::ORIGIN)
        taskPtr = m_pWorkflow->getTask(m_pWorkflow->getRootId());
    else
        taskPtr = m_pWorkflow->getTask(m_pWorkflow->getActiveTaskId());

    auto inputPtr = std::dynamic_pointer_cast<CImageIO>(taskPtr->getInput(index));
    if(!inputPtr)
    {
        qCCritical(logWorkflow).noquote() << tr("No valid image data for input #%1 of task %2.").arg(index+1).arg(QString::fromStdString(taskPtr->getName()));
        return;
    }

    std::vector<ProxyGraphicsItemPtr> graphicsItems;
    if(bWithGraphics)
    {
        auto graphicsInputs = taskPtr->getInputs( {IODataType::INPUT_GRAPHICS} );
        for(size_t i=0; i<graphicsInputs.size(); ++i)
        {
            auto graphicsInput = std::dynamic_pointer_cast<CGraphicsInput>(graphicsInputs[i]);
            if(graphicsInput)
            {
                auto items = graphicsInput->getItems();
                graphicsItems.insert(graphicsItems.end(), items.begin(), items.end());
            }
        }
    }
    m_pDataMgr->getImgMgr()->exportImage(inputPtr->getImage(), graphicsItems, path);
}

void CWorkflowManager::playVideoInput(size_t index)
{
    // If protocol is running, don't do anything
    if(isWorkflowRunning())
        return;

    updateVideoInputIndex(index);
    m_pDataMgr->getVideoMgr()->play(m_inputs[m_currentVideoInputIndex].getModelIndex(0), index);
}

void CWorkflowManager::exposeTaskParameters(const WorkflowVertex &taskId, const CWorkflow::ExposedParams &params)
{
    assert(m_pWorkflow);

    // Remove previous exposed parameters for this task
    CWorkflow::ExposedParams exposedParams = m_pWorkflow->getExposedParameters();
    for (auto it=exposedParams.begin(); it!=exposedParams.end(); ++it)
    {
        auto id = reinterpret_cast<WorkflowVertex>(it->second.getTaskId());
        if (id == taskId)
            m_pWorkflow->removeExposedParameter(it->first);
    }

    // Add exposed parameters for the given task
    for (auto it=params.begin(); it!=params.end(); ++it)
        m_pWorkflow->addExposedParameter(it->second.getName(), it->second.getDescription(), taskId, it->second.getTaskParamName());
}

void CWorkflowManager::exposeCurrentTaskOutput(int outputIndex)
{
    auto taskId = getActiveTaskId();
    m_pWorkflow->addOutput("", taskId, outputIndex);
}

void CWorkflowManager::removeCurrentTaskExposedOutput(int outputIndex)
{
    auto taskId = getActiveTaskId();
    m_pWorkflow->removeOutput(taskId, outputIndex);
}

#include "moc_CWorkflowManager.cpp"
