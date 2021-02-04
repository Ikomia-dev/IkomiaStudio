#ifndef CRESULTSMANAGER_H
#define CRESULTSMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include "Core/CProtocol.h"
#include "IO/CMeasureProcessIO.h"
#include "CResultDbManager.h"
#include "CResultItem.hpp"
#include "View/DoubleView/Result/CResultsViewer.h"
#include "CDataPlot.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"

class CProjectModel;
class CProjectManager;
class CProtocolManager;
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

        void                setManagers(CProjectManager* pProjectMgr, CProtocolManager* pProtocolMgr, CGraphicsManager* pGraphicsMgr,
                                        CRenderManager* pRenderMgr, CMainDataManager* pDataMgr, CProgressBarManager* pProgressMgr = nullptr);
        void                setCurrentResult(const QModelIndex& index);
        void                setCurrentOutputImage(const QModelIndex &index);

        QModelIndex         getRootIndex() const;

        void                manageOutputs(const ProtocolTaskPtr& pTask, const ProtocolVertex &taskId, const QModelIndex &itemIndex);

        void                loadImageResults(const QModelIndex& index);

        void                displayResult(const QModelIndex& index);

        void                notifyProjectSaved(int projectIndex);
        void                notifyBeforeProjectClosed(int projectIndex, bool bWithCurrentImage);
        void                notifyBeforeProtocolCleared();
        void                notifyDisplaySelected(int index);

        void                removeResult(const QModelIndex& index);

    signals:

        void                doDisplayOverlay(QImage image, int imageIndex);
        void                doDisplayImage(size_t index, QImage image, const QString& imageName, CViewPropertyIO* pViewProperty);
        void                doDisplayVideo(size_t index, QImage image, const QString& imageName, const std::vector<int>& syncToIndices, CViewPropertyIO* pViewProperty);
        void                doDisplayMeasuresTable(const QString& taskName, CMeasuresTableModel* pModel, CViewPropertyIO* pViewProperty);
        void                doDisplayFeaturesTable(const QString& taskName, CFeaturesTableModel* pModel, CViewPropertyIO* pViewProperty);
        void                doDisplayPlot(const QString& taskName, CDataPlot* pPlot, CViewPropertyIO* pViewProperty);
        void                doDisplayDnnDataset(CMultiImageModel* pModel, const QString& taskName, CViewPropertyIO* pViewProperty);

        void                doNewResultNotification(const QString& message, Notification type, CProgressCircle* pItem=nullptr, int duration=Ikomia::_NotifDefaultDuration);

        void                doAddResultWidget(size_t index, QWidget* pWidget, bool bDeleteOnClose, CViewPropertyIO* pViewProperty);

        void                doHideResultsView();

        void                doClearOverlay();
        void                doClearResultsView();

        void                doInitDisplay(const OutputDisplays& outDisplays);

        void                doApplyViewProperty();

        void                doAddRecordVideo(const QString& path);

        //void                doInitVideoInfo(const QModelIndex& modelIndex, int index);
        void                doSetVideoStream(int index, bool bIsStream);
        void                doSetVideoFPS(int index, int fps);
        void                doSetVideoLength(int index, int frameCount);
        void                doSetVideoPos(int index, int pos);
        void                doSetVideoTotalTime(int index, int time);
        void                doSetVideoCurrentTime(int index, int time);

        void                doStopRecording(size_t id);

        void                doVideoSaveIsFinished(const std::string& path);

    public slots:

        void                onSaveCurrentImage(int index);
        void                onExportCurrentImage(int index, const QString& path, bool bWithGraphics);
        void                onSaveCurrentVideo(size_t index);
        void                onExportCurrentVideo(size_t id, const QString& path);
        void                onSaveCurrentTableData();
        void                onExportCurrentTableData(const QString& path);

        void                onProtocolClosed();

        void                onRecordResultVideo(size_t index, bool bRecord);

    private slots:

        void                onVideoSaveIsFinished(const std::string& path);

    private:

        void                createCustomMeasureTable();
        QModelIndex         createRootResult();

        void                setResultHighlighted(const QModelIndex &index, bool bHighlighted);
        void                setRecordVideoState(size_t id, bool bRecord);

        ResultItemPtr       getResultItem(const QModelIndex& index) const;
        OutputDisplays      getOutputDisplays(const ProtocolTaskPtr& pTask) const;
        DisplayType         getResultViewType(IODataType type) const;
        std::set<IODataType> getImageBasedDataTypes() const;

        bool                isResultFromCurrentImage(const QModelIndex &index) const;
        bool                isParentIndex(const QModelIndex& index, const QModelIndex& parent) const;

        void                clearOverlay();
        void                clearGraphics();
        void                clearPreviousOutputs();

        void                manageImageOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, size_t index, CViewPropertyIO *pViewProp);
        void                manageVolumeOutput(const ProtocolTaskIOPtr& outputPtr, const std::string &taskName, size_t index, CViewPropertyIO* pViewProp);
        void                manageGraphicsOutput(const ProtocolTaskIOPtr& pOutput);
        void                manageBlobOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, CViewPropertyIO* pViewProp);
        void                manageNumericOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, CViewPropertyIO *pViewProp);
        void                manageVideoOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, size_t index, const std::vector<int>& videoInputIndices, CViewPropertyIO *pViewProp);
        void                manageWidgetOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, size_t index, CViewPropertyIO *pViewProp);
        void                manageVideoRecord(size_t index, const CMat& image);
        void                manageDatasetOutput(const ProtocolTaskIOPtr& pOutput, const std::string &taskName, CViewPropertyIO* pViewProp);

        QModelIndex         findResultFromName(const QString& name, QModelIndex startIndex=QModelIndex()) const;

        void                fillResultTreeIds(const QModelIndex& index, std::vector<int>& ids);

        void                runProtocolAndSaveVideo(size_t id, const std::string& path);

        void                updateVolumeRender(const ProtocolTaskIOPtr &outputPtr);

        void                saveOutputImage(int index, const std::string& path, bool bWithGraphics);
        void                saveOutputVideo(size_t id, const std::string& path);
        void                saveOutputMeasures();
        void                saveOutputMeasures(const std::string& path);
        void                saveOutputFeatures(const std::string& path);
        void                saveOutputGraphics();

        void                loadResults(const QModelIndex& index);

        void                startRecordVideo();
        void                stopRecordVideo();

    private:

        bool                    m_bProtocolInProgress = false;
        bool                    m_bImageOverlay = false;
        bool                    m_bBinaryVolume = false;
        int                     m_tmpGraphicsImageIndex = -1;
        int                     m_selectedIndex = -1;
        int                     m_videoPosition = 0;
        size_t                  m_currentOutputCount = 0;
        CProjectManager*        m_pProjectMgr = nullptr;
        CProtocolManager*       m_pProtocolMgr = nullptr;
        CGraphicsManager*       m_pGraphicsMgr = nullptr;
        CRenderManager*         m_pRenderMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CDataVideoBuffer*       m_pVideoMgr = nullptr;
        CMainDataManager*       m_pDataMgr = nullptr;
        QPersistentModelIndex   m_currentInputIndex = QPersistentModelIndex();
        QPersistentModelIndex   m_currentImgIndex = QPersistentModelIndex();
        QPersistentModelIndex   m_currentResultIndex = QPersistentModelIndex();
        QAbstractItemModel*     m_pCurrentTableModel = nullptr;
        CMultiImageModel*       m_pMultiImgModel = nullptr;
        CGraphicsLayerInfo      m_tempGraphicsLayerInfo;
        ProtocolTaskPtr         m_pCurrentTask = nullptr;
        std::string             m_currentVideoRecord = "";
        std::map<size_t,bool>   m_recordVideoMap;
        //Result database ids per project to be removed
        std::unordered_map<int, std::vector<int>>   m_removedResults;
};

#endif // CRESULTSMANAGER_H
