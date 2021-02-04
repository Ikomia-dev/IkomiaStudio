#ifndef CIMGMANAGER_H
#define CIMGMANAGER_H

#include "CImageDataManager.h"

class CImageScene;
class CProgressBarManager;
class CProjectManager;
class CResultManager;
class CRenderManager;
class CGraphicsManager;

using CImageDataMgrPtr = std::shared_ptr<CImageDataManager>;

class CImgManager : public QObject
{
    Q_OBJECT

    public:

        CImgManager();

        void                    setManagers(CProjectManager* pProjectMgr, CGraphicsManager* pGraphicsMgr, CResultManager* pResultMgr,
                                            CRenderManager* pRenderMgr, CProgressBarManager* pProgressMgr);
        void                    setProgressSignalHandler(CProgressSignalHandler* pHandler);

        CImageDataMgrPtr        getImageMgrPtr();
        CMat                    getImage(const QModelIndex& wrapIndex);
        CMat                    getImage(const QModelIndex& datasetWrapIndex, int imgIndex);

        void                    displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence);
        void                    displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence);

        void                    exportImage(const QModelIndex& index, const QString &path, bool bWithGraphics);
        void                    exportImage(const CMat& image, const std::vector<ProxyGraphicsItemPtr> &graphics, const QString &path);

        void                    enableInfoUpdate(bool bEnable);

    public slots:

        void                    onCloseProtocol();

    private:

        void                    displayImageInfo(const CMat& image, const QModelIndex& wrapIndex);

    signals:

        void                    doDisplayImage(int index, CImageScene* pScene, QImage image, QString name, CViewPropertyIO* pViewProp);
        void                    doDisplayImageInfo(const VectorPairString& infoMap);
        void                    doDisplayVolume(CImageScene* pScene, QImage image, QString imgName, bool bNewSequence, CViewPropertyIO* pViewProp);

        void                    doInputDataChanged(const QModelIndex& index, size_t inputIndex, bool bNewSequence);
        void                    doCurrentDataChanged(const QModelIndex& itemIndex, bool bNewSequence);

        void                    doUpdateCurrentImgIndex(size_t currentImgIndex);
        void                    doUpdateNbImg(size_t nbImg);

    private:

        CImageDataMgrPtr        m_pImgMgr = nullptr;
        CProjectManager*        m_pProjectMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CRenderManager*         m_pRenderMgr = nullptr;
        CGraphicsManager*       m_pGraphicsMgr = nullptr;
        CResultManager*         m_pResultMgr = nullptr;
        CProgressSignalHandler* m_pProgressSignal = nullptr;
        CDataInfoPtr            m_pCurrentDataInfo = nullptr;
        bool                    m_bInfoUpdate = false;
};

#endif // CIMGMANAGER_H
