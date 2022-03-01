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

#ifndef CRESULTSMANAGER_H
#define CRESULTSMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "Core/CWorkflow.h"
#include "IO/CBlobMeasureIO.h"
#include "CResultDbManager.h"
#include "CResultItem.hpp"
#include "View/DoubleView/Result/CResultsViewer.h"
#include "CDataPlot.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"

class CProjectModel;
class CProjectManager;
class CWorkflowManager;
class CGraphicsManager;
class CGraphicsLayer;
class CRenderManager;
class CProgressBarManager;
class CMainDataManager;
class CFeaturesTableModel;
class CMultiImageModel;
class CProgressCircle;


class CResultManager : public QObject
{
    Q_OBJECT

    public:

        CResultManager();
        ~CResultManager();

        void                init();

        void                setManagers(CProjectManager* pProjectMgr, CWorkflowManager* pWorkflowMgr, CGraphicsManager* pGraphicsMgr,
                                        CRenderManager* pRenderMgr, CMainDataManager* pDataMgr, CProgressBarManager* pProgressMgr = nullptr);
        void                setCurrentResult(const QModelIndex& index);
        void                setCurrentOutputImage(const QModelIndex &index);

        QModelIndex         getRootIndex() const;

        void                manageOutputs(const WorkflowTaskPtr& taskPtr, const WorkflowVertex &taskId, const QModelIndex &itemIndex);

        void                loadImageResults(const QModelIndex& index);

        void                displayResult(const QModelIndex& index);

        void                notifyProjectSaved(int projectIndex);
        void                notifyBeforeProjectClosed(int projectIndex, bool bWithCurrentImage);
        void                notifyBeforeWorkflowCleared();
        void                notifyDisplaySelected(int index);

        void                removeResult(const QModelIndex& index);

    signals:

        void                doDisplayOverlay(QImage image, int imageIndex);
        void                doDisplayImage(int index, QImage image, const QString& imageName, CViewPropertyIO* pViewProperty);
        void                doDisplayVideo(int index, QImage image, const QString& imageName, const std::vector<int>& syncToIndices, CViewPropertyIO* pViewProperty);
        void                doDisplayMeasuresTable(int index, const QString& taskName, CMeasuresTableModel* pModel, CViewPropertyIO* pViewProperty);
        void                doDisplayFeaturesTable(int index, const QString& taskName, CFeaturesTableModel* pModel, CViewPropertyIO* pViewProperty);
        void                doDisplayPlot(const QString& taskName, CDataPlot* pPlot, CViewPropertyIO* pViewProperty);
        void                doDisplayDnnDataset(CMultiImageModel* pModel, const QString& taskName, CViewPropertyIO* pViewProperty);

        void                doNewResultNotification(const QString& message, Notification type, CProgressCircle* pItem=nullptr, int duration=Ikomia::_NotifDefaultDuration);

        void                doAddResultWidget(int index, QWidget* pWidget, bool bDeleteOnClose, CViewPropertyIO* pViewProperty);

        void                doHideResultsView();

        void                doClearOverlay();
        void                doClearResultsView();

        void                doInitDisplay(const OutputDisplays& outDisplays);

        void                doApplyViewProperty();

        void                doAddRecordVideo(const QString& path);

        void                doSetVideoSourceType(int index, CDataVideoBuffer::Type type);
        void                doSetVideoFPS(int index, int fps);
        void                doSetVideoLength(int index, int frameCount);
        void                doSetVideoPos(int index, int pos);
        void                doSetVideoTotalTime(int index, int time);
        void                doSetVideoCurrentTime(int index, int time);

        void                doStopRecording(size_t id);

        void                doVideoSaveIsFinished(QStringList& paths, CDataVideoBuffer::Type sourceType);

    public slots:

        void                onSaveCurrentImage(int index);
        void                onExportCurrentImage(int index, const QString& path, bool bWithGraphics);
        void                onSaveCurrentVideo(size_t index);
        void                onExportCurrentVideo(size_t id, const QString& path, bool bWithGraphics);
        void                onSaveTableData(int index);
        void                onExportTableData(int index, const QString& path);
        void                onExportDatasetImage(const QString& path, CMat &img, CGraphicsLayer* pLayer);

        void                onWorkflowClosed();

        void                onRecordResultVideo(size_t index, bool bRecord);

    private slots:

        void                onVideoSaveIsFinished(QStringList &paths, CDataVideoBuffer::Type sourceType);

    private:

        void                createCustomMeasureTable();
        QModelIndex         createRootResult();

        void                setResultHighlighted(const QModelIndex &index, bool bHighlighted);
        void                setRecordVideoState(size_t id, bool bRecord);

        ResultItemPtr               getResultItem(const QModelIndex& index) const;
        OutputDisplays              getOutputDisplays(const WorkflowTaskPtr& pTask) const;
        DisplayType                 getResultViewType(IODataType type) const;
        std::set<IODataType>        getImageBasedDataTypes() const;
        CViewPropertyIO::ViewMode   getViewMode(const WorkflowTaskPtr& taskPtr);

        bool                isResultFromCurrentImage(const QModelIndex &index) const;
        bool                isParentIndex(const QModelIndex& index, const QModelIndex& parent) const;

        void                clearOverlay();
        void                clearGraphics();
        void                clearPreviousOutputs();
        void                clearTableModels();

        void                manageImageOutput(const WorkflowTaskIOPtr& pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp);
        void                manageVolumeOutput(const WorkflowTaskIOPtr& outputPtr, const std::string &taskName, int index, CViewPropertyIO* pViewProp);
        void                manageGraphicsOutput(const WorkflowTaskIOPtr& pOutput);
        void                manageBlobOutput(const WorkflowTaskIOPtr& pOutput, const std::string &taskName, int index, CViewPropertyIO* pViewProp);
        void                manageNumericOutput(const WorkflowTaskIOPtr& pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp);
        void                manageVideoOutput(const WorkflowTaskPtr &taskPtr, const WorkflowTaskIOPtr& pOutput, int index, const std::vector<int>& videoInputIndices, CViewPropertyIO *pViewProp);
        void                manageWidgetOutput(const WorkflowTaskIOPtr& pOutput, const std::string &taskName, int index, CViewPropertyIO *pViewProp);
        void                manageVideoRecord(const WorkflowTaskPtr &taskPtr, size_t index, const CMat& image);
        void                manageDatasetOutput(const WorkflowTaskIOPtr& pOutput, const std::string &taskName, CViewPropertyIO* pViewProp);

        QModelIndex         findResultFromName(const QString& name, QModelIndex startIndex=QModelIndex()) const;

        void                fillResultTreeIds(const QModelIndex& index, std::vector<int>& ids);

        void                runWorkflowAndSaveVideo(size_t id, const std::string& path, bool bWithGraphics);

        void                updateVolumeRender(const WorkflowTaskIOPtr &outputPtr);

        void                saveOutputImage(int index, const std::string& path, bool bWithGraphics);
        void                saveOutputVideo(size_t id, const std::string& path);
        void                saveOutputMeasures(int index);
        void                saveOutputMeasures(int index, const std::string& path);
        void                saveOutputFeatures(int index, const std::string& path);
        void                saveOutputGraphics();

        void                loadResults(const QModelIndex& index);

        void                startRecordVideo();
        void                stopRecordVideo();

    private:

        bool                                m_bWorkflowInProgress = false;
        bool                                m_bImageOverlay = false;
        bool                                m_bBinaryVolume = false;
        int                                 m_tmpGraphicsImageIndex = -1;
        int                                 m_selectedIndex = -1;
        int                                 m_videoPosition = 0;
        size_t                              m_currentOutputCount = 0;
        CProjectManager*                    m_pProjectMgr = nullptr;
        CWorkflowManager*                   m_pWorkflowMgr = nullptr;
        CGraphicsManager*                   m_pGraphicsMgr = nullptr;
        CRenderManager*                     m_pRenderMgr = nullptr;
        CProgressBarManager*                m_pProgressMgr = nullptr;
        CDataVideoBuffer*                   m_pVideoMgr = nullptr;
        CMainDataManager*                   m_pDataMgr = nullptr;
        QPersistentModelIndex               m_currentInputIndex = QPersistentModelIndex();
        QPersistentModelIndex               m_currentImgIndex = QPersistentModelIndex();
        QPersistentModelIndex               m_currentResultIndex = QPersistentModelIndex();
        std::vector<QAbstractItemModel*>    m_tableModels;
        CMultiImageModel*                   m_pMultiImgModel = nullptr;
        CGraphicsLayerInfo                  m_tempGraphicsLayerInfo;
        WorkflowTaskPtr                     m_pCurrentTask = nullptr;
        std::string                         m_currentVideoRecord = "";
        std::map<size_t,bool>               m_recordVideoMap;
        CViewPropertyIO::ViewMode           m_currentViewMode = CViewPropertyIO::ViewMode::GUI_DRIVEN;
        //Result database ids per project to be removed
        std::unordered_map<int, std::vector<int>>   m_removedResults;
};

#endif // CRESULTSMANAGER_H
