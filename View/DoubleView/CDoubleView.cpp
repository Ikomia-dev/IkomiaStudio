// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @file      CDoubleView.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDoubleView
 *
 * @details   Details
 */

#include "CDoubleView.h"
#include <QApplication>
#include <QSplitter>
#include <memory>
#include <QJsonDocument>
#include "CDataListView.h"
#include "Image/CImageDisplay.h"
#include "Result/CResultTableDisplay.h"
#include "3D/CVolumeDisplay.h"
#include "3D/C3dDisplay.h"
#include "Plot/CPlotDisplay.h"
#include "CWidgetDataDisplay.h"

CDoubleView::CDoubleView(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    // Left view
    m_pDataViewer = new CDataViewer(this);
    m_pDataViewer->setImageViewSync(&m_imageViewSync);
    m_pDataViewer->setVideoViewSync(&m_videoViewSync);
    m_pDataViewer->setVolumeViewSync(&m_volumeViewSync);

    // Right view
    m_pResultsViewer = new CResultsViewer(&m_imageViewSync, &m_videoViewSync, this);
    m_pResultsViewer->addTabToResults(DisplayType::EMPTY_DISPLAY);
    m_pResultsViewer->hide(); //Hidden by default

    initConnections();
}

void CDoubleView::initConnections()
{
    //Data viewer -> double view
    connect(m_pDataViewer, &CDataViewer::doUpdateIndex, this, &CDoubleView::onUpdateIndex);
    connect(m_pDataViewer, &CDataViewer::doToggleMaximize, this, &CDoubleView::onShowHideResultsViewer);
    connect(m_pDataViewer, &CDataViewer::doDisplaySelected, this, &CDoubleView::onInputDisplaySelected);
    connect(m_pDataViewer, &CDataViewer::doToggleGraphicsToolbar, [&]{ emit doToggleGraphicsToolbar(); });

    //Result viewer -> double view
    connect(m_pResultsViewer, &CResultsViewer::doToggleMaximize, this, &CDoubleView::onShowHideDataViewer);
    connect(m_pResultsViewer, &CResultsViewer::doMaximize, this, &CDoubleView::onHideDataView);
    connect(m_pResultsViewer, &CResultsViewer::doBeforeDisplayRemoved, this, &CDoubleView::onBeforeResultDisplayRemoved);
    connect(m_pResultsViewer, &CResultsViewer::doDisplaySelected, this, &CDoubleView::onOutputDisplaySelected);

    //Data viewer -> result viewer
    connect(m_pDataViewer, &CDataViewer::doDisplayListView, m_pResultsViewer, &CResultsViewer::hide);
}

void CDoubleView::applyViewModeProperty(CViewPropertyIO *pViewProp)
{
    if(pViewProp == nullptr)
        return;

    switch(pViewProp->getViewMode())
    {
        case CViewPropertyIO::ViewMode::GUI_DRIVEN:
            break;
        case CViewPropertyIO::ViewMode::INPUT_OUTPUT:
            m_pDataViewer->show();
            m_pResultsViewer->show();
            break;
        case CViewPropertyIO::ViewMode::INPUT_ONLY:
            m_pDataViewer->show();
            m_pResultsViewer->hide();
            break;
        case CViewPropertyIO::ViewMode::OUTPUT_ONLY:
            m_pDataViewer->hide();
            m_pResultsViewer->show();
            break;
    }
}

CDataViewer *CDoubleView::getDataViewer() const
{
    return m_pDataViewer;
}

CResultsViewer *CDoubleView::getResultsViewer() const
{
    return m_pResultsViewer;
}

void CDoubleView::updateMainSplitter()
{
    m_pSplitter->setSizes(QList<int>({INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX}));
}

void CDoubleView::insertIntoSplitter(QSplitter* pSplitter)
{
    pSplitter->addWidget(m_pDataViewer);
    pSplitter->addWidget(m_pResultsViewer);
    m_pSplitter = pSplitter;
}

int CDoubleView::addTabToResults(DisplayType type, const QString& name, QIcon icon)
{
    return m_pResultsViewer->addTabToResults(type, name, icon);
}

void CDoubleView::addDataToDataViewer(CDataDisplay* pData, int r, int c)
{
    assert(pData != nullptr);
    assert(m_pDataViewer != nullptr && m_pDataViewer->getDataDisplay() != nullptr);

    // Add data at (0,0) only
    m_pDataViewer->addDataView(pData, r, c);
}

void CDoubleView::onInitDisplay(const std::map<DisplayType, int> &mapTypeCount)
{
    m_pDataViewer->initDisplay(mapTypeCount);
}

void CDoubleView::onInitResultDisplay(const OutputDisplays& displaysProp)
{
    //Current show status
    bool bResultVisible = m_pResultsViewer->isVisible();
    // Force show resultViewer
    m_pResultsViewer->show();
    // Force update mainsplitter in order to show resultViewer => child widgets are well resized
    updateMainSplitter();
    //Init display according to output types
    m_pResultsViewer->initDisplay(displaysProp);

    // Notify data viewer that the result viewer is visible
    if(bResultVisible == false)
        m_pDataViewer->notifyResultViewVisible(true);
}

void CDoubleView::onUpdateDataListviewModel(QAbstractItemModel* pModel)
{
    m_pDataViewer->updateDataListViewModel(pModel);
}

void CDoubleView::onUpdateDataViewerIndex(const QModelIndex& index)
{
    m_pDataViewer->updateDataListViewIndex(index);
}

void CDoubleView::onUpdateVolumeImage(int index, QImage image, QString name, CViewPropertyIO *pViewProperty)
{
    m_pDataViewer->fillZoomProperties(pViewProperty);
    m_pDataViewer->updateVolumeImage(index, image, name, pViewProperty);
}

void CDoubleView::onInputDisplaySelected(CDataDisplay *pDisplay)
{
    int index = 0;
    QList<CDataDisplay*> displays = m_pDataViewer->getDataDisplay()->getDataViews();

    for(int i=0; i<displays.size(); ++i)
    {
        if(displays[i] != pDisplay)
            displays[i]->setSelected(false);
        else
            index = i;
    }

    displays = m_pResultsViewer->getDataViews();
    for(int i=0; i<displays.size(); ++i)
        displays[i]->setSelected(false);

    emit doSetSelectedDisplay(DisplayCategory::INPUT, index);
}

void CDoubleView::onOutputDisplaySelected(CDataDisplay *pDisplay)
{
    QList<CDataDisplay*> displays = m_pDataViewer->getDataDisplay()->getDataViews();
    for(int i=0; i<displays.size(); ++i)
        displays[i]->setSelected(false);

    int index = 0;
    displays = m_pResultsViewer->getDataViews();

    for(int i=0; i<displays.size(); ++i)
    {
        if(displays[i] != pDisplay)
            displays[i]->setSelected(false);
        else
            index = i;
    }

    emit doSetSelectedDisplay(DisplayCategory::OUTPUT, index);
}

void CDoubleView::onDisplayImage(int index, CImageScene *pScene, QImage image, QString name, CViewPropertyIO *pViewProperty)
{
    m_pDataViewer->fillZoomProperties(pViewProperty);
    m_pDataViewer->displayImage(index, pScene, image, name, pViewProperty);
}

void CDoubleView::onDisplayVideo(const QModelIndex &modelIndex, int displayIndex, CImageScene *pScene, QImage image, QString name, bool bVideoChanged, CViewPropertyIO *pViewProperty)
{
    m_pDataViewer->fillZoomProperties(pViewProperty);
    m_pDataViewer->displayVideo(modelIndex, displayIndex, pScene, image, name, bVideoChanged, pViewProperty);
}

void CDoubleView::onDisplayVolume(CImageScene *pScene, QImage image, QString imgName, bool bStackHasChanged, CViewPropertyIO *pViewProperty)
{
    m_pDataViewer->fillZoomProperties(pViewProperty);
    m_pDataViewer->displayVolume(pScene, image, imgName, bStackHasChanged, pViewProperty);
}

void CDoubleView::onDisplayPosition(CImageScene *pScene, QImage image, QString imgName, bool bStackHasChanged, CViewPropertyIO *pViewProperty)
{
    m_pDataViewer->fillZoomProperties(pViewProperty);
    m_pDataViewer->displayPosition(pScene, image, imgName, bStackHasChanged, pViewProperty);
}

void CDoubleView::onDisplayResultImage(int index, QImage image, const QString &imageName, CViewPropertyIO *pViewProperty)
{
    // Add image display to result viewer
    m_pDataViewer->fillZoomProperties(pViewProperty);
    auto pDisplay = m_pResultsViewer->displayImage(index, image, imageName);
    // Synchronize with main view
    m_pDataViewer->syncImageView(pDisplay);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onAddResultWidget(int index, QWidget* pWidget, bool bDeleteOnClose, CViewPropertyIO* pViewProperty)
{
    m_pResultsViewer->addWidgetDisplay(index, pWidget, bDeleteOnClose, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onAddGraphicsLayer(const CGraphicsLayerInfo &layerInfo)
{
    switch (layerInfo.m_displayTarget)
    {
        case CGraphicsLayerInfo::SOURCE:
            m_pDataViewer->addGraphicsLayer(layerInfo);
            break;
        case CGraphicsLayerInfo::RESULT:
            m_pResultsViewer->addGraphicsLayer(layerInfo);
            break;
    }
}

void CDoubleView::onRemoveGraphicsLayer(const CGraphicsLayerInfo &layerInfo, bool bDelete)
{
    switch (layerInfo.m_displayTarget)
    {
        case CGraphicsLayerInfo::SOURCE:
            m_pDataViewer->removeGraphicsLayer(layerInfo, bDelete);
            break;
        case CGraphicsLayerInfo::RESULT:
            m_pResultsViewer->removeGraphicsLayer(layerInfo, bDelete);
            break;
    }
}

void CDoubleView::onDisplayResultVideo(int index, QImage image, const QString& imageName, const std::vector<int>& syncToIndices, CViewPropertyIO* pViewProperty)
{
    // Add video display to result viewer
    m_pDataViewer->fillZoomProperties(pViewProperty);
    auto pDisplay = m_pResultsViewer->displayVideo(index, image, imageName);
    // Sync video views
    m_pDataViewer->syncVideoView(pDisplay, syncToIndices);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultText(int index, const QString &text, const QString &taskName, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayText(index, text, taskName, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultJson(int index, const QJsonDocument &jsonDocument, const QString &taskName, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayJson(index, jsonDocument, taskName, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultMeasuresTable(int index, const QString taskName, CMeasuresTableModel* pModel, CViewPropertyIO* pViewProperty)
{
    m_pResultsViewer->displayTable(index, taskName, pModel, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultFeaturesTable(int index, const QString taskName, CFeaturesTableModel *pModel, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayTable(index, taskName, pModel, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultPlot(int index, const QString& taskName, CDataPlot* pPlot, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayPlot(index, taskName, pPlot, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayMultiImage(CMultiImageModel *pModel, const QString &taskName, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayMultiImage(pModel, taskName, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onDisplayResultScene3d(const CScene3d& scene, int index, const QString &taskName, CViewPropertyIO *pViewProperty)
{
    m_pResultsViewer->displayScene3d(scene, index, taskName, pViewProperty);
    applyViewModeProperty(pViewProperty);
}

void CDoubleView::onUpdateIndex(const QModelIndex& index)
{
    emit doUpdateIndex(index);
}

//Unused
void CDoubleView::onImageCheck(CDataDisplay* pData)
{
    assert(pData != nullptr);
}

//Unused
void CDoubleView::onImageUncheck(CDataDisplay* pData)
{
    assert(pData != nullptr);
}

void CDoubleView::onClearResults()
{
    if(m_pDataViewer->isHidden())
        m_pDataViewer->show();

    m_pResultsViewer->hide();
    m_pResultsViewer->clearAll();

    // Notify data viewer that the result viewer is visible (ex: change style sheet for volume display)
    m_pDataViewer->notifyResultViewVisible(false);
}

void CDoubleView::onCloseProject(bool bCurrentImgClosed)
{
    m_pDataViewer->updateDataListViewIndex(QModelIndex());

    if(bCurrentImgClosed)
    {
        m_pDataViewer->notifySceneDeleted();
        m_pDataViewer->clearAll();
    }

    m_pResultsViewer->clearAll();
}

void CDoubleView::onInputDataChanged(const QModelIndex& index, bool bNewSequence)
{
    Q_UNUSED(index)
    if(bNewSequence)
    {
        m_pDataViewer->show();
        m_pResultsViewer->clearAll();
        emit doHideGraphicsToolbar();
    }
}

void CDoubleView::onSwitchView()
{
    assert(m_pDataViewer);
    assert(m_pResultsViewer);
    m_pDataViewer->switchView();
}

void CDoubleView::onImageDeleted()
{
    m_pDataViewer->notifySceneDeleted();
}

void CDoubleView::onHideResultsView()
{
    if(m_pResultsViewer)
        m_pResultsViewer->hide();
}

void CDoubleView::onBeforeResultDisplayRemoved(CDataDisplay *pDisplay)
{
    m_pDataViewer->unsyncView(pDisplay);
}

void CDoubleView::onStopVideo()
{
    m_pDataViewer->onStopVideo();
}

void CDoubleView::onStopVideoPlayer(const QModelIndex &index)
{
    m_pDataViewer->onStopVideoPlayer(index);
}

void CDoubleView::onShowHideDataViewer()
{
    if(m_pDataViewer->isHidden())
        m_pDataViewer->show();
    else
        m_pDataViewer->hide();
}

void CDoubleView::onShowHideResultsViewer()
{
    if(m_pResultsViewer->isHidden())
    {
        m_pResultsViewer->show();
        m_pDataViewer->notifyResultViewVisible(true);
        m_bShowResults = true;
    }
    else
    {
        m_pResultsViewer->hide();
        m_pDataViewer->notifyResultViewVisible(false);
        m_bShowResults = false;
    }
}

void CDoubleView::onHideDataView()
{
    if(m_pDataViewer->isVisible())
        m_pDataViewer->hide();
}

#include "moc_CDoubleView.cpp"
