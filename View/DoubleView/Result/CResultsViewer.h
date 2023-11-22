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

#ifndef CRESULTSVIEWER_H
#define CRESULTSVIEWER_H

/**
 * @file      CResultsViewer.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CResultsViewer definition
 *
 * @details   Details
 */
#include <QWidget>
#include <QTabWidget>
#include <QJsonDocument>
#include "View/DoubleView/CDataDisplay.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"
#include "CDataVideoBuffer.h"

#include "IO/CScene3dIO.h"


class CResultTableDisplay;
class CImageViewSync;
class CVideoViewSync;
class CVideoDisplay;
class CWidgetDataDisplay;
class CGraphicsLayer;
class CMeasuresTableModel;
class CFeaturesTableModel;
class CPlotDisplay;
class CDataPlot;
class CJsonDisplay;
class CMultiImageDisplay;
class CMultiImageModel;
class CTextDisplay;
class CScene3dDisplay;

/**
 * @brief
 *
 */
class CResultsViewer : public QWidget
{
    Q_OBJECT

    public:

        CResultsViewer(CImageViewSync *pViewSync, CVideoViewSync* pVideoViewSync, QWidget* parent = Q_NULLPTR);

        void                    initDisplay(const OutputDisplays& displaysProp);

        CImageDisplay*          displayImage(int index, QImage image, const QString &name);
        CVideoDisplay*          displayVideo(int index, QImage image, const QString& name);
        CResultTableDisplay*    displayTable(int index, const QString name, CMeasuresTableModel *pModel, CViewPropertyIO* pViewProperty);
        CResultTableDisplay*    displayTable(int index, const QString name, CFeaturesTableModel *pModel, CViewPropertyIO* pViewProperty);
        CPlotDisplay*           displayPlot(int index, const QString& name, CDataPlot* pPlot, CViewPropertyIO *pViewProperty);
        CTextDisplay*           displayText(int index, const QString& text, const QString& name, CViewPropertyIO *pViewProperty);
        CJsonDisplay*           displayJson(int index, const QJsonDocument &jsonDocument, const QString& name, CViewPropertyIO *pViewProperty);
        CMultiImageDisplay*     displayMultiImage(CMultiImageModel *pModel, const QString &name, CViewPropertyIO *pViewProperty);
        CScene3dDisplay*        displayScene3d(const CScene3d& scene, int index, const QString &name, CViewPropertyIO *pViewProperty);

        CWidgetDataDisplay*     addWidgetDisplay(int index, QWidget* pWidget, bool bDeleteOnClose, CViewPropertyIO* pViewProperty);
        int                     addTabToResults(DisplayType type);
        int                     addTabToResults(DisplayType type, const QString& name, const QIcon& icon);
        void                    addDataViewToTab(DisplayType type, CDataDisplay* pData);
        void                    addDataViewToTab(int indTab, CDataDisplay* pData, int r = 0, int c = 0);
        void                    addGraphicsLayer(const CGraphicsLayerInfo &layerInfo);

        int                     getTabCount() const;
        int                     getTabIndex(DisplayType type) const;
        QTabWidget*             getTabWidget() const;
        CDataDisplay*           getDataViewFromTabIndex(int indTab, int r, int c) const;
        QList<CDataDisplay*>    getDataViews(DisplayType type) const;
        QList<CDataDisplay*>    getDataViews() const;

        bool                    isEmpty() const;
        bool                    hasTab(DisplayType type) const;

        void                    removeTab(DisplayType type);
        void                    removeGraphicsLayer(const CGraphicsLayerInfo &layerInfo, bool bDelete);

        void                    clearAll();

    signals:

        void                    doToggleMaximize();
        void                    doMaximize();

        void                    doSaveCurrentResultImage(int index);
        void                    doExportCurrentResultImage(int index, const QString& path, bool bWithGraphics);
        void                    doExportDatasetImage(const QString& path, CMat& img, CGraphicsLayer* pLayer);

        void                    doSaveCurrentResultVideo(size_t id);
        void                    doExportCurrentResultVideo(size_t id, const QString& path, bool bWithGraphics);
        void                    doRecordVideo(size_t id, bool bRecord);

        void                    doSaveTableData(int index);
        void                    doExportTableData(int index, const QString& path);

        void                    doBeforeDisplayRemoved(CDataDisplay* pDisplay);

        void                    doDisplaySelected(CDataDisplay* pDisplay);

    public slots:

        void                    onApplyViewProperty();
        void                    onDisplayOverlay(const QImage& image, int refImageIndex, DisplayType refImageType);
        void                    onClearOverlay();

        //Video
        void                    onSetVideoLength(int index, size_t length);
        void                    onSetVideoPos(int index, size_t pos);
        void                    onSetVideoFPS(int index, double fps);
        void                    onSetVideoTotalTime(int index, size_t totalTime);
        void                    onSetVideoCurrentTime(int index, size_t currentTime);
        void                    onSetVideoSourceType(int index, CDataVideoBuffer::Type srcType);
        void                    onStopRecordingVideo(int index);

    private slots:

        void                    onDataViewDblClicked(CDataDisplay* pData);
        void                    onDataViewToggleMaximize(CDataDisplay* pData);
        void                    onDataViewMaximize(CDataDisplay* pData);

    private:

        void                    initImageBasedDisplay(DisplayType type, const std::vector<CViewPropertyIO*>& viewProps);
        void                    initConnections(CDataDisplay *pData);
        void                    initImageConnections(CImageDisplay* pDisplay);
        void                    initVideoConnections(CVideoDisplay* pDisplay);
        void                    initTableConnections(CResultTableDisplay* pDisplay);
        void                    initMultiImageConnections(CMultiImageDisplay* pDisplay);

        CDataDisplay*           getDataView(DisplayType type, int index) const;
        int                     getDataViewIndex(CDataDisplay* pDisplay) const;

        CDataDisplay*           createDisplay(DisplayType type);
        CImageDisplay*          createImageDisplay();
        CVideoDisplay*          createVideoDisplay();
        CResultTableDisplay*    createTableDisplay(int index, const QString &name, CViewPropertyIO *pViewProperty);
        CPlotDisplay*           createPlotDisplay(int index, const QString &name, CViewPropertyIO *pViewProperty);
        CJsonDisplay*           createJsonDisplay(int index, const QJsonDocument &jsonDocument, const QString& name, CViewPropertyIO *pViewProperty);
        CTextDisplay*           createTextDisplay(int index, const QString& text, const QString& name, CViewPropertyIO *pViewProperty);

        void                    addDefaultDisplay(int tabIndex);

        void                    decrementTabIndex(int indexFrom);

        void                    syncImageView(CImageDisplay* pDisplay);
        void                    syncVideoView(CVideoDisplay* pDisplay);

        void                    unsyncView(DisplayType type, CDataDisplay* pDisplay);
        void                    unsyncImageView(CImageDisplay* pDisplay);
        void                    unsyncVideoView(CVideoDisplay* pDisplay);

        void                    clearTab(QMap<DisplayType, int>::const_iterator tabIt);
        void                    clearImageSync(CDataDisplay* pDataDisplay);
        void                    clearVideoSync(CDataDisplay* pDataDisplay);

    private:

        QVBoxLayout*            m_pLayout = nullptr;
        QTabWidget*             m_pTabWidget = nullptr;
        QMap<DisplayType, int>  m_mapTypeIndex;
        CImageViewSync*         m_pImageViewSync = nullptr;
        CVideoViewSync*         m_pVideoViewSync = nullptr;
};

#endif // CRESULTSVIEWER_H
