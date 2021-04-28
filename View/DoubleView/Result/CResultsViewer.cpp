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
 * @file      CResultsViewer.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CResultsViewer
 *
 * @details   Details
 */

#include "CResultsViewer.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QMenu>
#include <QFileDialog>
#include "Model/Data/CFeaturesTableModel.h"
#include "Model/Data/CMeasuresTableModel.h"
#include "Model/Data/CMultiImageModel.h"
#include "View/DoubleView/Plot/CPlotDisplay.h"
#include "View/DoubleView/CStaticDisplay.h"
#include "View/DoubleView/Image/CImageDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"
#include "View/DoubleView/Image/CMultiImageDisplay.h"
#include "View/DoubleView/Video/CVideoViewSync.h"
#include "View/DoubleView/Result/CResultTableDisplay.h"
#include "View/DoubleView/Video/CVideoDisplay.h"
#include "View/DoubleView/CWidgetDataDisplay.h"
#include "Protocol/CViewPropertyIO.h"

CResultsViewer::CResultsViewer(CImageViewSync* pViewSync, CVideoViewSync* pVideoViewSync, QWidget* parent) : QWidget(parent)
{
    m_pImageViewSync = pViewSync;
    m_pVideoViewSync = pVideoViewSync;
    m_pLayout = new QVBoxLayout;
    m_pLayout->setMargin(10);
    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->setProperty("class", "ResultTabWidget");
    m_pTabWidget->tabBar()->setProperty("class", "ResultTabWidget");
    m_pTabWidget->setContentsMargins(0, 0, 0, 0);
    m_pTabWidget->setTabPosition(QTabWidget::North);
    m_pLayout->addWidget(m_pTabWidget);
    setLayout(m_pLayout);
    installEventFilter(this);
}

void CResultsViewer::initDisplay(const OutputDisplays& displaysProp)
{
    if(displaysProp.empty())
    {
        clearAll();
        return;
    }

    //Remove unnecessary tabs
    for(auto it=m_mapTypeIndex.begin(); it!= m_mapTypeIndex.end();)
    {
        auto searchIt = displaysProp.find(it.key());
        if(searchIt == displaysProp.end())
        {
            auto tabType = it.key();
            ++it;
            removeTab(tabType);
        }
        else
            ++it;
    }

    //Init tab displays
    for(auto it=displaysProp.begin(); it!=displaysProp.end(); ++it)
    {
        switch(it->first)
        {
            case DisplayType::IMAGE_DISPLAY:
            case DisplayType::VIDEO_DISPLAY:
                initImageBasedDisplay(it->first, it->second);
                break;
            case DisplayType::TABLE_DISPLAY:
                break;
            default:
                auto itTab = m_mapTypeIndex.find(it->first);
                if(itTab != m_mapTypeIndex.end())
                {
                    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->widget(itTab.value()));
                    if(pView)
                        pView->removeAll();
                }
                break;
        }
    }
}

CImageDisplay *CResultsViewer::displayImage(size_t index, QImage image, const QString &name)
{
    if(hasTab(DisplayType::IMAGE_DISPLAY) == false)
        addTabToResults(DisplayType::IMAGE_DISPLAY);

    CImageDisplay* pDisplay = nullptr;
    auto displays = getDataViews(DisplayType::IMAGE_DISPLAY);

    if((int)index >= displays.size())
    {
        if((index - displays.size()) > 0)
        {
            qCritical().noquote() << tr("Error while creating image display : invalid index");
            return nullptr;
        }

        //Create new one
        pDisplay = createImageDisplay();
        addDataViewToTab(DisplayType::IMAGE_DISPLAY, pDisplay);
    }
    else
        pDisplay = static_cast<CImageDisplay*>(displays[index]);

    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->setImage(image, false);
    pDisplay->setName(name);        
    return pDisplay;
}

CVideoDisplay *CResultsViewer::displayVideo(size_t index, QImage image, const QString &name)
{
    if(hasTab(DisplayType::VIDEO_DISPLAY) == false)
        addTabToResults(DisplayType::VIDEO_DISPLAY);

    CVideoDisplay* pDisplay = nullptr;
    auto displays = getDataViews(DisplayType::VIDEO_DISPLAY);

    if((int)index >= displays.size())
    {
        if((index - displays.size()) > 0)
        {
            qCritical().noquote() << tr("Error while creating image display : invalid index");
            return nullptr;
        }

        //Create new one
        pDisplay = createVideoDisplay();
        addDataViewToTab(DisplayType::VIDEO_DISPLAY, pDisplay);
    }
    else
        pDisplay = static_cast<CVideoDisplay*>(displays[index]);

    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->setImage(nullptr, image, name, false);
    return pDisplay;
}

CResultTableDisplay *CResultsViewer::displayTable(const QString name, CMeasuresTableModel *pModel, CViewPropertyIO *pViewProperty)
{
    CResultTableDisplay* pResultDisplay = createTableDisplay(pViewProperty);
    pResultDisplay->setName(name);
    pResultDisplay->setModel(pModel);
    pResultDisplay->show();
    return pResultDisplay;
}

CResultTableDisplay *CResultsViewer::displayTable(const QString name, CFeaturesTableModel *pModel, CViewPropertyIO *pViewProperty)
{
    CResultTableDisplay* pResultDisplay = createTableDisplay(pViewProperty);
    pResultDisplay->setName(name);
    pResultDisplay->setModel(pModel);
    pResultDisplay->show();
    return pResultDisplay;
}

CPlotDisplay *CResultsViewer::displayPlot(const QString &name, CDataPlot *pPlot)
{
    if(hasTab(DisplayType::PLOT_DISPLAY) == false)
        addTabToResults(DisplayType::PLOT_DISPLAY);

    CPlotDisplay* pDisplay = nullptr;
    auto displays = getDataViews(DisplayType::PLOT_DISPLAY);

    if(displays.size() == 0)
    {
        pDisplay = new CPlotDisplay;
        addDataViewToTab(DisplayType::PLOT_DISPLAY, pDisplay);
    }
    else
        pDisplay = static_cast<CPlotDisplay*>(displays[0]);

    pDisplay->setName(name);
    pDisplay->setDataPlot(pPlot);
    pDisplay->show();
    return pDisplay;
}

CMultiImageDisplay *CResultsViewer::displayMultiImage(CMultiImageModel *pModel, const QString &name, CViewPropertyIO *pViewProperty)
{
    if(hasTab(DisplayType::MULTI_IMAGE_DISPLAY) == false)
        addTabToResults(DisplayType::MULTI_IMAGE_DISPLAY);

    CMultiImageDisplay* pDisplay = nullptr;
    auto displays = getDataViews(DisplayType::MULTI_IMAGE_DISPLAY);

    if(displays.size() == 0)
    {
        pDisplay = new CMultiImageDisplay;
        addDataViewToTab(DisplayType::MULTI_IMAGE_DISPLAY, pDisplay);
    }
    else
        pDisplay = static_cast<CMultiImageDisplay*>(displays[0]);

    pDisplay->setName(name);
    pDisplay->setModel(pModel);
    pDisplay->setViewProperty(pViewProperty);
    pDisplay->show();
    return pDisplay;
}

CWidgetDataDisplay *CResultsViewer::addWidgetDisplay(size_t index, QWidget *pWidget, bool bDeleteOnClose, CViewPropertyIO *pViewProperty)
{
    if(hasTab(DisplayType::WIDGET_DISPLAY) == false)
        addTabToResults(DisplayType::WIDGET_DISPLAY);

    CWidgetDataDisplay* pDisplay = nullptr;
    auto displays = getDataViews(DisplayType::WIDGET_DISPLAY);

    if((int)index >= displays.size())
    {
        if((index - displays.size()) > 0)
        {
            qCritical().noquote() << tr("Error while creating widget display : invalid index");
            return nullptr;
        }

        //Create new one
        pDisplay = new CWidgetDataDisplay(pWidget, nullptr, bDeleteOnClose);
        pWidget->installEventFilter(pDisplay);
        addDataViewToTab(DisplayType::WIDGET_DISPLAY, pDisplay);
        pDisplay->setViewProperty(pViewProperty);
        pDisplay->show();
    }
    else
        pDisplay = static_cast<CWidgetDataDisplay*>(displays[index]);

    return pDisplay;
}

int CResultsViewer::addTabToResults(DisplayType type)
{
    assert(m_pTabWidget != nullptr);

    //One tab per type
    auto it = m_mapTypeIndex.find(type);
    if(it != m_mapTypeIndex.end())
        return it.value();

    //Remove Empty tab if necessary
    if(type != DisplayType::EMPTY_DISPLAY)
        removeTab(DisplayType::EMPTY_DISPLAY);

    int indTab = 0;
    switch(type)
    {
        case DisplayType::EMPTY_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/tool-table-color.png"), tr("Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, "Results");
            m_mapTypeIndex.insert(type, indTab);
            addDefaultDisplay(indTab);
            break;

        case DisplayType::IMAGE_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/view-image-color.png"), tr("Image Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Image Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        case DisplayType::TABLE_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/tool-table-color.png"), tr("Table Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Table Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        case DisplayType::PLOT_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/view-plot-color.png"), tr("Plot Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Plot Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        case DisplayType::VIDEO_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/view-plot-color.png"), tr("Video Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Video Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        case DisplayType::WIDGET_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/view-plot-color.png"), tr("Custom Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Custom Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        case DisplayType::MULTI_IMAGE_DISPLAY:
            indTab = m_pTabWidget->addTab(new CDataDisplay(this), QIcon(":/Images/view-image-color.png"), tr("Dataset Results"));
            m_pTabWidget->setIconSize(QSize(16,16));
            m_pTabWidget->setTabToolTip(indTab, tr("Dnn Datatset Results"));
            m_mapTypeIndex.insert(type, indTab);
            break;

        default:
            break;
    }
    return indTab;
}

int CResultsViewer::addTabToResults(DisplayType type, const QString& name, const QIcon& icon)
{
    int indTab = m_pTabWidget->addTab(new CDataDisplay, icon, "");
    m_pTabWidget->setIconSize(QSize(48,48));
    m_pTabWidget->setTabBarAutoHide(true);
    m_pTabWidget->setTabToolTip(indTab, name);
    m_mapTypeIndex.insert(type, indTab);
    return indTab;
}

void CResultsViewer::addDataViewToTab(DisplayType type, CDataDisplay *pData)
{
    assert(m_pTabWidget);
    assert(pData);

    int index = getTabIndex(type);
    auto pContainer = static_cast<CDataDisplay*>(m_pTabWidget->widget(index));
    assert(pContainer);
    pContainer->addDataView(pData);
    initConnections(pData);
}

void CResultsViewer::addDataViewToTab(int indTab, CDataDisplay* pData, int r, int c)
{
    assert(m_pTabWidget);
    assert(pData);

    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->widget(indTab));
    assert(pView);
    pView->addDataView(pData, r, c);
    initConnections(pData);
}

int CResultsViewer::getTabCount() const
{
    assert(m_pTabWidget != nullptr);
    return m_pTabWidget->count();
}

int CResultsViewer::getTabIndex(DisplayType type) const
{
    auto it = m_mapTypeIndex.find(type);
    if(it != m_mapTypeIndex.end())
        return it.value();

    return -1;
}

QTabWidget*CResultsViewer::getTabWidget() const
{
    return m_pTabWidget;
}

CDataDisplay* CResultsViewer::getDataViewFromTabIndex(int indTab, int r, int c) const
{
    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->widget(indTab));
    return pView->getDataView(r, c);
}

QList<CDataDisplay*> CResultsViewer::getDataViews(DisplayType type) const
{
    int index = getTabIndex(type);
    if(index == -1)
        return QList<CDataDisplay*>();

    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->widget(index));
    if(pView)
        return pView->getDataViews();
    else
        return QList<CDataDisplay*>();
}

QList<CDataDisplay *> CResultsViewer::getDataViews() const
{
    QList<CDataDisplay*> displays;
    for(int i=0; i<m_pTabWidget->count(); ++i)
    {
        auto pView = static_cast<CDataDisplay*>(m_pTabWidget->widget(i));
        if(pView)
            displays += pView->getDataViews();
    }
    return displays;
}

bool CResultsViewer::isEmpty() const
{
    auto it = m_mapTypeIndex.find(DisplayType::EMPTY_DISPLAY);
    return (m_mapTypeIndex.empty() || it != m_mapTypeIndex.end());
}

bool CResultsViewer::hasTab(DisplayType type) const
{
    auto it = m_mapTypeIndex.find(type);
    return it != m_mapTypeIndex.end();
}

void CResultsViewer::removeTab(DisplayType type)
{
    auto it = m_mapTypeIndex.find(type);
    if(it != m_mapTypeIndex.end())
    {
        auto indexRemoved = it.value();
        // Get widget for deleting it after removal
        CDataDisplay* pContainer = static_cast<CDataDisplay*>(m_pTabWidget->widget(indexRemoved));
        auto dataDisplays = pContainer->getDataViews();

        //Notify that all data displays of the tab will be destroyed -> disconnet synchronisation signals
        for(int i=0; i<dataDisplays.size(); ++i)
            emit doBeforeDisplayRemoved(dataDisplays[i]);

        m_pTabWidget->removeTab(indexRemoved);
        pContainer->deleteLater();
        m_mapTypeIndex.erase(it);
        decrementTabIndex(indexRemoved);
    }
}

void CResultsViewer::clearAll()
{
    if(m_mapTypeIndex.size() == 1 && m_mapTypeIndex.begin().key() == DisplayType::EMPTY_DISPLAY)
        return;

    // Delete all widget in tabwidget
    for(auto it = m_mapTypeIndex.constBegin(); it != m_mapTypeIndex.constEnd(); ++it )
        clearTab(it);

    m_mapTypeIndex.clear();
    m_pTabWidget->clear();
    addTabToResults(DisplayType::EMPTY_DISPLAY);
}

void CResultsViewer::onDataViewDblClicked(CDataDisplay* pData)
{
    Q_UNUSED(pData);
    emit doToggleMaximize();
}

void CResultsViewer::onDataViewToggleMaximize(CDataDisplay *pData)
{
    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->currentWidget());
    int nbView = pView->getDataViewCount();

    if(nbView == 1)
        emit doToggleMaximize();
    else
        pView->toggleMaximizeView(pData);
}

void CResultsViewer::onDataViewMaximize(CDataDisplay *pData)
{
    auto pView = static_cast<CDataDisplay*>(m_pTabWidget->currentWidget());
    int nbView = pView->getDataViewCount();

    if(nbView == 1)
        emit doMaximize();
    else
        pView->maximizeView(pData);
}

void CResultsViewer::initImageBasedDisplay(DisplayType type, const std::vector<CViewPropertyIO*>& viewProps)
{
    if(hasTab(type) == false)
        addTabToResults(type);

    int index = getTabIndex(type);
    auto pDataDisplay = static_cast<CDataDisplay*>(m_pTabWidget->widget(index));
    auto displays = pDataDisplay->getDataViews();
    int nbDisplay = displays.size();
    int nb = (int)viewProps.size();

    if(nb < nbDisplay)
    {
        //Remove unnecessary displays
        QList<CDataDisplay*> displaysToRemove;
        for(int i=nb; i<nbDisplay; ++i)
        {
            emit doBeforeDisplayRemoved(displays[i]);
            unsyncView(type, displays[i]);
            displaysToRemove.push_back(displays[i]);
        }
        pDataDisplay->removeDataViews(displaysToRemove, true);
    }
    else if(nb > nbDisplay)
    {
        //Add displays
        for(int i=0; i<(nb - nbDisplay); ++i)
        {
            auto pDisplay = createDisplay(type);
            pDataDisplay->addDataView(pDisplay);
            initConnections(pDisplay);
        }
    }

    //Set view properties for each display
    int maximizedDisplayIndex = -1;
    displays = pDataDisplay->getDataViews();
    assert(displays.size() == nb);

    for(int i=0; i<nb; ++i)
        displays[i]->setViewProperty(viewProps[i]);

    //Update maximized status
    for(int i=0; i<nb; ++i)
    {
        if(viewProps[i]->isMaximized())
        {
            maximizedDisplayIndex = i;
            break;
        }
    }

    if(maximizedDisplayIndex == -1)
    {
        for(int i=0; i<displays.size(); ++i)
        {
            displays[i]->show();
            displays[i]->setMaximized(false);
        }
    }
    else
        pDataDisplay->maximizeView(displays[maximizedDisplayIndex]);
}

void CResultsViewer::onApplyViewProperty()
{
    for(int i=0; i<m_pTabWidget->count(); ++i)
    {
        auto pContainer = static_cast<CDataDisplay*>(m_pTabWidget->widget(i));
        if(pContainer)
        {
            auto views = pContainer->getDataViews();
            for(int i=0; i<views.size(); ++i)
                views[i]->applyViewProperty();
        }
    }
}

void CResultsViewer::onAddGraphicsLayer(const CGraphicsLayerInfo& layerInfo)
{
    auto pImageView = static_cast<CImageDisplay*>(getDataView(DisplayType::IMAGE_DISPLAY, layerInfo.m_imageIndex));
    if(pImageView)
        pImageView->getView()->addGraphicsLayer(layerInfo.m_pLayer, layerInfo.m_bTopMost);
    else
    {
        auto pVideoView = static_cast<CVideoDisplay*>(getDataView(DisplayType::VIDEO_DISPLAY, layerInfo.m_imageIndex));
        if(pVideoView)
            pVideoView->getImageDisplay()->getView()->addGraphicsLayer(layerInfo.m_pLayer, layerInfo.m_bTopMost);
    }
}

void CResultsViewer::onRemoveGraphicsLayer(const CGraphicsLayerInfo &layerInfo, bool bDelete)
{
    auto pImageView = static_cast<CImageDisplay*>(getDataView(DisplayType::IMAGE_DISPLAY, layerInfo.m_imageIndex));
    if(pImageView)
        pImageView->getView()->removeGraphicsLayer(layerInfo.m_pLayer, bDelete);
    else
    {
        auto pVideoView = static_cast<CVideoDisplay*>(getDataView(DisplayType::VIDEO_DISPLAY, layerInfo.m_imageIndex));
        if(pVideoView)
            pVideoView->getImageDisplay()->getView()->removeGraphicsLayer(layerInfo.m_pLayer, bDelete);
    }
}

void CResultsViewer::onDisplayOverlay(const QImage &image, int imageIndex)
{
    auto pImageView = static_cast<CImageDisplay*>(getDataView(DisplayType::IMAGE_DISPLAY, imageIndex));
    if(pImageView)
        pImageView->getView()->setOverlayImage(image);
    else
    {
        auto pVideoView = static_cast<CVideoDisplay*>(getDataView(DisplayType::VIDEO_DISPLAY, imageIndex));
        if(pVideoView)
            pVideoView->getImageDisplay()->getView()->setOverlayImage(image);
    }
}

void CResultsViewer::onClearOverlay()
{
    auto imageViews = getDataViews(DisplayType::IMAGE_DISPLAY);
    for(int i=0; i<imageViews.size(); ++i)
        static_cast<CImageDisplay*>(imageViews[i])->getView()->clearOverlay();

    auto videoViews = getDataViews(DisplayType::VIDEO_DISPLAY);
    for(int i=0; i<videoViews.size(); ++i)
        static_cast<CVideoDisplay*>(videoViews[i])->getImageDisplay()->getView()->clearOverlay();
}

void CResultsViewer::onSetVideoLength(int index, int length)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetSliderLength(length);
        m_pVideoViewSync->syncSliderLength(pDisplay, length);
    }
}

void CResultsViewer::onSetVideoPos(int index, int pos)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetSliderPos(pos);
        m_pVideoViewSync->syncSliderPos(pDisplay, pos);
    }
}

void CResultsViewer::onSetVideoFPS(int index, double fps)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetFPS(fps);
        m_pVideoViewSync->syncFps(pDisplay, fps);
    }
}

void CResultsViewer::onSetVideoTotalTime(int index, int totalTime)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetTotalTime(totalTime);
        m_pVideoViewSync->syncTotalTime(pDisplay, totalTime);
    }
}

void CResultsViewer::onSetVideoCurrentTime(int index, int currentTime)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetCurrentTime(currentTime);
        m_pVideoViewSync->syncCurrentTime(pDisplay, currentTime);
    }
}

void CResultsViewer::onSetVideoSourceType(int index, CDataVideoBuffer::Type srcType)
{
    auto views = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < views.size())
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->setSourceType(srcType);
        m_pVideoViewSync->syncSourceType(pDisplay, srcType);
    }
}

void CResultsViewer::onStopRecordingVideo(int index)
{
    auto videoViews = getDataViews(DisplayType::VIDEO_DISPLAY);
    if(index < videoViews.size())
        static_cast<CVideoDisplay*>(videoViews[index])->onRecordVideo();
}

void CResultsViewer::initConnections(CDataDisplay* pData)
{
    connect(pData, &CDataDisplay::doDoubleClicked, this, &CResultsViewer::onDataViewDblClicked);
    connect(pData, &CDataDisplay::doToggleMaximize, this, &CResultsViewer::onDataViewToggleMaximize);
    connect(pData, &CDataDisplay::doMaximize, this, &CResultsViewer::onDataViewMaximize);
    connect(pData, &CDataDisplay::doSelect, [&](CDataDisplay* pDisplay)
    {
        emit doDisplaySelected(pDisplay);
    });

    if(pData->getTypeId() == DisplayType::IMAGE_DISPLAY)
        initImageConnections(static_cast<CImageDisplay*>(pData));
    else if(pData->getTypeId() == DisplayType::TABLE_DISPLAY)
        initTableConnections(static_cast<CResultTableDisplay*>(pData));
    else if(pData->getTypeId() == DisplayType::VIDEO_DISPLAY)
        initVideoConnections(static_cast<CVideoDisplay*>(pData));
    else if(pData->getTypeId() == DisplayType::MULTI_IMAGE_DISPLAY)
        initMultiImageConnections(static_cast<CMultiImageDisplay*>(pData));
}

void CResultsViewer::initImageConnections(CImageDisplay *pDisplay)
{
    connect(pDisplay, &CImageDisplay::doSave, [this, pDisplay]
    {
        int index = getDataViewIndex(pDisplay);
        emit doSaveCurrentResultImage(index);
    });
    connect(pDisplay, &CImageDisplay::doExport, [this, pDisplay](const QString& path, bool bWithGraphics)
    {
        int index = getDataViewIndex(pDisplay);
        emit doExportCurrentResultImage(index, path, bWithGraphics);
    });

    syncImageView(pDisplay);
}

void CResultsViewer::initVideoConnections(CVideoDisplay* pDisplay)
{
    connect(pDisplay, &CVideoDisplay::doSaveVideo, [&, pDisplay]()
    {
        emit doSaveCurrentResultVideo(getDataViewIndex(pDisplay));
    });
    connect(pDisplay, &CVideoDisplay::doExportVideo, [&, pDisplay](const QString& path)
    {
        emit doExportCurrentResultVideo(getDataViewIndex(pDisplay), path);
    });
    connect(pDisplay, &CVideoDisplay::doRecordVideo, [&, pDisplay](bool bRecord)
    {
        emit doRecordVideo(getDataViewIndex(pDisplay), bRecord);
    });
    connect(pDisplay, &CVideoDisplay::doImageDoubleClicked, [&]
    {
        emit doToggleMaximize();
    });
    connect(pDisplay, &CVideoDisplay::doSaveCurrentFrame, [&, pDisplay]
    {
        int index = getDataViewIndex(pDisplay);
        emit doSaveCurrentResultImage(index);
    });
    connect(pDisplay, &CVideoDisplay::doExportCurrentFrame, [&, pDisplay](const QString& path, bool bWithGraphics)
    {
        int index = getDataViewIndex(pDisplay);
        emit doExportCurrentResultImage(index, path, bWithGraphics);
    });

    syncVideoView(pDisplay);
}

void CResultsViewer::initTableConnections(CResultTableDisplay *pDisplay)
{
    connect(pDisplay, &CResultTableDisplay::doSave, [&]{ emit doSaveCurrentTableData(); });
    connect(pDisplay, &CResultTableDisplay::doExport, [&](const QString& path){ emit doExportCurrentTableData(path); });
}

void CResultsViewer::initMultiImageConnections(CMultiImageDisplay *pDisplay)
{
    connect(pDisplay, &CMultiImageDisplay::doExportImage, [&](const QString& path, CMat& img, CGraphicsLayer* pLayer)
    {
        emit doExportDatasetImage(path, img, pLayer);
    });
}

CDataDisplay *CResultsViewer::getDataView(DisplayType type, int index) const
{
    auto dataViews = getDataViews(type);
    if(index >= 0 && index < dataViews.size())
        return dataViews[index];
    else
        return nullptr;
}

int CResultsViewer::getDataViewIndex(CDataDisplay *pDisplay) const
{
    assert(pDisplay);
    int index = 0;
    auto displays = getDataViews(static_cast<DisplayType>(pDisplay->getTypeId()));

    for(int i=0; i<displays.size(); ++i)
    {
        if(pDisplay == displays[i])
            return index;

        index++;
    }
    return -1;
}

CDataDisplay *CResultsViewer::createDisplay(DisplayType type)
{
    if(type == DisplayType::IMAGE_DISPLAY)
        return createImageDisplay();
    else if(type == DisplayType::VIDEO_DISPLAY)
        return createVideoDisplay();
    else
        return nullptr;
}

CImageDisplay *CResultsViewer::createImageDisplay()
{
    int imgDisplayFlags = CImageDisplay::DEFAULT | CImageDisplay::SAVE_BUTTON;
    auto pDisplay = new CImageDisplay(nullptr, "", imgDisplayFlags);
    pDisplay->setSelectOnClick(true);
    pDisplay->enableGraphicsInteraction(false);
    return pDisplay;
}

CVideoDisplay* CResultsViewer::createVideoDisplay()
{
    CVideoDisplay* pVideoDisplay = new CVideoDisplay(nullptr, "", CVideoDisplay::RESULTS);
    pVideoDisplay->setSelectOnClick(true);
    pVideoDisplay->enableGraphicsInteraction(false);
    return pVideoDisplay;
}

CResultTableDisplay *CResultsViewer::createTableDisplay(CViewPropertyIO *pViewProperty)
{
    CResultTableDisplay* pResultDisplay;
    if(hasTab(DisplayType::TABLE_DISPLAY) == false)
    {
        addTabToResults(DisplayType::TABLE_DISPLAY);
        pResultDisplay = new CResultTableDisplay;
        pResultDisplay->setViewProperty(pViewProperty);
        addDataViewToTab(DisplayType::TABLE_DISPLAY, pResultDisplay);
    }
    else
    {
        int ind = getTabIndex(DisplayType::TABLE_DISPLAY);
        pResultDisplay = static_cast<CResultTableDisplay*>(getDataViewFromTabIndex(ind, 0, 0));

        // If not present, create it
        if(pResultDisplay == nullptr)
        {
            pResultDisplay = new CResultTableDisplay;
            addDataViewToTab(DisplayType::TABLE_DISPLAY, pResultDisplay);
        }
    }
    return pResultDisplay;
}

void CResultsViewer::addDefaultDisplay(int tabIndex)
{
    auto pDefaultDisplay = new CStaticDisplay(tr("No result to show"), "Static display");
    pDefaultDisplay->setFont(24, QFont::Bold);
    addDataViewToTab(tabIndex, pDefaultDisplay, 0, 0);
}

void CResultsViewer::decrementTabIndex(int indexFrom)
{
    for(auto it=m_mapTypeIndex.begin(); it!= m_mapTypeIndex.end(); ++it)
    {
        if(it.value() > indexFrom)
            it.value()--;
    }
}

void CResultsViewer::syncImageView(CImageDisplay *pDisplay)
{
    //Synchronize with existing image displays
    if(m_pImageViewSync == nullptr)
        return;

    auto displays = getDataViews(DisplayType::IMAGE_DISPLAY);
    for(int i=0; i<displays.size(); ++i)
    {
        if(displays[i] != pDisplay)
            m_pImageViewSync->syncView(static_cast<CImageDisplay*>(displays[i]), pDisplay);
    }
}

void CResultsViewer::syncVideoView(CVideoDisplay *pDisplay)
{
    //Synchronize with existing video displays
    if(m_pVideoViewSync == nullptr)
        return;

    auto displays = getDataViews(DisplayType::VIDEO_DISPLAY);
    for(int i=0; i<displays.size(); ++i)
    {
        if(displays[i] != pDisplay)
            m_pVideoViewSync->syncView(static_cast<CVideoDisplay*>(displays[i]), pDisplay);
    }
}

void CResultsViewer::unsyncView(DisplayType type, CDataDisplay *pDisplay)
{
    if(type == DisplayType::IMAGE_DISPLAY)
        unsyncImageView(dynamic_cast<CImageDisplay*>(pDisplay));
    else if(type == DisplayType::VIDEO_DISPLAY)
        unsyncVideoView(dynamic_cast<CVideoDisplay*>(pDisplay));
}

void CResultsViewer::unsyncImageView(CImageDisplay* pDisplay)
{
    assert(pDisplay);
    if(m_pImageViewSync == nullptr)
        return;

    auto displays = getDataViews(DisplayType::IMAGE_DISPLAY);
    for(int i=0; i<displays.size(); ++i)
    {
        auto pTmpDisplay = static_cast<CImageDisplay*>(displays[i]);
        if(pTmpDisplay != pDisplay)
            m_pImageViewSync->unsyncView(pTmpDisplay, pDisplay);
    }
}

void CResultsViewer::unsyncVideoView(CVideoDisplay* pDisplay)
{
    assert(pDisplay);
    if(m_pVideoViewSync == nullptr)
        return;

    auto displays = getDataViews(DisplayType::VIDEO_DISPLAY);
    for(int i=0; i<displays.size(); ++i)
    {
        auto pTmpDisplay = static_cast<CVideoDisplay*>(displays[i]);
        if(pTmpDisplay !=  pDisplay)
            m_pVideoViewSync->unsyncView(pTmpDisplay, pDisplay);
    }
}

void CResultsViewer::clearTab(QMap<DisplayType, int>::const_iterator tabIt)
{
    auto pDataDisplay = static_cast<CDataDisplay*>(m_pTabWidget->widget(tabIt.value()));
    assert(pDataDisplay);
    // Ensure that all synchronizations (images and/or videos) are removed
    switch(tabIt.key())
    {
        case DisplayType::IMAGE_DISPLAY:
            clearImageSync(pDataDisplay);
            break;
        case DisplayType::VIDEO_DISPLAY:
            clearVideoSync(pDataDisplay);
            break;
        default:
            break;
    }
    pDataDisplay->deleteLater();
}

void CResultsViewer::clearImageSync(CDataDisplay* pDataDisplay)
{
    auto displays = pDataDisplay->getDataViews();

    for(auto&& it : displays)
    {
        emit doBeforeDisplayRemoved(it);
        unsyncImageView(static_cast<CImageDisplay*>(it));
    }
}

void CResultsViewer::clearVideoSync(CDataDisplay* pDataDisplay)
{
    auto displays = pDataDisplay->getDataViews();

    for(auto&& it : displays)
    {
        emit doBeforeDisplayRemoved(it);
        unsyncVideoView(static_cast<CVideoDisplay*>(it));
    }
}
