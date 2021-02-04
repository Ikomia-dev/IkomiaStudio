#ifndef CMAINDATAMANAGER_H
#define CMAINDATAMANAGER_H

#include <QObject>
#include "Image/CImgManager.h"
#include "Video/CVideoManager.h"
#include "Main/AppDefine.hpp"

class CProgressSignalHandler;
class CImageScene;

class CMainDataManager : public QObject
{
    Q_OBJECT

    public:

        CMainDataManager();
        ~CMainDataManager();

        CImgManager*    getImgMgr();
        CVideoManager*  getVideoMgr();
        QString         getDataPath(const QModelIndex& wrapIndex);
        CDataInfoPtr    getDataInfoPtr(const QModelIndex& wrapIndex);
        int             getSelectedDisplayCategory() const;
        int             getSelectedDisplayIndex() const;

        void            setManagers(CProjectManager* pProjectMgr, CProtocolManager* pProtocolMgr, CGraphicsManager* pGraphicsMgr,
                                    CResultManager *pResultMgr, CRenderManager* pRenderMgr, CProgressBarManager* pProgressMgr);
        void            setProgressSignalHandler(CProgressSignalHandler* pHandler);

        void            closeData(const QModelIndex &index);

        void            displaySimpleImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, size_t inputIndex, bool bNewSequence);
        void            displayVolumeImage(CImageScene* pScene, const QModelIndex& index, const QModelIndex& wrapIndex, bool bNewSequence);
        void            displayVideoImage(const QModelIndex& index, size_t inputIndex, bool bNewSequence);
        void            displayImageSequence(const QModelIndex& index, size_t inputIndex, bool bNewSequence);

        void            beforeProjectClose(int projectIndex, bool bWithCurrentImage);

        void            reloadCurrent();

    public slots:

        void            onSetSelectedDisplay(DisplayCategory category, int index);

        void            onSaveCurrentVideoFrame(const QModelIndex &modelIndex, int index);

        void            onExportCurrentImage(int index, const QString &path, bool bWithGraphics);
        void            onExportCurrentVideoFrame(const QModelIndex &index, int inputIndex, const QString &path, bool bWithGraphics);

        void            onEnableInfo(bool bEnable);

        void            onPlayVideo(int index);

        void            onStopVideo(const QModelIndex &index);

    private:

        CImgManager         m_imgMgr;
        CVideoManager       m_videoMgr;
        CProjectManager*    m_pProjectMgr = nullptr;
        CProtocolManager*   m_pProtocolMgr = nullptr;
        int                 m_selectedDisplayCategory = DisplayCategory::INPUT;
        int                 m_selectedDisplayIndex = 0;
};

#endif // CMAINDATAMANAGER_H
