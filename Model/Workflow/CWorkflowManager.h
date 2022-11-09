/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CWORKFLOWMANAGER_H
#define CWORKFLOWMANAGER_H

#include <QObject>
#include "Core/CWorkflow.h"
#include "Model/Project/CProjectManager.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"
#include "Model/Workflow/CWorkflowRunManager.h"
#include "Model/User/CUser.h"
#include "CWorkflowInputViewManager.h"
#include "CWorkflowDBManager.h"

class CProcessManager;
class CProjectManager;
class CGraphicsManager;
class CProgressBarManager;
class CResultManager;
class CProgressCircle;
class CMainDataManager;
class CSettingsManager;

class CWorkflowManager : public QObject
{
    Q_OBJECT

    public:

        CWorkflowManager();
        ~CWorkflowManager();

        //Getters
        std::string                 getWorkflowName() const;
        WorkflowVertex              getRootId() const;
        WorkflowTaskIOPtr           getInput(size_t index) const;
        IODataType                  getInputDataType(size_t index) const;
        WorkflowInputViewMode       getInputViewMode() const;
        std::vector<WorkflowVertex> getAllChilds(const WorkflowVertex& id) const;
        QStringList                 getWorkflowNames() const;
        WorkflowTaskPtr             getActiveTask() const;
        int                         getWorkflowDbId() const;
        CWorkflowInputViewManager*  getInputViewManager();
        int                         getCurrentFPS() const;
        QModelIndex                 getCurrentVideoInputModelIndex() const;
        std::vector<int>            getDisplayedInputIndices(const WorkflowTaskPtr& taskPtr, const std::set<IODataType> &types) const;

        //Setters
        void                        setManagers(CProcessManager* pProcessMgr, CProjectManager* pProjectMgr, CGraphicsManager* pGraphicsMgr,
                                                CResultManager* pResultsMgr, CMainDataManager* pDataMgr, CProgressBarManager* pProgressMgr,
                                                CSettingsManager* pSettingsMgr);
        void                        setWorkflowName(const std::string& name);
        void                        setWorkflowKeywords(const std::string& keywords);
        void                        setWorkflowDescription(const std::string& description);
        void                        setInputViewMode(WorkflowInputViewMode mode);
        void                        setCurrentUser(const CUser& user);
        void                        setCurrentTaskAutoSave(size_t outputIndex, bool bAutoSave);
        void                        setCurrentTaskSaveFolder(const std::string& path);
        void                        setCurrentTaskSaveFormat(size_t outputIndex, size_t formatIndex);
        void                        setCurrentTaskSaveFormat(size_t outputIndex, DataFileFormat format);
        void                        setWorkflowConfig(const std::string& key, const std::string& value);

        void                        notifyViewShow();
        void                        notifyGraphicsChanged();

        void                        enableAutoLoadBatchResults(bool bEnable);
        void                        enableWholeVideo(bool bEnable);

        void                        beforeProjectClose(bool bWithCurrentImage);
        void                        beforeDataDeleted(const QModelIndex& index);
        void                        beforeDataDeleted(const std::vector<QModelIndex>& indexes);
        void                        beforeGraphicsLayerRemoved(CGraphicsLayer* pLayer);

        bool                        isRoot(const WorkflowVertex& id) const;
        bool                        isWorkflowExists() const;
        bool                        isWorkflowModified() const;
        bool                        isWorkflowRunning() const;
        bool                        isBatchInput(size_t index) const;

        void                        createWorkflow(const std::string& name, const std::string& keywords="", const std::string& description="");
        WorkflowTaskWidgetPtr       createTaskWidget(const WorkflowTaskPtr &pTask);

        void                        runWorkflow();
        void                        runWorkflowFromActiveTask();
        void                        runWorkflowToActiveTask();

        void                        stopWorkflow();

        void                        saveWorkflow();
        void                        saveWorkflow(const QString& path);

        void                        loadWorkflow(const QString& path);
        void                        loadImageWorkflows(const QModelIndex& imageIndex);

        void                        updateDataInfo();

        void                        reloadCurrentPlugins();

        void                        saveCurrentInputImage(size_t index);
        void                        exportCurrentInputImage(size_t index, const QString& path, bool bWithGraphics);

        void                        playVideoInput(size_t index);

    public slots:

        void                        onApplyProcess(const QModelIndex& itemIndex, const std::string& processName, const WorkflowTaskParamPtr& pParam);

        void                        onActiveTaskChanged(const WorkflowVertex& id);
        void                        onInputDataChanged(const QModelIndex& itemIndex, int inputIndex, bool bNewSequence);

        void                        onDeleteTask(const WorkflowVertex& id);
        void                        onDeleteEdge(const WorkflowEdge& id);
        void                        onDeleteWorkflow(const QModelIndex& itemIndex);
        void                        onDeleteInput(int index);

        void                        onConnectTask(const WorkflowVertex& srcId, int srcIndex, const WorkflowVertex& dstId, int dstIndex);

        void                        onRunFromActiveTask(const WorkflowTaskParamPtr& pParam);
        void                        onRunWorkflowFinished();
        void                        onRunWorkflowFailed();

        void                        onLoadWorkflow(const QModelIndex& itemIndex);

        void                        onAddProcess(const std::string& processName, const WorkflowTaskParamPtr& pParam);
        void                        onAddTaskToWorkflowView(const WorkflowVertex& id);
        void                        onAddInput(const CWorkflowInput& input);

        void                        onSetTaskActionFlag(const WorkflowVertex& id, CWorkflowTask::ActionFlag action, bool bEnable);
        void                        onSetGraphicsLayerInput(const WorkflowVertex& id, int portIndex, const QModelIndex& layerIndex);
        void                        onSetFolderPathInput(const WorkflowVertex &id, int index, const QString& path);
        void                        onSetInteractionGraphicsLayer(CGraphicsLayer* pLayer);
        void                        onSetGraphicsTool(GraphicsShape tool);
        void                        onSetGraphicsCategory(const QString& category);
        void                        onSetInput(int inputIndex, const CWorkflowInput& input);

        void                        onIODisplaysSelected(DisplayCategory category, int index);

        void                        onGetWorkflowInfo(const QModelIndex& index);

        void                        onClearInteractionGraphicsLayer(CGraphicsLayer* pLayer);

        void                        onFinishTask(const WorkflowVertex& id, CWorkflowTask::State status, const QString& msg = QString());

        void                        onQueryGraphicsProxyModel();
        void                        onQueryProjectDataProxyModel(const std::vector<TreeItemType> &dataTypes, const std::vector<DataDimension> &filters);
        void                        onQueryIOInfo(const WorkflowVertex& taskId, int index, bool bInput);

        void                        onWorkflowClosed();

        void                        onSearchWorkflow(const QString& text);

        void                        onOutputChanged();

        void                        onUpdateTaskGraphicsInput(const GraphicsInputPtr& inputPtr);
        void                        onUpdateProcessInfo(bool bFullEdit, const CTaskInfo& info);

        void                        onNotifyVideoStart(int frameCount);

        void                        onSendProcessAction(const WorkflowTaskPtr& pTask, int flags);

        void                        onStopThread();

        void                        onAllProcessReloaded();
        void                        onProcessReloaded(const QString& name);

    private slots:

        void                        onWorkflowLive(int inputIndex, bool bNewSequence);

    signals:

        void                        doSetManager(CWorkflowManager* pModel);
        void                        doSetNamesModel(QStringListModel* pModel);
        void                        doSetNamesFromImageModel(QStringListModel* pModel);
        void                        doSetActiveTask(const WorkflowVertex& id);
        void                        doSetElapsedTime(double time);
        void                        doSetMessage(QString msg);
        void                        doSetTaskState(const WorkflowVertex& id, CWorkflowTask::State state, const QString& msg=QString());
        void                        doSetGraphicsProxyModel(CProjectGraphicsProxyModel* pModel);
        void                        doSetProjectDataProxyModel(CProjectDataProxyModel* pModel);
        void                        doSetWorkflowChangedIcon();
        void                        doSetDescription(const QString& text);
        void                        doSetIOInfo(const CDataInfoPtr& info, const WorkflowVertex& taskId, int index, bool bInput);

        void                        doAddTask(const WorkflowTaskPtr& pTask, const WorkflowVertex& id, const WorkflowVertex& parentId);
        void                        doAddCandidateTask(const WorkflowTaskPtr& pTask, const WorkflowVertex& id);
        void                        doAddConnection(const WorkflowEdge& id, const WorkflowVertex& srcId, size_t srcIndex, const WorkflowVertex& dstId, size_t dstIndex);

        void                        doUpdateCandidateTask();
        void                        doUpdateTaskInfo(const WorkflowTaskPtr& pTask, const CTaskInfo& info);
        void                        doUpdateTaskIOInfo(const WorkflowTaskPtr& taskPtr);
        void                        doUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap);
        void                        doUpdateTaskItemView(const WorkflowTaskPtr& pTask, const WorkflowVertex& id);
        void                        doUpdateTaskState(const WorkflowVertex& id, CWorkflowTask::State status, const QString& msg = QString());
        void                        doUpdateImageInfo(const VectorPairString& infoList);

        void                        doWorkflowCreated();
        void                        doWorkflowFinished();
        void                        doWorkflowFailed();

        void                        doInputChanged(int index);
        void                        doInputRemoved(int index);
        void                        doInputsCleared();
        void                        doInputAssigned(const WorkflowVertex& id, int portIndex, bool bAssigned);

        void                        doCloseWorkflow();

        void                        doNewWorkflowNotification(const QString& message, Notification type, CProgressCircle* pItem=nullptr, int duration=Ikomia::_NotifDefaultDuration);

        void                        doAbortProgressBar();

        void                        doDeleteConnection(const WorkflowEdge& id);

        void                        doNotifyWorkflowSaved();

        void                        doStopVideo();

    private:

        void                        initGlobalConnections();
        void                        initConnections();
        void                        initWorkflow();
        void                        initInputs();

        void                        loadWorkflows();

        void                        buildWorkflowView();

        bool                        isValidInputItemType(TreeItemType type) const;

        void                        setWorkflowInput(size_t inputIndex, bool bNewSequence);

        WorkflowVertex              addProcess(const std::string& name, const WorkflowTaskParamPtr& pParam);

        void                        updateProcessingFPS();
        void                        updateExecTimeInfo(const WorkflowVertex &taskId);
        void                        updateVideoInputIndex(size_t index);

        QString                     timeToString(double time);

        void                        manageWaitThread(bool bNewSequence);
        void                        stopWaitThread();

        void                        clearAllTasks();
        void                        clearFrom(const WorkflowVertex& taskId);
        void                        clearTo(const WorkflowVertex& taskId);
        void                        clearTask(const WorkflowVertex& taskId);

        void                        checkInput(size_t index) const;

        void                        rootConnectionChanged();

    private:

        const int                   m_maxFps = 25;
        std::mutex                  m_mutex;
        QStringListModel*           m_pModel = nullptr;
        QStringListModel*           m_pImageModel = nullptr;
        //We ensure protocol name unicity by design
        std::map<QString, int>      m_mapWorkflowNameToId;
        std::map<int, QString>      m_mapWorkflowIdToName;
        CWorkflowRunManager         m_runMgr;
        CProcessManager*            m_pProcessMgr = nullptr;
        CProjectManager*            m_pProjectMgr = nullptr;
        CGraphicsManager*           m_pGraphicsMgr = nullptr;
        CResultManager*             m_pResultsMgr = nullptr;
        CWorkflowDBManager          m_dbMgr;
        CProgressBarManager*        m_pProgressMgr = nullptr;
        CMainDataManager*           m_pDataMgr = nullptr;
        CSettingsManager*           m_pSettingsMgr = nullptr;
        CWorkflowInputViewManager   m_inputViewMgr;
        WorkflowPtr                 m_pWorkflow = nullptr;
        CUser                       m_currentUser;
        QFutureSynchronizer<void>   m_sync;
        CWorkflowInputs             m_inputs;
        std::atomic_bool            m_bWorkflowRunning{false};
        Utils::CTimer               m_protocolTimer;
        std::list<WorkflowVertex>   m_sequentialRuns;
        CGraphicsLayerInfo          m_interactionLayerInfo;
        size_t                      m_outputVideoId = -1;
        size_t                      m_currentVideoInputIndex = 0;
        VectorPairString            m_execTimeInfo;
        WorkflowInputViewMode       m_inputViewMode = WorkflowInputViewMode::ORIGIN;
        int                         m_currentFPS = 0;
        bool                        m_bAutoLoadBatchResult = true;
};

#endif // CWORKFLOWMANAGER_H
