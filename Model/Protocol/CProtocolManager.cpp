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

 #include "CProtocolManager.h"
#include <QtConcurrent/QtConcurrent>
#include "Main/AppTools.hpp"
#include "Model/Process/CProcessManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Graphics/CGraphicsManager.h"
#include "Model/Results/CResultManager.h"
#include "Model/Data/CMainDataManager.h"
#include "Model/Settings/CSettingsManager.h"
#include "IO/CGraphicsProcessInput.h"
#include "IO/CImageProcessIO.h"
#include "IO/CVideoProcessIO.h"

#include "IO/CPathIO.h"

CProtocolManager::CProtocolManager() :
    m_runMgr(&m_inputs),
    m_inputViewMgr(&m_inputs, ProtocolInputViewMode::ORIGIN)
{
    initGlobalConnections();
}

CProtocolManager::~CProtocolManager()
{
}

std::string CProtocolManager::getProtocolName() const
{
    if(m_pProtocol)
        return m_pProtocol->getName();
    else
        return "";
}

ProtocolVertex CProtocolManager::getRootId() const
{
    if(m_pProtocol)
        return m_pProtocol->getRootId();
    else
        return boost::graph_traits<ProtocolGraph>::null_vertex();
}

ProtocolTaskIOPtr CProtocolManager::getInput(size_t index) const
{
    if(m_pProtocol)
        return m_pProtocol->getInput(index);
    else
        return nullptr;
}

IODataType CProtocolManager::getInputDataType(size_t index) const
{
    if(m_pProtocol)
        return m_pProtocol->getInputDataType(index);
    else
        return IODataType::NONE;
}

ProtocolInputViewMode CProtocolManager::getInputViewMode() const
{
    return m_inputViewMode;
}

void CProtocolManager::setManagers(CProcessManager *pProcessMgr, CProjectManager *pProjectMgr,
                                   CGraphicsManager *pGraphicsMgr, CResultManager *pResultsMgr,
                                   CMainDataManager  *pDataMgr, CProgressBarManager *pProgressMgr,
                                   CSettingsManager *pSettingsMgr)
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

    if(m_pProgressMgr)
    {
        connect(this, &CProtocolManager::doSetElapsedTime, m_pProgressMgr, &CProgressBarManager::onSetElapsedTime);
        connect(this, &CProtocolManager::doSetMessage, m_pProgressMgr, &CProgressBarManager::onSetMessage);
    }
}

void CProtocolManager::setProtocolName(const std::string &name)
{
    if(m_pProtocol)
        m_pProtocol->setName(name);
}

void CProtocolManager::setProtocolKeywords(const std::string &keywords)
{
    if(m_pProtocol)
        m_pProtocol->setKeywords(keywords);
}

void CProtocolManager::setProtocolDescription(const std::string &description)
{
    if(m_pProtocol)
        m_pProtocol->setDescription(description);
}

void CProtocolManager::setInputViewMode(ProtocolInputViewMode mode)
{
    if(m_pProtocol == nullptr)
        return;

    m_inputViewMode = mode;
    m_inputViewMgr.setViewMode(mode);

    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
        m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getRootId()));
    else
        m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getActiveTaskId()));
}

void CProtocolManager::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

void CProtocolManager::setCurrentTaskAutoSave(size_t outputIndex, bool bAutoSave)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    auto outputPtr = taskPtr->getOutput(outputIndex);
    if(outputPtr)
    {
        outputPtr->setAutoSave(bAutoSave);
        if(m_pProtocol->isBatchMode())
        {
            taskPtr->setActionFlag(CProtocolTask::ActionFlag::OUTPUT_AUTO_EXPORT, taskPtr->isAutoSave());
            doUpdateTaskItemView(taskPtr, m_pProtocol->getActiveTaskId());
        }
    }
}

void CProtocolManager::setCurrentTaskSaveFolder(const std::string &path)
{
    auto taskPtr = getActiveTask();
    if(!taskPtr)
        return;

    taskPtr->setOutputFolder(path);
}

void CProtocolManager::setCurrentTaskSaveFormat(size_t outputIndex, size_t formatIndex)
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

int CProtocolManager::getCurrentFPS() const
{
    return m_currentFPS;
}

QModelIndex CProtocolManager::getCurrentVideoInputModelIndex() const
{
    if(m_currentVideoInputIndex < m_inputs.size())
        return m_inputs[m_currentVideoInputIndex].getModelIndex(0);
    else
        return QModelIndex();
}

std::vector<int> CProtocolManager::getDisplayedInputIndices(const ProtocolTaskPtr &taskPtr, const std::set<IODataType>& types) const
{
    std::vector<int> inputIndices;

    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
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

void CProtocolManager::notifyViewShow()
{
    loadProtocols();
    emit doSetManager(this);
}

void CProtocolManager::notifyGraphicsChanged()
{
    m_runMgr.notifyGraphicsChanged();
}

void CProtocolManager::forceBatchMode(bool bEnable)
{
    if(m_pProtocol)
        m_pProtocol->forceBatchMode(bEnable);
}

void CProtocolManager::enableAutoLoadBatchResults(bool bEnable)
{
    m_bAutoLoadBatchResult = bEnable;
}

void CProtocolManager::enableSaveWithGraphics(bool bEnable)
{
    if(m_pProtocol)
        m_pProtocol->setCfgEntry("GraphicsEmbedded", std::to_string(bEnable));
}

void CProtocolManager::beforeProjectClose(bool bWithCurrentImage)
{
    if(bWithCurrentImage == true)
    {
        if(m_interactionLayerInfo.m_pLayer != nullptr)
        {
            assert(m_pGraphicsMgr);
            m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
            m_interactionLayerInfo.m_pLayer = nullptr;
        }

        if(m_pProtocol)
        {
            for(size_t i=0; i<m_inputs.size(); ++i)
                m_inputs[i].clearModelIndex();

            clearAllTasks();
            m_pProtocol->clearInputs();
            emit doInputsCleared();
        }
        else
            initInputs();
    }
}

void CProtocolManager::beforeDataDeleted(const QModelIndex &index)
{
    for(size_t i=0; i<m_inputs.size(); ++i)
    {
        if(m_inputs[i].contains(index))
        {
            if(m_inputs[i].getMode() == ProtocolInputMode::CURRENT_DATA)
            {
                assert(m_pGraphicsMgr);
                m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
                m_interactionLayerInfo.m_pLayer = nullptr;
            }

            m_inputs[i].clearModelIndex();
            if(m_pProtocol)
            {
                m_pProtocol->clearInputData(i);
                emit doInputChanged(i);
            }
        }
    }
}

void CProtocolManager::beforeDataDeleted(const std::vector<QModelIndex> &indexes)
{
    for(size_t i=0; i<indexes.size(); ++i)
        beforeDataDeleted(indexes[i]);
}

bool CProtocolManager::isRoot(const ProtocolVertex &id) const
{
    if(m_pProtocol)
        return m_pProtocol->isRoot(id);
    else
        return false;
}

bool CProtocolManager::isProtocolExists() const
{
    return m_pProtocol != nullptr;
}

bool CProtocolManager::isProtocolModified() const
{
    if(m_pProtocol)
    {
        try
        {
            return m_pProtocol->isModified();
        }
        catch(std::exception& e)
        {
            qCWarning(logProtocol).noquote() << QString::fromStdString(e.what());
            return false;
        }
    }
    else
        return false;
}

bool CProtocolManager::isProtocolRunning() const
{
    return m_runMgr.isRunning();
}

bool CProtocolManager::isBatchInput(size_t index) const
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

void CProtocolManager::createProtocol(const std::string &name, const std::string &keywords, const std::string &description)
{
    assert(m_pProcessMgr);
    try
    {
        m_pProtocol = std::make_unique<CProtocol>(name);
        m_pProtocol->setKeywords(keywords);
        m_pProtocol->setDescription(description);
        m_pProtocol->setOutputFolder(m_pSettingsMgr->getProtocolSaveFolder() + name + "/");
        initProtocol();
    }
    catch(std::exception& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

ProtocolTaskWidgetPtr CProtocolManager::createTaskWidget(const ProtocolTaskPtr& pTask)
{
    assert(pTask);
    return m_pProcessMgr->createWidget(pTask->getName(), pTask->getParam());
}

void CProtocolManager::runProtocol()
{
    if(m_pProtocol == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
        clearAllTasks();

    m_protocolTimer.start();
    m_runMgr.run();
}

void CProtocolManager::runProtocolFromActiveTask()
{
    if(m_pProtocol == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
    {
        clearFrom(m_pProtocol->getActiveTaskId());
        m_protocolTimer.start();
        m_runMgr.runFromActiveTask();
    }
}

void CProtocolManager::runProtocolToActiveTask()
{
    if(m_pProtocol == nullptr)
        return;

    if(m_runMgr.isRunning() == false)
    {
        clearTo(m_pProtocol->getActiveTaskId());
        m_protocolTimer.start();
        m_runMgr.runToActiveTask();
    }
}

void CProtocolManager::stopProtocol()
{
    m_runMgr.stop();
}

void CProtocolManager::beforeGraphicsLayerRemoved(CGraphicsLayer *pLayer)
{
    if(m_pProtocol && pLayer)
    {
        std::set<const CGraphicsLayer*> layersToBeRemoved;
        auto childLayers = pLayer->getChildLayers();

        layersToBeRemoved.insert(pLayer);
        for(int i=0; i<childLayers.size(); ++i)
            layersToBeRemoved.insert(childLayers[i]);

        auto vertexRangeIt = m_pProtocol->getVertices();
        for(auto it=vertexRangeIt.first; it!=vertexRangeIt.second; ++it)
        {
            auto pTask = m_pProtocol->getTask(*it);
            if(pTask)
            {
                auto inputs = pTask->getInputs();
                for(size_t i=0; i<inputs.size(); ++i)
                {
                    if(inputs[i]->getDataType() == IODataType::INPUT_GRAPHICS)
                    {
                        auto pGraphicsInput = std::static_pointer_cast<CGraphicsProcessInput>(inputs[i]);
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

void CProtocolManager::saveProtocol()
{
    if(m_pProtocol)
    {
        try
        {
            QString protocolName = QString::fromStdString(m_pProtocol->getName());
            auto it = m_mapProtocolNameToId.find(protocolName);
            if(it != m_mapProtocolNameToId.end())
            {
                //Overwrite existing protocol
                m_dbMgr.remove(it->second);
                m_dbMgr.save(m_pProtocol);
            }
            else
            {
                //New internal protocol
                int protocolDbId = m_dbMgr.save(m_pProtocol);
                if(m_pModel->insertRow(m_pModel->rowCount()))
                {
                    QModelIndex index = m_pModel->index(m_pModel->rowCount() - 1, 0);
                    m_pModel->setData(index, protocolName);
                    m_mapProtocolNameToId.insert(std::make_pair(protocolName, protocolDbId));
                    m_mapProtocolIdToName.insert(std::make_pair(protocolDbId, protocolName));
                }
            }
            m_pProtocol->updateHash();
            emit doNotifyProtocolSaved();
            emit doNewProtocolNotification(tr("Workflow has been saved."), Notification::INFO);
        }
        catch(std::exception& e)
        {
            qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CProtocolManager::saveProtocol(const QString &path)
{
    if(m_pProtocol)
    {
        try
        {
            m_dbMgr.save(m_pProtocol, path);
            emit doNewProtocolNotification(tr("Workflow has been exported."), Notification::INFO);
        }
        catch(std::exception& e)
        {
            qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CProtocolManager::loadProtocol(const QString &path)
{
    assert(m_pProcessMgr);
    assert(m_pGraphicsMgr);

    try
    {
        if(m_pProtocol)
            emit doCloseProtocol();

        m_pProtocol = m_dbMgr.load(path, m_pProcessMgr, m_pGraphicsMgr->getContext());
        if(m_pProtocol)
            initProtocol();
    }
    catch(std::exception& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

void CProtocolManager::loadImageProtocols(const QModelIndex &imageIndex)
{
    assert(m_pProjectMgr);

    if(imageIndex.isValid() == false)
        qCCritical(logProtocol).noquote() << tr("Invalid image index");

    auto imgItemPtr = CProjectUtils::getImageItem(m_pProjectMgr->wrapIndex(imageIndex));
    if(imgItemPtr == nullptr)
        qCCritical(logProtocol).noquote() << tr("Invalid image item");

    if(m_pImageModel == nullptr)
        m_pImageModel = new QStringListModel(this);

    QStringList protocolNames;
    auto protocolIds = imgItemPtr->getProtocolDbIds();

    for(size_t i=0; i<protocolIds.size(); ++i)
    {
        auto it = m_mapProtocolIdToName.find(protocolIds[i]);
        if(it != m_mapProtocolIdToName.end())
            protocolNames.push_back(it->second);
    }
    m_pImageModel->setStringList(protocolNames);
    emit doSetNamesFromImageModel(m_pImageModel);
}

void CProtocolManager::onProtocolClosed()
{
    if(m_pProtocol == nullptr)
        return;

    // VIDEO Stop wait thread if running
    m_runMgr.stopWaitThread();

    // Wait for the end of live protocol before closing
    stopProtocol();
    m_runMgr.waitForProtocol();

    //Reset input view
    size_t inputNb = m_pProtocol->getInputCount();
    if(inputNb > 1)
        m_inputViewMgr.clear(m_pProtocol->getTask(m_pProtocol->getRootId()));

    if(inputNb > 1 && m_inputViewMode == ProtocolInputViewMode::CURRENT)
        m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getRootId()));

    //Protocol must be set to null before reloading source data
    m_runMgr.setProtocol(nullptr);
    m_pProtocol = nullptr;
    m_pDataMgr->reloadCurrent();

    if(m_interactionLayerInfo.m_pLayer)
    {
        assert(m_pGraphicsMgr);
        m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
        m_interactionLayerInfo.m_pLayer = nullptr;
    }
}

void CProtocolManager::onSearchProtocol(const QString &text)
{
    QStringList protocolNames = m_dbMgr.searchProtocols(text);
    m_pModel->setStringList(protocolNames);
}

void CProtocolManager::onOutputChanged()
{
    assert(m_pResultsMgr);
    auto taskId = m_pProtocol->getActiveTaskId();
    auto taskPtr = m_pProtocol->getTask(taskId);
    QModelIndex inputIndex = m_inputs[0].getModelIndex(0);
    m_pResultsMgr->manageOutputs(taskPtr, taskId, inputIndex);
}

void CProtocolManager::onUpdateTaskGraphicsInput(const GraphicsProcessInputPtr &inputPtr)
{
    assert(m_pGraphicsMgr);
    assert(m_pProjectMgr);

    auto pLayer = m_pGraphicsMgr->getLayer(m_pGraphicsMgr->getCurrentLayerIndex());
    if(pLayer)
        inputPtr->setLayer(pLayer);
    else
        inputPtr->clearData();
}

void CProtocolManager::onUpdateProcessInfo(bool bFullEdit, const CProcessInfo &info)
{
    assert(m_pProcessMgr);
    m_pProcessMgr->onUpdateProcessInfo(bFullEdit, info);
    auto pTask = m_pProtocol->getTask(m_pProtocol->getActiveTaskId());
    emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
}

void CProtocolManager::onNotifyVideoStart(int frameCount)
{
    if(m_pProtocol)
    {
        m_pProtocol->notifyVideoStart(frameCount);
        m_protocolTimer.start();
    }
}

void CProtocolManager::onSendProcessAction(const ProtocolTaskPtr& pTask, int flags)
{
    try
    {
        pTask->executeActions(flags);
    }
    catch (std::exception& e)
    {
        m_runMgr.protocolErrorHandling(e);
    }
}

void CProtocolManager::onStopThread()
{
    m_runMgr.stopWaitThread();
}

void CProtocolManager::onAllProcessReloaded()
{
    if(m_pProtocol == nullptr)
        return;

    assert(m_pProcessMgr);
    auto rangeIt = m_pProtocol->getVertices();

    for(auto it=rangeIt.first; it!=rangeIt.second; ++it)
    {
        auto taskPtr = m_pProtocol->getTask(*it);
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
                m_pProtocol->replaceTask(newTaskPtr, *it);
                if(m_pProtocol->getActiveTaskId() == *it)
                    emit doUpdateTaskInfo(newTaskPtr, processInfo);
            }
        }
    }
}

void CProtocolManager::onProcessReloaded(const QString &name)
{
    if(m_pProtocol == nullptr)
        return;

    assert(m_pProcessMgr);

    auto taskId = m_pProtocol->getTaskId(name.toStdString());
    if(taskId == boost::graph_traits<ProtocolGraph>::null_vertex())
        return;

    auto taskPtr = m_pProtocol->getTask(taskId);
    auto processInfo = m_pProcessMgr->getProcessInfo(taskPtr->getName());
    //Create new instance
    //We pass nullptr as parameter because we want to take into account possible modifications
    //in implementation side -> in this case new instance must be created
    auto newTaskPtr = m_pProcessMgr->createObject(taskPtr->getName(), nullptr);
    if(newTaskPtr)
    {
        m_pProtocol->replaceTask(newTaskPtr, taskId);
        if(m_pProtocol->getActiveTaskId() == taskId)
            emit doUpdateTaskInfo(newTaskPtr, processInfo);
    }
}

void CProtocolManager::onProtocolLive(int inputIndex, bool bNewSequence)
{
    if(m_pProtocol == nullptr)
        return;

    try
    {
        setProtocolInput(inputIndex, bNewSequence);
        m_runMgr.runLive(inputIndex);
    }
    catch(std::exception& e)
    {
        emit doStopVideo();
        qCritical(logProtocol).noquote() << e.what();
    }
}

std::vector<ProtocolVertex> CProtocolManager::getAllChilds(const ProtocolVertex& id) const
{
    if(m_pProtocol)
        return m_pProtocol->getAllChilds(id);
    else
        return std::vector<ProtocolVertex>();
}

QStringList CProtocolManager::getProtocolNames() const
{
    return m_pModel->stringList();
}

ProtocolTaskPtr CProtocolManager::getActiveTask() const
{
    if(m_pProtocol == nullptr)
        return nullptr;
    else
    {
        auto id = m_pProtocol->getActiveTaskId();
        auto pCurrentTask = m_pProtocol->getTask(id);
        return pCurrentTask;
    }
}

int CProtocolManager::getProtocolDbId() const
{
    if(m_pProtocol == nullptr)
        return -1;

    auto it = m_mapProtocolNameToId.find(QString::fromStdString(m_pProtocol->getName()));
    if(it == m_mapProtocolNameToId.end())
        return -1;
    else
        return it->second;
}

CProtocolInputViewManager *CProtocolManager::getInputViewManager()
{
    return &m_inputViewMgr;
}

void CProtocolManager::onApplyProcess(const QModelIndex &itemIndex, const std::string &processName, const ProtocolTaskParamPtr &pParam)
{
    assert(m_pProcessMgr && m_pProjectMgr);

    try
    {
        if(m_pProtocol == nullptr)
            createProtocol(tr("New workflow").toStdString());

        // Store current item index
        if(itemIndex.isValid() && !m_inputs.empty())
            m_inputs[0].setModelIndex(itemIndex, 0);

        ProtocolVertex newTaskId = addProcess(processName, pParam);
        //Select the added task to ease next connection
        m_pProtocol->setActiveTask(newTaskId);
        emit doSetActiveTask(newTaskId);
        //Update task info
        auto pTask = m_pProtocol->getTask(newTaskId);
        emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
        //Display the next possible task to add to the protocol (empty informative item)
        emit doUpdateCandidateTask();
        //Update protocol module icon to notify change
        emit doSetProtocolChangedIcon();

        m_runMgr.addSequentialRun(newTaskId);
        m_runMgr.runSequentialTask(newTaskId);
    }
    catch(std::exception& e)
    {
        //Connection failed
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
        m_pProtocol->deleteTask(m_pProtocol->getLastTaskId());
    }
}

void CProtocolManager::onActiveTaskChanged(const ProtocolVertex &id)
{    
    if(m_pProtocol == nullptr)
        return;

    //Test if new task
    auto previousId = m_pProtocol->getActiveTaskId();
    if(previousId == id)
        return;

    m_pProtocol->setActiveTask(id);
    auto pTask = m_pProtocol->getTask(id);
    assert(pTask);

    if( m_inputViewMode == ProtocolInputViewMode::CURRENT)
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

void CProtocolManager::onDeleteTask(const ProtocolVertex &id)
{
    assert(m_pProtocol && m_pProjectMgr);

    //Select first parent
    ProtocolVertex parentId = m_pProtocol->getRootId();
    auto parents = m_pProtocol->getParents(id);

    if(parents.size() > 0)
        parentId = parents.front();

    auto pTask = m_pProtocol->getTask(parentId);
    if(pTask != nullptr)
    {
        //Set the parent task as active
        m_pProtocol->setActiveTask(parentId);
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
    if(m_pProtocol->isConnectedToRoot(id))
        bRootChanged = true;

    //Delete task
    m_pProtocol->deleteTask(id);

    if(bRootChanged)
        rootConnectionChanged();
}

void CProtocolManager::onDeleteEdge(const ProtocolEdge &id)
{
    assert(m_pProtocol);
    bool bRootChanged = false;
    auto srcTaskId = m_pProtocol->getEdgeSource(id);

    if(srcTaskId == m_pProtocol->getRootId())
        bRootChanged = true;

    m_pProtocol->deleteEdge(id);

    if(bRootChanged)
        rootConnectionChanged();
}

void CProtocolManager::onConnectTask(const ProtocolVertex &srcId, int srcIndex, const ProtocolVertex &dstId, int dstIndex)
{
    assert(m_pProtocol);

    try
    {
        auto edgeId = m_pProtocol->connect(srcId, srcIndex, dstId, dstIndex);
        emit doAddConnection(edgeId, srcId, srcIndex, dstId, dstIndex);

        if(srcId == m_pProtocol->getRootId())
            rootConnectionChanged();
    }
    catch(std::exception& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

void CProtocolManager::onRunFromActiveTask(const ProtocolTaskParamPtr &pParam)
{
    assert(m_pProtocol);
    auto id = m_pProtocol->getActiveTaskId();
    auto pTask = m_pProtocol->getTask(id);

    if(pTask == nullptr)
        qCCritical(logProtocol).noquote() << tr("Invalid workflow current task");
    else
    {
        pTask->setParam(pParam);
        // Update task item view in order to take into account #port changes
        emit doUpdateTaskItemView(pTask, id);
        runProtocolFromActiveTask();
    }
}

void CProtocolManager::onRunProtocolFinished()
{
    assert(m_pProtocol && m_pProjectMgr);

    emit m_pProtocol->getSignalRawPtr()->doFinish();
    emit doSetElapsedTime(m_runMgr.getTotalElapsedTime());
    updateProcessingFPS();
    emit m_pProtocol->getSignalRawPtr()->doFinish();
    auto taskId = m_pProtocol->getActiveTaskId();
    auto pTask = m_pProtocol->getTask(taskId);
    auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();

    if(m_pProtocol->isBatchMode())
    {
        if(m_bAutoLoadBatchResult)
            m_pProjectMgr->onLoadFolder(QString::fromStdString(m_pProtocol->getOutputFolder()), currentModelIndex);
    }
    else if(pTask && m_pResultsMgr)
    {
        if(m_inputViewMode == ProtocolInputViewMode::CURRENT)
            m_inputViewMgr.manageInputs(pTask);
        else
        {
            // Video only -> update input video frame and result frame at the same time
            m_inputViewMgr.manageOriginVideoInput(m_currentVideoInputIndex);
        }
        updateDataInfo();
        m_pResultsMgr->manageOutputs(pTask, taskId, currentModelIndex);
    }
    emit doFinishedProtocol();
}

void CProtocolManager::onInputDataChanged(const QModelIndex& itemIndex, int inputIndex, bool bNewSequence)
{
    assert(m_pProjectMgr);
    try
    {
        if(m_pProtocol == nullptr)
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
                onProtocolLive(m_currentVideoInputIndex, false);
            else
            {
                clearAllTasks();
                setProtocolInput(m_currentVideoInputIndex, bNewSequence);
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
                m_pProtocol->stop();

            if(bNewSequence == true)
                clearAllTasks();

            setProtocolInput(inputIndex, bNewSequence);
        }

        //Reload source data if necessary
        if(bNewSequence == true && m_inputViewMode == ProtocolInputViewMode::CURRENT)
            m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getRootId()));
    }
    catch(std::exception& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

void CProtocolManager::onLoadProtocol(const QModelIndex &itemIndex)
{
    assert(m_pProcessMgr);
    assert(m_pGraphicsMgr);

    if(!itemIndex.isValid())
    {
        qCCritical(logProtocol).noquote() << tr("Load workflow error: invalid item index");
        return;
    }

    QString protocolName = itemIndex.data(Qt::DisplayRole).toString();
    auto it = m_mapProtocolNameToId.find(protocolName);

    if(it != m_mapProtocolNameToId.end())
    {
        try
        {
            if(m_pProtocol != nullptr)
                emit doCloseProtocol();

            m_pProtocol = m_dbMgr.load(it->second, m_pProcessMgr, m_pGraphicsMgr->getContext());
            if(m_pProtocol)
                initProtocol();
        }
        catch(std::exception& e)
        {
            qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CProtocolManager::onDeleteProtocol(const QModelIndex &itemIndex)
{
    if(!itemIndex.isValid())
    {
        qCCritical(logProtocol).noquote() << tr("Delete workflow error: invalid item index");
        return;
    }

    QString protocolName = itemIndex.data(Qt::DisplayRole).toString();
    auto it = m_mapProtocolNameToId.find(protocolName);

    if(it != m_mapProtocolNameToId.end())
    {
        if(m_pProtocol && m_pProtocol->getName() == protocolName.toStdString())
        {
            emit doCloseProtocol();                
            m_pProtocol = nullptr;
        }
        m_dbMgr.remove(it->second);
        m_pModel->removeRow(itemIndex.row(), itemIndex.parent());

        auto itTmp = m_mapProtocolIdToName.find(it->second);
        m_mapProtocolIdToName.erase(itTmp);
        m_mapProtocolNameToId.erase(it);
    }
}

void CProtocolManager::onDeleteInput(int index)
{
    assert(m_pProtocol);

    if(index < (int)m_inputs.size())
        m_inputs.erase(m_inputs.begin() + index);

    auto rootId = m_pProtocol->getRootId();
    m_pProtocol->deleteOutEdges(rootId, index);
    m_pProtocol->removeInput(index);

    //Update inputs vizualisation if view mode is ORIGIN or active task is connected to root
    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
        m_inputViewMgr.manageInputs(m_pProtocol->getTask(rootId));
    else if(m_pProtocol->isConnectedToRoot(m_pProtocol->getActiveTaskId()))
        m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getActiveTaskId()));

    //Notify CProtocolScene
    emit doInputRemoved(index);
}

void CProtocolManager::onSetTaskActionFlag(const ProtocolVertex &id, CProtocolTask::ActionFlag action, bool bEnable)
{
    assert(m_pProtocol);
    m_pProtocol->setTaskActionFlag(id, action, bEnable);

    if(action == CProtocolTask::ActionFlag::OUTPUT_AUTO_EXPORT)
        emit doUpdateTaskIOInfo(m_pProtocol->getTask(id));
}

void CProtocolManager::onSetGraphicsLayerInput(const ProtocolVertex &id, int portIndex, const QModelIndex &layerIndex)
{
    assert(m_pGraphicsMgr);
    assert(m_pProtocol);

    auto pTask = m_pProtocol->getTask(id);
    if(pTask == nullptr)
    {
        qCCritical(logProtocol).noquote() << tr("Invalid task: graphics layer input can't be set.");
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
            qCCritical(logProtocol).noquote() << tr("Invalid graphics layer: input can't be set.");
            return;
        }

        try
        {
            m_pGraphicsMgr->setCurrentLayer(layerIndex, true);
            pTask->setInput(std::make_shared<CGraphicsProcessInput>(pLayer), portIndex);
            emit doInputAssigned(id, portIndex, true);
        }
        catch(std::exception& e)
        {
            qCritical(logProtocol).noquote() << e.what();
        }
    }
}

void CProtocolManager::onSetFolderPathInput(const ProtocolVertex &id, int index, const QString &path)
{
    assert(m_pProtocol);

    auto taskPtr = m_pProtocol->getTask(id);
    if(taskPtr == nullptr)
    {
        qCCritical(logProtocol).noquote() << tr("Invalid task: folder path can't be set.");
        return;
    }

    try
    {
        taskPtr->setInput(std::make_shared<CPathIO>(IODataType::FOLDER_PATH, path.toStdString()), index);
        emit doInputAssigned(id, index, true);
    }
    catch(std::exception& e)
    {
        qCritical(logProtocol).noquote() << e.what();
    }
}

void CProtocolManager::onSetInteractionGraphicsLayer(CGraphicsLayer *pLayer)
{
    assert(m_pGraphicsMgr);
    m_interactionLayerInfo = CGraphicsLayerInfo(pLayer, 0, CGraphicsLayerInfo::SOURCE, true);
    m_pGraphicsMgr->addTemporaryLayer(m_interactionLayerInfo);
    m_pGraphicsMgr->setCurrentLayer(pLayer);
}

void CProtocolManager::onSetGraphicsTool(GraphicsShape tool)
{
    assert(m_pGraphicsMgr);
    m_pGraphicsMgr->setCurrentTool(tool);
}

void CProtocolManager::onSetGraphicsCategory(const QString &category)
{
    assert(m_pGraphicsMgr);
    m_pGraphicsMgr->setCurrentCategory(category);
}

void CProtocolManager::onSetInput(int inputIndex, const CProtocolInput &input)
{
    if(inputIndex >= (int)m_inputs.size())
        return;

    m_inputs[inputIndex] = input;

    try
    {        
        setProtocolInput(inputIndex, true);

        //Update origin inputs vizualisation
        if(m_inputViewMode == ProtocolInputViewMode::ORIGIN || m_pProtocol->getActiveTaskId() == m_pProtocol->getRootId())
            m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getRootId()));
    }
    catch(std::exception& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

void CProtocolManager::onIODisplaysSelected(DisplayCategory category, int index)
{
    if(isProtocolExists())
    {
        if(category ==  DisplayCategory::OUTPUT)
            m_pResultsMgr->notifyDisplaySelected(index);

        updateDataInfo();
    }
}

void CProtocolManager::onGetProtocolInfo(const QModelIndex &index)
{
    if(!index.isValid())
    {
        qCDebug(logProtocol) << tr("Retrieve workflow information failed: invalid item index");
        return;
    }

    QString protocolName = index.data(Qt::DisplayRole).toString();
    auto it = m_mapProtocolNameToId.find(protocolName);

    if(it != m_mapProtocolNameToId.end())
    {
        try
        {
            auto description = m_dbMgr.getProtocolDescription(it->second);
            emit doSetDescription(description);
        }
        catch(std::exception& e)
        {
            qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CProtocolManager::onClearInteractionGraphicsLayer(CGraphicsLayer *pLayer)
{
    assert(m_pGraphicsMgr);
    assert(m_interactionLayerInfo.m_pLayer == pLayer);
    m_pGraphicsMgr->removeTemporaryLayer(m_interactionLayerInfo);
    m_interactionLayerInfo.m_pLayer = nullptr;
}

void CProtocolManager::onAddProcess(const std::string& processName, const ProtocolTaskParamPtr& pParam)
{
    assert(m_pProcessMgr && m_pProjectMgr);

    try
    {
        if(m_pProtocol == nullptr)
            createProtocol(tr("New workflow").toStdString());

        ProtocolVertex newTaskId = addProcess(processName, pParam);
        //Set the added task as active to ease next connection
        m_pProtocol->setActiveTask(newTaskId);
        emit doSetActiveTask(newTaskId);
        //Update task info
        auto pTask = m_pProtocol->getTask(newTaskId);
        updateExecTimeInfo(newTaskId);
        emit doUpdateTaskInfo(pTask, m_pProcessMgr->getProcessInfo(pTask->getName()));
        //Display the next possible task to add to the protocol (empty informative item)
        emit doUpdateCandidateTask();
    }
    catch(CException& e)
    {
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());

        //Check if connection failed so we have to delete task from protocol
        if(e.getCode() == CoreExCode::INVALID_CONNECTION)
            m_pProtocol->deleteTask(m_pProtocol->getLastTaskId());
    }
}

void CProtocolManager::onAddTaskToProtocolView(const ProtocolVertex &id)
{
    if(isRoot(id) == false)
    {
        //Get first parent
        auto inRangeIt = m_pProtocol->getInEdges(id);
        auto parentId = m_pProtocol->getEdgeSource(*inRangeIt.first);
        //Get task
        auto pProcess = m_pProtocol->getTask(id);
        //Add task to view
        emit doAddTask(pProcess, id, parentId);
    }
}

void CProtocolManager::onAddInput(const CProtocolInput &input)
{
    assert(m_pProtocol);
    m_inputs.push_back(input);

    try
    {
        setProtocolInput((int)m_pProtocol->getInputCount(), true);

        //Update origin inputs vizualisation if view mode is ORIGIN
        if(m_inputViewMode == ProtocolInputViewMode::ORIGIN || m_pProtocol->getActiveTaskId() == m_pProtocol->getRootId())
            m_inputViewMgr.manageInputs(m_pProtocol->getTask(m_pProtocol->getRootId()));
    }
    catch(std::exception& e)
    {
        m_inputs.pop_back();
        qCCritical(logProtocol).noquote() << QString::fromStdString(e.what());
    }
}

void CProtocolManager::onFinishTask(const ProtocolVertex& id, CProtocolTask::State status, const QString& msg)
{
    if(m_pProtocol == nullptr)
        return;

    // Update info for active task
    updateExecTimeInfo(m_pProtocol->getActiveTaskId());

    if(m_pProtocol->isValid(id))
        emit doSetTaskState(id, status, msg);
}

void CProtocolManager::onQueryGraphicsProxyModel()
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

void CProtocolManager::onQueryProjectDataProxyModel(const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters)
{
    assert(m_pProjectMgr);
    assert(m_inputs.size() > 0);

    // Case where there are no data loaded + a protocol has been loaded + someone clicked on graphics port
    auto currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentModelIndex.isValid() == false)
        return;

    auto pDataProxyModel = m_pProjectMgr->getDataProxyModel(currentModelIndex, dataTypes, filters);
    emit doSetProjectDataProxyModel(pDataProxyModel);
}

void CProtocolManager::initGlobalConnections()
{
    //Run manager -> protocol manager
    connect(&m_runMgr, &CProtocolRunManager::doSetElapsedTime, [&](double time){ emit doSetElapsedTime(time); });
    connect(&m_runMgr, &CProtocolRunManager::doProtocolLive, this, &CProtocolManager::onProtocolLive, Qt::BlockingQueuedConnection);
}

void CProtocolManager::onQueryIOInfo(const ProtocolVertex &taskId, int index, bool bInput)
{
    if(m_pProtocol)
    {
        auto info = m_pProtocol->getIOInfo(taskId, index, bInput);
        emit doSetIOInfo(info, taskId, index, bInput);
    }
}

void CProtocolManager::initConnections()
{
    //Protocol run manager -> protocol manager
    connect(&m_runMgr, &CProtocolRunManager::doProtocolFinished, this, &CProtocolManager::onRunProtocolFinished);

    //Protocol -> Protocol manager
    auto pHandler = static_cast<CProtocolSignalHandler*>(m_pProtocol->getSignalRawPtr());
    connect(pHandler, &CProtocolSignalHandler::doFinishTask, this, &CProtocolManager::onFinishTask);
    connect(pHandler, &CProtocolSignalHandler::doAddGraphicsLayer, this, &CProtocolManager::onSetInteractionGraphicsLayer);
    connect(pHandler, &CProtocolSignalHandler::doRemoveGraphicsLayer, this, &CProtocolManager::onClearInteractionGraphicsLayer);
    connect(pHandler, &CProtocolSignalHandler::doOutputChanged, this, &CProtocolManager::onOutputChanged);
    connect(pHandler, &CProtocolSignalHandler::doUpdateTaskGraphicsInput, this, &CProtocolManager::onUpdateTaskGraphicsInput);

    connect(pHandler, &CProtocolSignalHandler::doUpdateTaskItemView, [&](const ProtocolTaskPtr& taskPtr, const ProtocolVertex& id)
    {
        emit doUpdateTaskItemView(taskPtr, id);
    });
    connect(pHandler, &CProtocolSignalHandler::doSetTaskState, [&](const ProtocolVertex& id, CProtocolTask::State state, const QString& msg)
    {
        emit doSetTaskState(id, state, msg);
    });
    connect(pHandler, &CProtocolSignalHandler::doDeleteConnection, [&](const ProtocolEdge& id)
    {
        emit doDeleteConnection(id);
    });
    connect(pHandler, &CProtocolSignalHandler::doGraphicsContextChanged, [&]
    {
        assert(m_pGraphicsMgr);
        emit m_pGraphicsMgr->doGraphicsContextChanged();
    });
    connect(pHandler, &CProtocolSignalHandler::doLog, [&](const QString& msg)
    {
        qCInfo(logProtocol).noquote() << msg;
    });

    //Protocol -> ProgressBar manager
    connect(m_pProtocol->getSignalRawPtr(), &CSignalHandler::doSetMessage, m_pProgressMgr, &CProgressBarManager::onSetMessage);
}

void CProtocolManager::initProtocol()
{
    assert(m_pProtocol);

    m_runMgr.setProtocol(m_pProtocol);
    //Set I/O object factory
    m_pProtocol->setTaskIORegistration(&m_pProcessMgr->m_ioRegistrator);
    emit doProtocolCreated();

    //Set inputs
    initInputs();

    for(size_t i=0; i<m_inputs.size(); ++i)
        setProtocolInput(i, true);

    //Create signals/slots connections
    initConnections();

    //Create protocol view
    buildProtocolView();
    m_pProtocol->updateHash();
    emit doNewProtocolNotification(QString("Workflow %1 has been created.").arg(QString::fromStdString(m_pProtocol->getName())), Notification::INFO);
}

void CProtocolManager::initInputs()
{
    assert(m_pProjectMgr);
    m_inputs.clear();

    QModelIndex currentModelIndex = m_pProjectMgr->getCurrentDataItemIndex();
    if(currentModelIndex.isValid())
    {
        TreeItemType type = m_pProjectMgr->getItemType(currentModelIndex);
        CProtocolInput input(ProtocolInputMode::CURRENT_DATA, type);
        input.appendModelIndex(currentModelIndex);
        m_inputs.push_back(input);
    }

    if(m_pProtocol)
    {        
        // Get other possible inputs different from current data (ie index = 0)
        std::set<size_t> connectedIndices;
        ProtocolVertex root = m_pProtocol->getRootId();
        auto outEdges = m_pProtocol->getOutEdges(root);

        for(auto it=outEdges.first; it!=outEdges.second; ++it)
        {
            ProtocolEdgePtr edge = m_pProtocol->getEdge(*it);
            size_t index = edge->getSourceIndex();

            if(index > 0)
                connectedIndices.insert(edge->getSourceIndex());
        }

        for(auto it=connectedIndices.begin(); it!=connectedIndices.end(); ++it)
            m_inputs.push_back(CProtocolInput());
    }
}

void CProtocolManager::loadProtocols()
{
    QStringList protocolNames;

    if(m_pModel == nullptr)
        m_pModel = new QStringListModel(this);

    m_mapProtocolNameToId = m_dbMgr.getProtocols();
    for(auto it=m_mapProtocolNameToId.begin(); it!=m_mapProtocolNameToId.end(); ++it)
    {
        m_mapProtocolIdToName.insert(std::make_pair(it->second, it->first));
        protocolNames.push_back(it->first);
    }
    m_pModel->setStringList(protocolNames);
    emit doSetNamesModel(m_pModel);
}

void CProtocolManager::buildProtocolView()
{
    assert(m_pProtocol);

    //Depth first search to build the protocol view in the right order
    auto pHandler = static_cast<CProtocolSignalHandler*>(m_pProtocol->getSignalRawPtr());
    connect(pHandler, &CProtocolSignalHandler::doDfsTaskVisited, this, &CProtocolManager::onAddTaskToProtocolView);
    m_pProtocol->startDepthFirstSearch();
    disconnect(pHandler, &CProtocolSignalHandler::doDfsTaskVisited, this, &CProtocolManager::onAddTaskToProtocolView);

    //Add connections
    auto edgeRangeIt = m_pProtocol->getEdges();
    for(auto it=edgeRangeIt.first; it!=edgeRangeIt.second; ++it)
    {
        auto srcId = m_pProtocol->getEdgeSource(*it);
        auto targetId = m_pProtocol->getEdgeTarget(*it);
        auto pEdge = m_pProtocol->getEdge(*it);
        emit doAddConnection(*it, srcId, pEdge->getSourceIndex(), targetId, pEdge->getTargetIndex());
    }

    //Select last added task
    m_pProtocol->setActiveTask(m_pProtocol->getLastTaskId());
    emit doSetActiveTask(m_pProtocol->getLastTaskId());
    //Update task info
    auto lastId = m_pProtocol->getLastTaskId();
    auto pLastTask = m_pProtocol->getTask(lastId);
    updateExecTimeInfo(lastId);
    emit doUpdateTaskInfo(pLastTask, m_pProcessMgr->getProcessInfo(pLastTask->getName()));
    //Update next candidate task position
    emit doUpdateCandidateTask();
}

void CProtocolManager::checkInput(size_t index) const
{
    if(index >= m_inputs.size())
        throw CException(CoreExCode::INDEX_OVERFLOW, tr("Input index overflow.").toStdString(), __func__, __FILE__, __LINE__);

    auto type = m_inputs[index].getType();
    if(!isValidInputItemType(type))
        throw CException(CoreExCode::INVALID_USAGE, tr("Invalid input type.").toStdString(), __func__, __FILE__, __LINE__);
}

bool CProtocolManager::isValidInputItemType(TreeItemType type) const
{
    return type == TreeItemType::FOLDER || type == TreeItemType::DATASET ||
            type == TreeItemType::IMAGE || type == TreeItemType::VIDEO || type == TreeItemType::LIVE_STREAM;
}

void CProtocolManager::setProtocolInput(size_t inputIndex, bool bNewSequence)
{
    assert(m_pProtocol);

    if(bNewSequence)
    {
        // May throw
        checkInput(inputIndex);
        // Set batch state
        m_pProtocol->setInputBatchState(inputIndex, isBatchInput(inputIndex));
    }
    //Set first workflow input data
    auto taskIOPtr = m_runMgr.createTaskIO(inputIndex, 0, bNewSequence);
    if(!taskIOPtr)
    {
        qCritical(logProtocol()).noquote() << tr("Workflow input cannot be set: invalid item.");
        return;
    }
    m_pProtocol->setInput(taskIOPtr, inputIndex, bNewSequence);
    // Notify protocol view
    emit doInputChanged(inputIndex);
}

ProtocolVertex CProtocolManager::addProcess(const std::string &name, const ProtocolTaskParamPtr &pParam)
{
    //Get process object
    auto pProcess = m_pProcessMgr->createObject(name, pParam);
    if(pProcess == nullptr)
    {
        std::string errorMsg = "Instanciation failed for object of type: " + name;
        throw CException(CoreExCode::CREATE_FAILED, errorMsg, __func__, __FILE__, __LINE__);
    }

    //Add process to protocol
    pProcess->setGraphicsContext(m_pGraphicsMgr->getContext());
    auto parentTaskId = m_pProtocol->getActiveTaskId();
    auto newTaskId = m_pProtocol->addTask(pProcess);

    //Connection
    std::vector<ProtocolEdge> newEdgeIds;
    try
    {
        newEdgeIds = m_pProtocol->connect(parentTaskId, newTaskId);
    }
    catch(std::exception& e)
    {
        qCritical(logProtocol()).noquote() << QString::fromStdString(e.what());
    }

    //Notify view
    emit doAddCandidateTask(pProcess, newTaskId);
    emit doNewProtocolNotification(QString("A new process (%1) has been added to the workflow.").arg(QString::fromStdString(name)), Notification::INFO);

    for(size_t i=0; i<newEdgeIds.size(); ++i)
    {
        auto edgePtr = m_pProtocol->getEdge(newEdgeIds[i]);
        emit doAddConnection(newEdgeIds[i], parentTaskId, edgePtr->getSourceIndex(), newTaskId, edgePtr->getTargetIndex());
    }

    if(parentTaskId == m_pProtocol->getRootId())
        rootConnectionChanged();

    return newTaskId;
}

void CProtocolManager::updateProcessingFPS()
{
    auto time = m_runMgr.getTotalElapsedTime();
    if(time != 0)
        m_currentFPS = (int)(1000.0/time);
    else
        m_currentFPS = 25;
}

void CProtocolManager::updateExecTimeInfo(const ProtocolVertex& taskId)
{
    m_execTimeInfo.clear();

    auto pTask = m_pProtocol->getTask(taskId);

    // Add here all values that you want to display in protocol module

    // Process elapsed time
    double elapsedTime = pTask->getElapsedTime();
    QString timeStr = timeToString(elapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Process elapsed time", timeStr.toStdString()));

    // Elapsed time to current task
    double currentElapsedTime = m_pProtocol->getElapsedTimeTo(taskId);
    QString currentTimeStr = timeToString(currentElapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Intermediate elapsed time", currentTimeStr.toStdString()));

    // Protocol elapsed time
    double totalElapsedTime = m_pProtocol->getTotalElapsedTime();
    QString totalTimeStr = timeToString(totalElapsedTime);
    m_execTimeInfo.push_back(std::make_pair("Total elapsed time", totalTimeStr.toStdString()));

    emit doUpdateTaskStateInfo(m_execTimeInfo, pTask->getCustomInfo());
}

void CProtocolManager::updateVideoInputIndex(size_t index)
{
    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
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
            if(m_pProjectMgr->isTimeDataItem(m_inputs[i].getModelIndex(0)) && m_pProtocol->isInputConnected(i))
            {
                m_currentVideoInputIndex = i;
                m_inputViewMgr.setLiveInputIndex(i);
                break;
            }
        }
    }
}

QString CProtocolManager::timeToString(double time)
{
    int t = int(time);
    auto ms = int(t%1000);
    auto sec = int((t/1000)%60);
    auto min = int((t/60000)%60);

    QString timeStr = QString(tr("%1 min %2 sec %3 ms")).arg(min).arg(sec).arg(ms);
    return timeStr;
}

void CProtocolManager::clearAllTasks()
{
    if(m_pProtocol == nullptr)
        return;

    //Notify results manager that the protocol will be cleared
    m_pResultsMgr->notifyBeforeProtocolCleared();

    auto vertexList = m_pProtocol->getAllChilds(m_pProtocol->getRootId());
    for(const auto& it : vertexList)
        clearTask(it);
}

void CProtocolManager::clearFrom(const ProtocolVertex& taskId)
{
    if(m_pProtocol == nullptr)
        return;

    //Notify results manager that the task will be cleared
    m_pResultsMgr->notifyBeforeProtocolCleared();

    auto vertexList = m_pProtocol->getAllChilds(taskId);
    vertexList.insert(vertexList.begin(), taskId);

    for(const auto& it : vertexList)
        clearTask(it);
}

void CProtocolManager::clearTo(const ProtocolVertex &taskId)
{
    if(m_pProtocol == nullptr)
        return;

    //Notify results manager that the task will be cleared
    m_pResultsMgr->notifyBeforeProtocolCleared();

    std::vector<ProtocolVertex> vertexList;
    m_pProtocol->getAllParents(taskId, vertexList);
    vertexList.push_back(taskId);

    for(const auto& it : vertexList)
    {
        if(it != m_pProtocol->getRootId())
            clearTask(it);
    }
}

void CProtocolManager::clearTask(const ProtocolVertex& taskId)
{
    assert(m_pProtocol);

    // Check if root and if so, avoid clearing data
    if(m_pProtocol->isRoot(taskId))
        return;

    auto pTask = m_pProtocol->getTask(taskId);
    // Clear all output data
    pTask->clearOutputData();
    // Reset status
    emit doSetTaskState(taskId, CProtocolTask::State::UNDONE);
}

void CProtocolManager::rootConnectionChanged()
{
    for(size_t i=0; i<m_inputs.size(); ++i)
    {
        if(m_inputs[i].getType() == TreeItemType::FOLDER)
            setProtocolInput(i, true);
    }
}

void CProtocolManager::updateDataInfo()
{
    assert(m_pDataMgr);
    ProtocolTaskPtr taskPtr;
    ProtocolTaskIOPtr ioPtr;
    IODataType dataType;

    if(m_pDataMgr->getSelectedDisplayCategory() == DisplayCategory::INPUT)
    {
        if(m_inputViewMode == ProtocolInputViewMode::CURRENT)
            taskPtr = m_pProtocol->getTask(m_pProtocol->getActiveTaskId());
        else
            taskPtr = m_pProtocol->getTask(m_pProtocol->getRootId());

        if(taskPtr == nullptr)
            return;

        ioPtr = taskPtr->getInput(m_pDataMgr->getSelectedDisplayIndex());
        if(ioPtr == nullptr)
            return;

        dataType = ioPtr->getDataType();
    }
    else
    {
        taskPtr = m_pProtocol->getTask(m_pProtocol->getActiveTaskId());
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
        auto image = std::static_pointer_cast<CVideoProcessIO>(ioPtr)->getImage();
        m_pDataMgr->getVideoMgr()->setSelectedImageFromWorkflow(image);
    }
}

void CProtocolManager::reloadCurrentPlugins()
{
    CPyEnsureGIL gil;
    if(m_pProtocol == nullptr)
        return;

    assert(m_pProcessMgr);
    auto rangeIt = m_pProtocol->getVertices();

    for(auto it=rangeIt.first; it!=rangeIt.second; ++it)
    {
        auto taskPtr = m_pProtocol->getTask(*it);
        auto processInfo = m_pProcessMgr->getProcessInfo(taskPtr->getName());

        if(processInfo.isInternal() == false)
            m_pProcessMgr->onReloadPlugin(QString::fromStdString(taskPtr->getName()), processInfo.getLanguage());
    }
}

void CProtocolManager::saveCurrentInputImage(size_t index)
{
    ProtocolTaskPtr taskPtr;
    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
        taskPtr = m_pProtocol->getTask(m_pProtocol->getRootId());
    else
        taskPtr = m_pProtocol->getTask(m_pProtocol->getActiveTaskId());

    auto inputPtr = std::dynamic_pointer_cast<CImageProcessIO>(taskPtr->getInput(index));
    if(!inputPtr)
    {
        qCCritical(logProtocol).noquote() << tr("No valid image data for input #%1 of task %2.").arg(index+1).arg(QString::fromStdString(taskPtr->getName()));
        return;
    }
    std::string path = taskPtr->getOutputFolder() + taskPtr->getName() + Utils::Data::getFileFormatExtension(DataFileFormat::PNG);
    m_pDataMgr->getImgMgr()->exportImage(inputPtr->getImage(), std::vector<ProxyGraphicsItemPtr>(), QString::fromStdString(path));
}

void CProtocolManager::exportCurrentInputImage(size_t index, const QString &path, bool bWithGraphics)
{
    ProtocolTaskPtr taskPtr;
    if(m_inputViewMode == ProtocolInputViewMode::ORIGIN)
        taskPtr = m_pProtocol->getTask(m_pProtocol->getRootId());
    else
        taskPtr = m_pProtocol->getTask(m_pProtocol->getActiveTaskId());

    auto inputPtr = std::dynamic_pointer_cast<CImageProcessIO>(taskPtr->getInput(index));
    if(!inputPtr)
    {
        qCCritical(logProtocol).noquote() << tr("No valid image data for input #%1 of task %2.").arg(index+1).arg(QString::fromStdString(taskPtr->getName()));
        return;
    }

    std::vector<ProxyGraphicsItemPtr> graphicsItems;
    if(bWithGraphics)
    {
        auto graphicsInputs = taskPtr->getInputs( {IODataType::INPUT_GRAPHICS} );
        for(size_t i=0; i<graphicsInputs.size(); ++i)
        {
            auto graphicsInput = std::dynamic_pointer_cast<CGraphicsProcessInput>(graphicsInputs[i]);
            if(graphicsInput)
            {
                auto items = graphicsInput->getItems();
                graphicsItems.insert(graphicsItems.end(), items.begin(), items.end());
            }
        }
    }
    m_pDataMgr->getImgMgr()->exportImage(inputPtr->getImage(), graphicsItems, path);
}

void CProtocolManager::playVideoInput(size_t index)
{
    // If protocol is running, don't do anything
    if(isProtocolRunning())
        return;

    updateVideoInputIndex(index);
    m_pDataMgr->getVideoMgr()->play(m_inputs[m_currentVideoInputIndex].getModelIndex(0), index);
}

#include "moc_CProtocolManager.cpp"
