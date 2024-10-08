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

#ifndef CDOUBLEVIEW_H
#define CDOUBLEVIEW_H

/**
 * @file      CDoubleView.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDoubleView definition
 *
 * @details   Details
 */
#include <QWidget>
#include <QTabWidget>
#include <QJsonDocument>
#include "Main/forwards.hpp"
#include "CDataViewer.h"
#include "Result/CResultsViewer.h"
#include "Image/CImageViewSync.h"
#include "Video/CVideoViewSync.h"
#include "3D/CVolumeViewSync.h"
#include "Model/Results/CResultManager.h"

#include "IO/CScene3dIO.h"


/**
 * @brief
 *
 */
class CDoubleView : public QWidget
{
    Q_OBJECT

    public:

        CDoubleView(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            insertIntoSplitter(QSplitter* pSplitter);

        int             addTabToResults(DisplayType type, const QString& name, QIcon icon = QIcon());
        void            addDataToDataViewer(CDataDisplay* pData, int r = 0, int c = 0);

        CDataViewer*    getDataViewer() const;
        CResultsViewer* getResultsViewer() const;

        void            updateMainSplitter();

    signals:

        void            doUpdateIndex(const QModelIndex& index);
        void            doSetSelectedDisplay(DisplayCategory category, int index);
        void            doToggleGraphicsToolbar();
        void            doHideGraphicsToolbar();

    public slots:

        void            onInitDisplay(const std::map<DisplayType, int>& mapTypeCount);
        void            onInitResultDisplay(const OutputDisplays& displaysProp);

        void            onUpdateDataListviewModel(QAbstractItemModel* pModel);
        void            onUpdateDataViewerIndex(const QModelIndex& index);
        void            onUpdateVolumeImage(int index, QImage image, QString name, CViewPropertyIO *pViewProperty);

        void            onDisplayImage(int index, CImageScene *pScene, QImage image, QString name, CViewPropertyIO *pViewProperty);
        void            onDisplayVideo(const QModelIndex &modelIndex, int displayIndex, CImageScene *pScene, QImage image, QString name, bool bVideoChanged, CViewPropertyIO *pViewProperty);
        void            onDisplayVolume(CImageScene *pScene, QImage image, QString imgName, bool bStackHasChanged, CViewPropertyIO *pViewProperty);
        void            onDisplayPosition(CImageScene *pScene, QImage image, QString imgName, bool bStackHasChanged, CViewPropertyIO *pViewProperty);

        void            onDisplayResultImage(int index, QImage image, const QString& imageName, CViewPropertyIO *pViewProperty);
        void            onDisplayResultMeasuresTable(int index, const QString taskName, CMeasuresTableModel *pModel, CViewPropertyIO *pViewProperty);
        void            onDisplayResultFeaturesTable(int index, const QString taskName, CFeaturesTableModel *pModel, CViewPropertyIO *pViewProperty);
        void            onDisplayResultPlot(int index, const QString& taskName, CDataPlot* pPlot, CViewPropertyIO *pViewProperty);
        void            onDisplayMultiImage(CMultiImageModel* pModel, const QString& taskName, CViewPropertyIO* pViewProperty);
        void            onDisplayResultVideo(int index, QImage image, const QString& imageName, const std::vector<int> &syncToIndices, CViewPropertyIO *pViewProperty);
        void            onDisplayResultText(int index, const QString& text, const QString& taskName, CViewPropertyIO* pViewProperty);
        void            onDisplayResultJson(int index, const QJsonDocument &jsonDocument, const QString& taskName, CViewPropertyIO* pViewProperty);
        void            onDisplayResultScene3d(const CScene3d& scene, int index, const QString& taskName, CViewPropertyIO* pViewProperty);

        void            onAddResultWidget(int index, QWidget *pWidget, bool bDeleteOnClose, CViewPropertyIO *pViewProperty);
        void            onAddGraphicsLayer(const CGraphicsLayerInfo& layerInfo);

        void            onRemoveGraphicsLayer(const CGraphicsLayerInfo& layerInfo, bool bDelete);

        void            onUpdateIndex(const QModelIndex& index);

        void            onImageCheck(CDataDisplay* pData);
        void            onImageUncheck(CDataDisplay* pData);
        void            onImageDeleted();

        void            onClearResults();

        void            onCloseProject(bool bCurrentImgClosed);

        void            onInputDataChanged(const QModelIndex& index, bool bNewSequence);
        void            onInputDisplaySelected(CDataDisplay* pDisplay);

        void            onOutputDisplaySelected(CDataDisplay* pDisplay);

        void            onSwitchView();

        void            onHideResultsView();

        void            onBeforeResultDisplayRemoved(CDataDisplay* pDisplay);

        void            onStopVideo();
        void            onStopVideoPlayer(const QModelIndex& index);

    private slots:

        void            onShowHideDataViewer();
        void            onShowHideResultsViewer();
        void            onHideDataView();

    private:

        void            initConnections();

        CVideoDisplay*  createVideoDisplay(CViewPropertyIO* pViewProperty, size_t id);

        void            applyViewModeProperty(CViewPropertyIO* pViewProp);

    private:

        CDataViewer*    m_pDataViewer = nullptr;
        CResultsViewer* m_pResultsViewer = nullptr;
        CImageViewSync  m_imageViewSync;
        CVideoViewSync  m_videoViewSync;
        CVolumeViewSync m_volumeViewSync;
        QSplitter*      m_pSplitter = nullptr;
        bool            m_bShowResults = true;
};

#endif // CDOUBLEVIEW_H
