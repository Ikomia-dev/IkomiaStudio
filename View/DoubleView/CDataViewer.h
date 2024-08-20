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

#ifndef CDATAVIEWER_H
#define CDATAVIEWER_H

/**
 * @file      CDataViewer.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataViewer definition
 *
 * @details   Details
 */

#include <QWidget>
#include "Main/forwards.hpp"
#include "CDataDisplay.h"
#include "3D/CPositionDisplay.h"
#include "3D/CVolumeDisplay.h"
#include "Image/CImageDisplay.h"
#include "Video/CVideoDisplay.h"
#include "CPathNavigator.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"

class CStaticDisplay;
class CImageViewSync;
class CVideoViewSync;
class CVolumeViewSync;

/**
 * @brief
 *
 */
class CDataViewer : public QWidget
{
    Q_OBJECT

    public:

        CDataViewer(QWidget* parent = Q_NULLPTR);

        CDataDisplay*           getDataDisplay();
        CDataListView*          getDataListView();

        bool                    isVideoStream() const;

        void                    setImageViewSync(CImageViewSync* pSync);
        void                    setVideoViewSync(CVideoViewSync* pSync);
        void                    setVolumeViewSync(CVolumeViewSync* pSync);

        void                    fillZoomProperties(CViewPropertyIO* pViewProp);

        void                    addDataView(CDataDisplay* pData, int r, int c);
        void                    addGraphicsLayer(const CGraphicsLayerInfo& layerInfo);

        void                    removeGraphicsLayer(const CGraphicsLayerInfo &layerInfo, bool bDelete);

        void                    updateDataListViewIndex(const QModelIndex& index);
        void                    updateDataListViewModel(QAbstractItemModel* pModel);
        void                    updateVolumeImage(int index, QImage image, const QString &name, CViewPropertyIO *pViewProp);

        void                    initDisplay(const std::map<DisplayType, int>& mapTypeCount);

        void                    displayListView();
        void                    displayImage(int index, CImageScene* pScene, QImage image, QString name, CViewPropertyIO *pViewProp);
        void                    displayVideo(const QModelIndex &modelIndex, int displayIndex, CImageScene *pScene, QImage image, QString name, bool bVideoChanged, CViewPropertyIO *pViewProp);
        void                    displayVolume(CImageScene *pScene, QImage image, QString name, bool bStackHasChanged, CViewPropertyIO *pViewProp);
        void                    displayPosition(CImageScene *pScene, QImage image, QString name, bool bStackHasChanged, CViewPropertyIO *pViewProp);

        void                    switchView();

        void                    notifySceneDeleted();
        void                    notifyResultViewVisible(bool bVisible);

        void                    syncImageView(CImageDisplay* pDisplay);
        void                    syncVideoView(CVideoDisplay* pDisplay);
        void                    syncVideoView(CVideoDisplay* pDisplay, const std::vector<int>& syncToIndices);

        void                    unsyncView(CDataDisplay* pDisplay);

        void                    clearAll();

    signals:

        void                    doUpdateIndex(const QModelIndex& index);
        void                    doUpdateImageIndex(int index);

        void                    doDisplayImage(const QModelIndex& index);
        void                    doDisplayListView();
        void                    doDisplaySelected(CDataDisplay* pDisplay);

        void                    doToggleMaximize();

        void                    doExportCurrentImage(int index, const QString& path, bool bWithGraphics);

        //Graphics signals from CImageView
        void                    doChangeGraphicsActivationState(bool bActivated);
        void                    doAddGraphicsLayer(CGraphicsLayer* pLayer);
        void                    doGraphicsChanged();
        void                    doGraphicsRemoved(const QSet<CGraphicsLayer *> &layers);
        void                    doToggleGraphicsToolbar();

        //Video signals from CVideoDisplay
        void                    doPlayVideo(int index);
        void                    doStopVideo(const QModelIndex& modelIndex);
        void                    doUpdateVideoPos(const QModelIndex& modelIndex, int index, size_t pos);
        void                    doRecordVideo(const QModelIndex& modelIndex, bool bRecord);
        void                    doNotifyVideoStart(const QModelIndex& modelIndex);
        void                    doSaveCurrentVideoFrame(const QModelIndex& modelIndex, int index);
        void                    doExportCurrentVideoFrame(const QModelIndex& modelIndex, int index, const QString& path, bool bWithGraphics);

        //3D signals from C3dDisplay
        void                    doInitCL(const std::string& name);
        void                    doInitPBO(GLuint pbo, GLuint pbo_depth);
        void                    doRenderVolume(QMatrix4x4& model, QMatrix4x4& view, QMatrix4x4& projection);
        void                    doUpdateWindowSize(int width, int height);
        void                    doUpdateRenderMode(int mode);
        void                    doUpdateColormap(int colormap);
        void                    doUpdateParam(int type, float value);
        void                    doSendErrorMessage(const QString& msg);

    public slots:

        void                    onDisplayVolume(const QModelIndex& index);
        void                    onDisplayBinaryOverlay(const QImage& image);

        void                    onClearBinaryOverlay();

        void                    onSwitchView();

        void                    onUpdateIndex(const QModelIndex& index);
        void                    onUpdateNbImage(int nb);
        void                    onUpdateCurrentImageIndex(int index);
        void                    onUpdateRenderVolume();
        void                    onEnableRender(bool bEnable);

        //Graphics
        void                    onChangeGraphicsActivationState(bool bActivate);
        void                    onSetCurrentGraphicsLayer(CGraphicsLayer* pLayer);
        void                    onAddGraphicsItem(QGraphicsItem* pItem, bool bForceParent);

        //Video
        void                    onSetVideoSliderLength(int index, size_t length);
        void                    onSetVideoSliderPos(int index, size_t pos);
        void                    onSetVideoFPS(int index, double fps);
        void                    onSetVideoTotalTime(int index, size_t totalTime);
        void                    onSetVideoCurrentTime(int index, size_t currentTime);
        void                    onSetVideoSourceType(int index, CDataVideoBuffer::Type srcType);

        void                    onStopVideo();
        void                    onStopRecordingVideo(const QModelIndex &modelIndex);
        void                    onStopVideoPlayer(const QModelIndex &modelIndex);

        void                    onApplyViewProperty();

    private slots:

        void                    onDataViewToggleMaximize(CDataDisplay* pData);

    private:

        CStaticDisplay*         createEmptyDisplay();
        CImageDisplay*          createImageDisplay();
        CVideoDisplay*          createVideoDisplay();
        CVolumeDisplay*         createVolumeDisplay();
        CPositionDisplay*       createPositionDisplay();

        void                    initConnections();
        void                    initDisplayConnections(CDataDisplay* pDisplay);
        void                    initImageConnections(CImageDisplay* pDisplay);
        void                    initImageViewConnections(CImageView* pView);
        void                    initVideoConnections(CVideoDisplay* pDisplay);
        void                    initVolumeConnections(CVolumeDisplay* pDisplay);
        void                    init3dDisplayConnections(C3dDisplay* pDisplay);
        void                    initPositionConnections(CPositionDisplay *pDisplay);

        void                    setActiveWidget(int index);

        QList<CImageDisplay*>   getImageDisplays() const;
        QList<CImageDisplay*>   getAllImageDisplays() const;
        CImageDisplay*          getImageDisplay(CDataDisplay* pDisplay) const;
        QList<CImageView*>      getImageViews() const;
        CImageView*             getImageView(CDataDisplay* pDisplay) const;
        QList<CVideoDisplay*>   getVideoDisplays() const;
        QList<CVolumeDisplay*>  getVolumeDisplays() const;
        QList<CPositionDisplay*> getPositionDisplays() const;
        int                     getDisplayIndex(CDataDisplay* pDisplay) const;

        bool                    isItemShownInListView(const QModelIndex& index);

        bool                    canPlayVideo(CVideoDisplay* pDisplay) const;

        void                    unsyncImageView(CImageDisplay* pDisplay);
        void                    unsyncVideoView(CVideoDisplay* pDisplay);

    private:

        CDataDisplay*           m_pDataDisplay = nullptr;
        QVBoxLayout*            m_pLayout = nullptr;
        CDataListView*          m_pDataListView = nullptr;
        QStackedWidget*         m_pStacked = nullptr;
        CPathNavigator*         m_pPathNavigator = nullptr;
        CImageViewSync*         m_pImageViewSync = nullptr;
        CVideoViewSync*         m_pVideoViewSync = nullptr;
        CVolumeViewSync*        m_pVolumeViewSync = nullptr;
        QMap<QPersistentModelIndex, int>  m_modelIndexToDisplayIndex;
        QMap<int, QPersistentModelIndex>  m_displayIndexToModelIndex;
};

#endif // CDATAVIEWER_H
