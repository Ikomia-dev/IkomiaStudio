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
#include "View/DoubleView/CDataDisplay.h"
#include "Model/Graphics/CGraphicsLayerInfo.hpp"

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
class CMultiImageDisplay;
class CMultiImageModel;

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

        CImageDisplay*          displayImage(size_t index, QImage image, const QString &name);
        CVideoDisplay*          displayVideo(size_t index, QImage image, const QString& name);
        CResultTableDisplay*    displayTable(const QString name, CMeasuresTableModel *pModel, CViewPropertyIO* pViewProperty);
        CResultTableDisplay*    displayTable(const QString name, CFeaturesTableModel *pModel, CViewPropertyIO* pViewProperty);
        CPlotDisplay*           displayPlot(const QString& name, CDataPlot* pPlot);
        CMultiImageDisplay*     displayMultiImage(CMultiImageModel *pModel, const QString &name, CViewPropertyIO *pViewProperty);

        CWidgetDataDisplay*     addWidgetDisplay(size_t index, QWidget* pWidget, bool bDeleteOnClose, CViewPropertyIO* pViewProperty);

        int                     addTabToResults(DisplayType type);
        int                     addTabToResults(DisplayType type, const QString& name, const QIcon& icon);
        void                    addDataViewToTab(DisplayType type, CDataDisplay* pData);
        void                    addDataViewToTab(int indTab, CDataDisplay* pData, int r = 0, int c = 0);

        int                     getTabCount() const;
        int                     getTabIndex(DisplayType type) const;
        QTabWidget*             getTabWidget() const;
        CDataDisplay*           getDataViewFromTabIndex(int indTab, int r, int c) const;
        QList<CDataDisplay*>    getDataViews(DisplayType type) const;
        QList<CDataDisplay*>    getDataViews() const;

        bool                    isEmpty() const;
        bool                    hasTab(DisplayType type) const;

        void                    removeTab(DisplayType type);

        void                    clearAll();

    signals:

        void                    doToggleMaximize();
        void                    doMaximize();

        void                    doSaveCurrentResultImage(int index);
        void                    doExportCurrentResultImage(int index, const QString& path, bool bWithGraphics);

        void                    doSaveCurrentResultVideo(size_t id);
        void                    doExportCurrentResultVideo(size_t id, const QString& path);
        void                    doRecordVideo(size_t id, bool bRecord);

        void                    doSaveCurrentTableData();
        void                    doExportCurrentTableData(const QString& path);

        void                    doBeforeDisplayRemoved(CDataDisplay* pDisplay);

        void                    doDisplaySelected(CDataDisplay* pDisplay);

    public slots:

        void                    onApplyViewProperty();
        void                    onAddGraphicsLayer(const CGraphicsLayerInfo &layerInfo);
        void                    onRemoveGraphicsLayer(const CGraphicsLayerInfo &layerInfo, bool bDelete);
        void                    onDisplayOverlay(const QImage& image, int imageIndex);
        void                    onClearOverlay();

        //Video
        void                    onSetVideoLength(int index, int length);
        void                    onSetVideoPos(int index, int pos);
        void                    onSetVideoFPS(int index, double fps);
        void                    onSetVideoTotalTime(int index, int totalTime);
        void                    onSetVideoCurrentTime(int index, int currentTime);
        void                    onSetStreamOptions(int index, bool bIsStream);
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

        CDataDisplay*           getDataView(DisplayType type, int index) const;
        int                     getDataViewIndex(CDataDisplay* pDisplay) const;

        CDataDisplay*           createDisplay(DisplayType type);
        CImageDisplay*          createImageDisplay();
        CVideoDisplay*          createVideoDisplay();
        CResultTableDisplay*    createTableDisplay(CViewPropertyIO *pViewProperty);

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
