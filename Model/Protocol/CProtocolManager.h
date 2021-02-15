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

#ifndef CPROTOCOLMANAGER_H
#define CPROTOCOLMANAGER_H

#include <QObject>
#include "Core/CProtocol.h"
#include "Model/Project/CProjectManager.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"
#include "Model/Protocol/CProtocolRunManager.h"
#include "Model/User/CUser.h"
#include "CProtocolInputViewManager.h"
#include "CProtocolDBManager.h"

class CProcessManager;
class CProjectManager;
class CGraphicsManager;
class CProgressBarManager;
class CResultManager;
class CProgressCircle;
class CMainDataManager;
class CSettingsManager;

class CProtocolManager : public QObject
{
    Q_OBJECT

    public:

        CProtocolManager();
        ~CProtocolManager();

        //Getters
        std::string                 getProtocolName() const;
        ProtocolVertex              getRootId() const;
        ProtocolTaskIOPtr           getInput(size_t index) const;
        IODataType                  getInputDataType(size_t index) const;
        ProtocolInputViewMode       getInputViewMode() const;
        std::vector<ProtocolVertex> getAllChilds(const ProtocolVertex& id) const;
        QStringList                 getProtocolNames() const;
        ProtocolTaskPtr             getActiveTask() const;
        int                         getProtocolDbId() const;
        CProtocolInputViewManager*  getInputViewManager();
        int                         getCurrentFPS() const;
        QModelIndex                 getCurrentVideoInputModelIndex() const;
        std::vector<int>            getDisplayedInputIndices(const ProtocolTaskPtr& taskPtr, const std::set<IODataType> &types) const;
        //Setters
        void                        setManagers(CProcessManager* pProcessMgr, CProjectManager* pProjectMgr, CGraphicsManager* pGraphicsMgr,
                                                CResultManager* pResultsMgr, CMainDataManager* pDataMgr, CProgressBarManager* pProgressMgr,
                                                CSettingsManager* pSettingsMgr);
        void                        setProtocolName(const std::string& name);
        void                        setProtocolKeywords(const std::string& keywords);
        void                        setProtocolDescription(const std::string& description);
        void                        setInputViewMode(ProtocolInputViewMode mode);
        void                        setCurrentUser(const CUser& user);
        void                        setCurrentTaskAutoSave(size_t outputIndex, bool bAutoSave);
        void                        setCurrentTaskSaveFolder(const std::string& path);

        void                        notifyViewShow();
        void                        notifyGraphicsChanged();

        void                        forceBatchMode(bool bEnable);

        void                        beforeProjectClose(bool bWithCurrentImage);
        void                        beforeDataDeleted(const QModelIndex& index);
        void                        beforeDataDeleted(const std::vector<QModelIndex>& indexes);
        void                        beforeGraphicsLayerRemoved(CGraphicsLayer* pLayer);

        bool                        isRoot(const ProtocolVertex& id) const;
        bool                        isProtocolExists() const;
        bool                        isProtocolModified() const;
        bool                        isProtocolRunning() const;
        bool                        isBatchInput(size_t index) const;

        void                        createProtocol(const std::string& name, const std::string& keywords="", const std::string& description="");
        ProtocolTaskWidgetPtr       createTaskWidget(const ProtocolTaskPtr &pTask);

        void                        runProtocol();
        void                        runProtocolFromActiveTask();
        void                        runProtocolToActiveTask();

        void                        stopProtocol();

        void                        saveProtocol();
        void                        saveProtocol(const QString& path);

        void                        loadProtocol(const QString& path);
        void                        loadImageProtocols(const QModelIndex& imageIndex);

        void                        updateDataInfo();

        void                        reloadCurrentPlugins();

        void                        saveCurrentInputImage(size_t index);
        void                        exportCurrentInputImage(size_t index, const QString& path, bool bWithGraphics);

        void                        playVideoInput(size_t index);

    public slots:

        void                        onApplyProcess(const QModelIndex& itemIndex, const std::string& processName, const ProtocolTaskParamPtr& pParam);

        void                        onActiveTaskChanged(const ProtocolVertex& id);
        void                        onInputDataChanged(const QModelIndex& itemIndex, int inputIndex, bool bNewSequence);

        void                        onDeleteTask(const ProtocolVertex& id);
        void                        onDeleteEdge(const ProtocolEdge& id);
        void                        onDeleteProtocol(const QModelIndex& itemIndex);
        void                        onDeleteInput(int index);

        void                        onConnectTask(const ProtocolVertex& srcId, int srcIndex, const ProtocolVertex& dstId, int dstIndex);

        void                        onRunFromActiveTask(const ProtocolTaskParamPtr& pParam);
        void                        onRunProtocolFinished();

        void                        onLoadProtocol(const QModelIndex& itemIndex);

        void                        onAddProcess(const std::string& processName, const ProtocolTaskParamPtr& pParam);
        void                        onAddTaskToProtocolView(const ProtocolVertex& id);
        void                        onAddInput(const CProtocolInput& input);

        void                        onSetTaskActionFlag(const ProtocolVertex& id, CProtocolTask::ActionFlag action, bool bEnable);
        void                        onSetGraphicsLayerInput(const ProtocolVertex& id, int portIndex, const QModelIndex& layerIndex);
        void                        onSetFolderPathInput(const ProtocolVertex &id, int index, const QString& path);
        void                        onSetInteractionGraphicsLayer(CGraphicsLayer* pLayer);
        void                        onSetGraphicsTool(GraphicsShape tool);
        void                        onSetGraphicsCategory(const QString& category);
        void                        onSetInput(int inputIndex, const CProtocolInput& input);

        void                        onIODisplaysSelected(DisplayCategory category, int index);

        void                        onGetProtocolInfo(const QModelIndex& index);

        void                        onClearInteractionGraphicsLayer(CGraphicsLayer* pLayer);

        void                        onFinishTask(const ProtocolVertex& id, CProtocolTask::State status, const QString& msg = QString());

        void                        onQueryGraphicsProxyModel();
        void                        onQueryProjectDataProxyModel(const std::vector<TreeItemType> &dataTypes, const std::vector<DataDimension> &filters);
        void                        onQueryIOInfo(const ProtocolVertex& taskId, int index, bool bInput);

        void                        onProtocolClosed();

        void                        onSearchProtocol(const QString& text);

        void                        onOutputChanged();

        void                        onUpdateTaskGraphicsInput(const GraphicsProcessInputPtr& inputPtr);
        void                        onUpdateProcessInfo(bool bFullEdit, const CProcessInfo& info);

        void                        onNotifyVideoStart(int frameCount);

        void                        onSendProcessAction(const ProtocolTaskPtr& pTask, int flags);

        void                        onStopThread();

        void                        onAllProcessReloaded();
        void                        onProcessReloaded(const QString& name);

    private slots:

        void                        onProtocolLive(int inputIndex, bool bNewSequence);

    signals:

        void                        doSetManager(CProtocolManager* pModel);
        void                        doSetNamesModel(QStringListModel* pModel);
        void                        doSetNamesFromImageModel(QStringListModel* pModel);
        void                        doSetActiveTask(const ProtocolVertex& id);
        void                        doSetElapsedTime(double time);
        void                        doSetMessage(QString msg);
        void                        doSetTaskState(const ProtocolVertex& id, CProtocolTask::State state, const QString& msg=QString());
        void                        doSetGraphicsProxyModel(CProjectGraphicsProxyModel* pModel);
        void                        doSetProjectDataProxyModel(CProjectDataProxyModel* pModel);
        void                        doSetProtocolChangedIcon();
        void                        doSetDescription(const QString& text);
        void                        doSetIOInfo(const CDataInfoPtr& info, const ProtocolVertex& taskId, int index, bool bInput);

        void                        doAddTask(const ProtocolTaskPtr& pTask, const ProtocolVertex& id, const ProtocolVertex& parentId);
        void                        doAddCandidateTask(const ProtocolTaskPtr& pTask, const ProtocolVertex& id);
        void                        doAddConnection(const ProtocolEdge& id, const ProtocolVertex& srcId, size_t srcIndex, const ProtocolVertex& dstId, size_t dstIndex);

        void                        doUpdateCandidateTask();
        void                        doUpdateTaskInfo(const ProtocolTaskPtr& pTask, const CProcessInfo& info);
        void                        doUpdateTaskIOInfo(const ProtocolTaskPtr& taskPtr);
        void                        doUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap);
        void                        doUpdateTaskItemView(const ProtocolTaskPtr& pTask, const ProtocolVertex& id);
        void                        doUpdateTaskState(const ProtocolVertex& id, CProtocolTask::State status, const QString& msg = QString());
        void                        doUpdateImageInfo(const VectorPairString& infoList);

        void                        doProtocolCreated();

        void                        doInputChanged(int index);
        void                        doInputRemoved(int index);
        void                        doInputsCleared();
        void                        doInputAssigned(const ProtocolVertex& id, int portIndex, bool bAssigned);

        void                        doCloseProtocol();

        void                        doNewProtocolNotification(const QString& message, Notification type, CProgressCircle* pItem=nullptr, int duration=Ikomia::_NotifDefaultDuration);

        void                        doAbortProgressBar();

        void                        doDeleteConnection(const ProtocolEdge& id);

        void                        doNotifyProtocolSaved();

        void                        doFinishedProtocol();

        void                        doStopVideo();

    private:

        void                        initGlobalConnections();
        void                        initConnections();
        void                        initProtocol();
        void                        initInputs();

        void                        loadProtocols();

        void                        buildProtocolView();

        bool                        isValidInputItemType(TreeItemType type) const;

        void                        setProtocolInput(size_t inputIndex, bool bNewSequence);

        ProtocolVertex              addProcess(const std::string& name, const ProtocolTaskParamPtr& pParam);

        void                        updateProcessingFPS();
        void                        updateExecTimeInfo(const ProtocolVertex &taskId);
        void                        updateVideoInputIndex(size_t index);

        QString                     timeToString(double time);

        void                        manageWaitThread(bool bNewSequence);
        void                        stopWaitThread();

        void                        clearAllTasks();
        void                        clearFrom(const ProtocolVertex& taskId);
        void                        clearTo(const ProtocolVertex& taskId);
        void                        clearTask(const ProtocolVertex& taskId);

        void                        checkInput(size_t index) const;

        void                        rootConnectionChanged();

    private:

        std::mutex                  m_mutex;
        QStringListModel*           m_pModel = nullptr;
        QStringListModel*           m_pImageModel = nullptr;
        //We ensure protocol name unicity by design
        std::map<QString, int>      m_mapProtocolNameToId;
        std::map<int, QString>      m_mapProtocolIdToName;
        CProtocolRunManager         m_runMgr;
        CProcessManager*            m_pProcessMgr = nullptr;
        CProjectManager*            m_pProjectMgr = nullptr;
        CGraphicsManager*           m_pGraphicsMgr = nullptr;
        CResultManager*             m_pResultsMgr = nullptr;
        CProtocolDBManager          m_dbMgr;
        CProgressBarManager*        m_pProgressMgr = nullptr;
        CMainDataManager*           m_pDataMgr = nullptr;
        CSettingsManager*           m_pSettingsMgr = nullptr;
        CProtocolInputViewManager   m_inputViewMgr;
        ProtocolPtr                 m_pProtocol = nullptr;
        CUser                       m_currentUser;
        QFutureSynchronizer<void>   m_sync;
        CProtocolInputs             m_inputs;
        std::atomic_bool            m_bProtocolRunning{false};
        Utils::CTimer               m_protocolTimer;
        std::list<ProtocolVertex>   m_sequentialRuns;
        CGraphicsLayerInfo          m_interactionLayerInfo;
        size_t                      m_outputVideoId = -1;
        size_t                      m_currentVideoInputIndex = 0;
        VectorPairString            m_execTimeInfo;
        ProtocolInputViewMode       m_inputViewMode = ProtocolInputViewMode::ORIGIN;
        int                         m_currentFPS = 0;
};

#endif // CPROTOCOLMANAGER_H
