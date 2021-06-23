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

#ifndef CWORKFLOWRUNMANAGER_H
#define CWORKFLOWRUNMANAGER_H

#include "Core/CWorkflow.h"
#include "CWorkflowInput.h"

class CProjectManager;
class CMainDataManager;
class CProgressBarManager;

class CWorkflowRunManager : public QObject
{
    Q_OBJECT

    public:

        CWorkflowRunManager(CWorkflowInputs* pInputs);
        ~CWorkflowRunManager();

        void                    setManagers(CProjectManager *pProjectMgr, CMainDataManager *pDataMgr, CProgressBarManager* pProgressMgr);
        void                    setWorkflow(WorkflowPtr WorkflowPtr);

        double                  getTotalElapsedTime() const;
        std::set<IODataType>    getTargetDataTypes(size_t inputIndex) const;
        std::set<IODataType>    getOriginTargetDataTypes(size_t inputIndex) const;

        bool                    isRunning() const;

        WorkflowTaskIOPtr       createTaskIO(size_t inputIndex, size_t dataIndex, bool bNewSequence);

        void                    run();
        void                    runLive(size_t inputIndex);
        void                    runFromActiveTask();
        void                    runToActiveTask();
        void                    runSequentialTask(const WorkflowVertex& taskId);

        void                    addSequentialRun(const WorkflowVertex &taskId);

        void                    notifyGraphicsChanged();

        void                    stop();
        void                    stopWaitThread();

        void                    manageWaitThread(bool bNewSequence);

        void                    waitForWorkflow();

        void                    protocolErrorHandling(const std::exception& e);
        void                    batchErrorHandling(const std::exception& e);

    signals:

        void                    doSetElapsedTime(double time);
        void                    doAbortProgressBar();
        void                    doWorkflowLive(int inputIndex, bool bNewSequence);
        void                    doWorkflowFinished();

    public slots:

        void                    onSetElapsedTime(double time);

    private slots:

        void                    onSequentialRunFinished();
        void                    onWorkflowFinished();

    private:

        void                    setBatchInput(int index);

        size_t                  getBatchCount() const;

        bool                    checkInputs(std::string &err) const;
        bool                    checkInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkLiveInputs() const;
        bool                    checkImageInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkImageVideoInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkImageDatasetInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkImageFolderInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkVideoInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkVideoDatasetInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkVideoFolderInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkDatasetInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkDatasetFolderInputs(size_t index1, size_t index2, std::string &err) const;
        bool                    checkFolderInputs(size_t index1, size_t index2, std::string &err) const;

        WorkflowTaskIOPtr       createIOFromDataItem(const QModelIndex& index, bool bNewSequence);

        void                    runBatch();
        void                    runFromBatch();
        void                    runToBatch();
        void                    runSingle();
        void                    runFromSingle();
        void                    runToSingle();

    private:

        WorkflowPtr                 m_workflowPtr = nullptr;
        CProjectManager*            m_pProjectMgr = nullptr;
        CMainDataManager*           m_pDataMgr = nullptr;
        CProgressBarManager*        m_pProgressMgr = nullptr;

        CWorkflowInputs*            m_pInputs = nullptr;

        std::atomic_bool            m_bRunning{false};
        std::atomic_bool            m_bStopThread{false};
        std::atomic_bool            m_bStop{false};
        std::list<WorkflowVertex>   m_sequentialRuns;

        std::mutex                  m_mutex;
        QFutureSynchronizer<void>   m_sync;
        QFutureWatcher<void>        m_processWatcher;
        QFutureWatcher<void>        m_waitThreadWatcher;
        QFutureWatcher<void>*       m_pSequentialRunWatcher = nullptr;
        QFutureWatcher<void>        m_liveWatcher;
        std::condition_variable     m_threadCond;

        size_t                      m_liveInputIndex = 0;
        size_t                      m_batchIndex = 0;
        size_t                      m_batchCount = 0;
        double                      m_totalElapsedTime = 0;
};

#endif // CWORKFLOWRUNMANAGER_H
