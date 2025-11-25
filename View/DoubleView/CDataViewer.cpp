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
 * @file      CDataViewer.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDataViewer
 *
 * @details   Details
 */

#include "CDataViewer.h"
#include "CDataDisplay.h"
#include "CDataListView.h"
#include "3D/CVolumeDisplay.h"
#include "3D/C3dDisplay.h"
#include "View/Common/CGLWidget.h"
#include "View/DoubleView/CStaticDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"
#include "View/DoubleView/Video/CVideoViewSync.h"
#include "View/DoubleView/3D/CVolumeViewSync.h"
#include "Model/Project/CMultiProjectModel.h"
#include "Model/Project/CProjectViewProxyModel.h"
#include <QStackedWidget>
#include <QModelIndex>

CDataViewer::CDataViewer(QWidget* parent) : QWidget(parent)
{
    setObjectName("DataViewer");

    m_pDataDisplay = new CDataDisplay;
    auto pEmptyDisplay = createEmptyDisplay();
    m_pDataDisplay->addDataView(pEmptyDisplay);

    m_pDataListView = new CDataListView();

    m_pPathNavigator = new CPathNavigator;

    m_pStacked = new QStackedWidget;
    m_pStacked->addWidget(m_pDataListView);
    m_pStacked->addWidget(m_pDataDisplay);
    setActiveWidget(1);

    m_pLayout = new QVBoxLayout;
    m_pLayout->setContentsMargins(10, 10, 10, 10);
    m_pLayout->setSpacing(2);
    m_pLayout->addWidget(m_pPathNavigator);
    m_pLayout->addWidget(m_pStacked);

    setLayout(m_pLayout);

    initConnections();
}

CStaticDisplay *CDataViewer::createEmptyDisplay()
{
    auto pDisplay = new CStaticDisplay(tr("No data loaded.\nUse top buttons to load project, images, videos or camera."), "Static display", this);
    pDisplay->setBackground(":/Images/background.png");
    pDisplay->setFont(24, QFont::Bold);
    return pDisplay;
}

CImageDisplay *CDataViewer::createImageDisplay()
{
    auto pDisplay = new CImageDisplay(nullptr, "", CImageDisplay::DEFAULT | CImageDisplay::GRAPHICS_BUTTON);
    pDisplay->setSelectOnClick(true);
    initDisplayConnections(pDisplay);
    initImageConnections(pDisplay);
    syncImageView(pDisplay);
    return pDisplay;
}

CVideoDisplay *CDataViewer::createVideoDisplay()
{
    auto pDisplay = new CVideoDisplay(this, "", CVideoDisplay::PLAYER);
    pDisplay->setSelectOnClick(true);
    initDisplayConnections(pDisplay);
    initVideoConnections(pDisplay);
    return pDisplay;
}

CVolumeDisplay *CDataViewer::createVolumeDisplay()
{
    auto pDisplay = new CVolumeDisplay(this);
    pDisplay->setSelectOnClick(true);
    initDisplayConnections(pDisplay);
    initVolumeConnections(pDisplay);
    return pDisplay;
}

CPositionDisplay *CDataViewer::createPositionDisplay()
{
    auto pDisplay = new CPositionDisplay(this);
    pDisplay->setSelectOnClick(true);
    initDisplayConnections(pDisplay);
    initPositionConnections(pDisplay);
    return pDisplay;
}

void CDataViewer::initConnections()
{
    connect(m_pDataListView, &CDataListView::doUpdateIndex, this, &CDataViewer::onUpdateIndex);

    connect(m_pPathNavigator, &CPathNavigator::doUpdateIndex, [&](const QModelIndex& index)
    {
        displayListView();       
        onUpdateIndex(index);
    });
    connect(m_pPathNavigator, &CPathNavigator::doGoHome, [&](const QModelIndex& index)
    {
        displayListView();
        m_pDataListView->setSourceModel( m_pDataListView->proxyModel()->sourceModel());
        onUpdateIndex(index);
    });
}

void CDataViewer::initDisplayConnections(CDataDisplay *pDisplay)
{
    connect(pDisplay, &CDataDisplay::doSelect, [&](CDataDisplay* pDisplay){ emit doDisplaySelected(pDisplay); });
}

void CDataViewer::initImageConnections(CImageDisplay *pDisplay)
{
    //Image display -> data viewer
    connect(pDisplay, &CImageDisplay::doUpdateIndex, [&](int index){ emit doUpdateImageIndex(index); });
    connect(pDisplay, &CImageDisplay::doToggleMaximize, this, &CDataViewer::onDataViewToggleMaximize);
    connect(pDisplay, &CImageDisplay::doDoubleClicked, [&]{ emit doToggleMaximize(); });
    connect(pDisplay, &CImageDisplay::doExport, [this, pDisplay](const QString& path, bool bWithGraphics)
    {
        int index = getDisplayIndex(pDisplay);
        emit doExportCurrentImage(index, path, bWithGraphics);
    });
    connect(pDisplay, &CImageDisplay::doToggleGraphicsToolbar, [&]{ emit doToggleGraphicsToolbar(); });

    //Image view -> data viewer
    initImageViewConnections(pDisplay->getView());
}

void CDataViewer::initImageViewConnections(CImageView *pView)
{
    connect(pView, &CImageView::doChangeGraphicsActivationState, [&](bool bActivated){ emit doChangeGraphicsActivationState(bActivated); });
    connect(pView, &CImageView::doAddGraphicsLayer, [&](CGraphicsLayer* pLayer){ emit doAddGraphicsLayer(pLayer); });
    connect(pView, &CImageView::doGraphicsChanged, [&]{ emit doGraphicsChanged(); });
    connect(pView, &CImageView::doGraphicsRemoved, [&](const QSet<CGraphicsLayer *> &layers){ emit doGraphicsRemoved(layers); });
}

void CDataViewer::initVideoConnections(CVideoDisplay *pDisplay)
{
    //Video display signals
    connect(pDisplay, &CVideoDisplay::doToggleMaximize, this, &CDataViewer::onDataViewToggleMaximize);

    connect(pDisplay, &CVideoDisplay::doUpdateVideoPos, [&, pDisplay](size_t pos)
    {
        int index = getDisplayIndex(pDisplay);
        emit doUpdateVideoPos(m_displayIndexToModelIndex[index], index, pos);
    });
    connect(pDisplay, &CVideoDisplay::doPlayVideo, [&, pDisplay]
    {
        int index = getDisplayIndex(pDisplay);
        if(canPlayVideo(pDisplay))
            emit doPlayVideo(index);
        else
            pDisplay->onStopVideo();
    });
    connect(pDisplay, &CVideoDisplay::doStopVideo, [&, pDisplay]
    {
        int index = getDisplayIndex(pDisplay);
        emit doStopVideo(m_displayIndexToModelIndex[index]);
    });
    connect(pDisplay, &CVideoDisplay::doRecordVideo, [&, pDisplay](bool bRecord)
    {
        int index = getDisplayIndex(pDisplay);
        emit doRecordVideo(m_displayIndexToModelIndex[index], bRecord);
    });
    connect(pDisplay, &CVideoDisplay::doNotifyVideoStart, [&, pDisplay]
    {
        int index = getDisplayIndex(pDisplay);
        emit doNotifyVideoStart(m_displayIndexToModelIndex[index]);
    });
    connect(pDisplay, &CVideoDisplay::doImageDoubleClicked, [&]
    {
        emit doToggleMaximize();
    });
    connect(pDisplay, &CVideoDisplay::doSaveCurrentFrame, [&, pDisplay]
    {
        int index = getDisplayIndex(pDisplay);
        emit doSaveCurrentVideoFrame(m_displayIndexToModelIndex[index], index);
    });
    connect(pDisplay, &CVideoDisplay::doExportCurrentFrame, [&, pDisplay](const QString& path, bool bWithGraphics)
    {
        int index = getDisplayIndex(pDisplay);
        emit doExportCurrentVideoFrame(m_displayIndexToModelIndex[index], index, path, bWithGraphics);
    });
    connect(pDisplay, &CVideoDisplay::doToggleGraphicsToolbar, [&]
    {
        emit doToggleGraphicsToolbar();
    });

    initImageViewConnections(pDisplay->getImageDisplay()->getView());
}

void CDataViewer::initVolumeConnections(CVolumeDisplay *pDisplay)
{
    connect(pDisplay, &CVolumeDisplay::doToggleMaximize, [&]
    {
        emit doToggleMaximize();
    });

    initImageConnections(pDisplay->getImageDisplay());
    init3dDisplayConnections(pDisplay->get3dDisplay());
}

void CDataViewer::init3dDisplayConnections(C3dDisplay *pDisplay)
{
    connect(pDisplay, &C3dDisplay::doUpdateRenderMode, [&](size_t mode){ emit doUpdateRenderMode(mode); });
    connect(pDisplay, &C3dDisplay::doUpdateColormap, [&](size_t colormap){ emit doUpdateColormap(colormap); });
    connect(pDisplay, &C3dDisplay::doUpdateParam, [&](size_t type, float value){ emit doUpdateParam(type, value); });

    connect(pDisplay->getGLWidget(), &CGLWidget::doInitCL, [&](const std::string& name){ emit doInitCL(name); });
    connect(pDisplay->getGLWidget(), &CGLWidget::doInitPBO, [&](GLuint pbo, GLuint pbo_depth){ emit doInitPBO(pbo, pbo_depth); });
    connect(pDisplay->getGLWidget(), &CGLWidget::doRenderVolume, [&](QMatrix4x4& model, QMatrix4x4& view, QMatrix4x4& projection){
        emit doRenderVolume(model, view, projection);
    });
    connect(pDisplay->getGLWidget(), &CGLWidget::doUpdateWindowSize, [&](int width, int height){ emit doUpdateWindowSize(width, height); });
    connect(pDisplay->getGLWidget(), &CGLWidget::doSendErrorMessage, [&](const QString& msg){ emit doSendErrorMessage(msg); });
}

void CDataViewer::initPositionConnections(CPositionDisplay *pDisplay)
{
    initImageConnections(pDisplay->getImageDisplay());
}

void CDataViewer::setActiveWidget(int index)
{
    m_pStacked->setCurrentIndex(index);

    auto displays = m_pDataDisplay->getDataViews();
    for(auto it : displays)
        it->setActive(index == 1);
}

QList<CImageDisplay *> CDataViewer::getImageDisplays() const
{
    QList<CImageDisplay*> imageDisplays;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::IMAGE_DISPLAY)
            imageDisplays.push_back(static_cast<CImageDisplay*>(views[i]));
    }
    return imageDisplays;
}

QList<CImageDisplay *> CDataViewer::getAllImageDisplays() const
{
    QList<CImageDisplay*> imageDisplays;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        auto pImageDisplay = getImageDisplay(views[i]);
        if(pImageDisplay)
            imageDisplays.push_back(pImageDisplay);
    }
    return imageDisplays;
}

CImageDisplay *CDataViewer::getImageDisplay(CDataDisplay *pDisplay) const
{
    if(pDisplay->getTypeId() == DisplayType::IMAGE_DISPLAY)
        return static_cast<CImageDisplay*>(pDisplay);
    else if(pDisplay->getTypeId() == DisplayType::VIDEO_DISPLAY)
        return (static_cast<CVideoDisplay*>(pDisplay))->getImageDisplay();
    else if(pDisplay->getTypeId() == DisplayType::VOLUME_DISPLAY)
        return (static_cast<CVolumeDisplay*>(pDisplay))->getImageDisplay();
    else if(pDisplay->getTypeId() == DisplayType::POSITION_DISPLAY)
            return (static_cast<CPositionDisplay*>(pDisplay))->getImageDisplay();
    else
        return nullptr;
}

QList<CImageView*> CDataViewer::getImageViews() const
{
    QList<CImageView*> imageViews;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        auto pImageView = getImageView(views[i]);
        if(pImageView)
            imageViews.push_back(pImageView);
    }
    return imageViews;
}

CImageView *CDataViewer::getImageView(CDataDisplay *pDisplay) const
{
    if(pDisplay->getTypeId() == DisplayType::IMAGE_DISPLAY)
        return (static_cast<CImageDisplay*>(pDisplay))->getView();
    else if(pDisplay->getTypeId() == DisplayType::VIDEO_DISPLAY)
        return (static_cast<CVideoDisplay*>(pDisplay))->getImageDisplay()->getView();
    else if(pDisplay->getTypeId() == DisplayType::VOLUME_DISPLAY)
        return (static_cast<CVolumeDisplay*>(pDisplay))->getImageDisplay()->getView();
    else if(pDisplay->getTypeId() == DisplayType::POSITION_DISPLAY)
        return (static_cast<CPositionDisplay*>(pDisplay))->getImageDisplay()->getView();
    else
        return nullptr;
}

QList<CVideoDisplay*> CDataViewer::getVideoDisplays() const
{
    QList<CVideoDisplay*> videoDisplays;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::VIDEO_DISPLAY)
            videoDisplays.push_back(static_cast<CVideoDisplay*>(views[i]));
    }
    return videoDisplays;
}

QList<CVolumeDisplay *> CDataViewer::getVolumeDisplays() const
{
    QList<CVolumeDisplay*> volumeDisplays;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::VOLUME_DISPLAY)
            volumeDisplays.push_back(static_cast<CVolumeDisplay*>(views[i]));
    }
    return volumeDisplays;
}

QList<CPositionDisplay *> CDataViewer::getPositionDisplays() const
{
    QList<CPositionDisplay*> positionDisplays;
    auto views = m_pDataDisplay->getDataViews();

    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::POSITION_DISPLAY)
            positionDisplays.push_back(static_cast<CPositionDisplay*>(views[i]));
    }
    return positionDisplays;
}

int CDataViewer::getDisplayIndex(CDataDisplay *pDisplay) const
{
    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
    {
        if(views[i] == pDisplay)
            return i;
    }
    return 0;
}

bool CDataViewer::isItemShownInListView(const QModelIndex &index)
{
    // Get item type by wrapping multimodel index to project model index
    auto pModel = static_cast<const CMultiProjectModel*>(index.model());
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
    auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

    bool bImageSubFolder = false;
    ProjectTreeItem* parentItemPtr = nullptr;
    auto pParentTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.parent().internalPointer());

    if(pParentTreeItem)
    {
        wrapIndex = pModel->wrappedIndex(pParentTreeItem, pTreeItem->m_pModel, index.parent());
        parentItemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
        bImageSubFolder = (itemPtr->getTypeId() == TreeItemType::FOLDER &&
                           parentItemPtr->getTypeId() == TreeItemType::IMAGE);
    }

    return (itemPtr->getTypeId() != TreeItemType::IMAGE &&
            itemPtr->getTypeId() != TreeItemType::GRAPHICS_LAYER &&
            itemPtr->getTypeId() != TreeItemType::RESULT &&
            itemPtr->getTypeId() != TreeItemType::VIDEO &&
            itemPtr->getTypeId() != TreeItemType::LIVE_STREAM &&
            bImageSubFolder == false);
}

bool CDataViewer::canPlayVideo(CVideoDisplay* pDisplay) const
{
    auto videoDisplays = getVideoDisplays();
    for(int i=0; i<videoDisplays.size(); ++i)
    {
        if(videoDisplays[i] != pDisplay && videoDisplays[i]->isPlaying())
            return false;
    }
    return true;
}

CDataDisplay* CDataViewer::getDataDisplay()
{
    return m_pDataDisplay;
}

CDataListView* CDataViewer::getDataListView()
{
    return m_pDataListView;
}

bool CDataViewer::isVideoStream() const
{
    auto views = m_pDataDisplay->getDataViews();
    if(views.size() > 0)
        return (static_cast<CVideoDisplay*>(views[0]))->hasStreamOptions();
    else
        return false;
}

void CDataViewer::setImageViewSync(CImageViewSync *pSync)
{
    m_pImageViewSync = pSync;
}

void CDataViewer::setVideoViewSync(CVideoViewSync *pSync)
{
    m_pVideoViewSync = pSync;
}

void CDataViewer::setVolumeViewSync(CVolumeViewSync *pSync)
{
    m_pVolumeViewSync = pSync;
}

void CDataViewer::fillZoomProperties(CViewPropertyIO *pViewProp)
{
    if(pViewProp == nullptr)
        return;

    auto displays = getAllImageDisplays();
    if(displays.size() == 0)
        return;

    CImageView* pView = displays[0]->getView();
    assert(pView);

    if(pView->isZoomFit())
        pViewProp->setZoomFit(true);
    else
        pViewProp->setZoomScale(pView->getScale());

    //Position informations
    auto center = pView->mapToScene(pView->viewport()->rect().center());
    pViewProp->setZoomPosition(pView->getNumScheduledScalings(), pView->getTargetScenePos(), center);
}

void CDataViewer::addDataView(CDataDisplay* pData, int r, int c)
{
    m_pDataDisplay->addDataView(pData, r, c);
}

void CDataViewer::addGraphicsLayer(const CGraphicsLayerInfo& layerInfo)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->addGraphicsLayer(layerInfo.m_pLayer, layerInfo.m_bTopMost);
}

void CDataViewer::removeGraphicsLayer(const CGraphicsLayerInfo& layerInfo, bool bDelete)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->removeGraphicsLayer(layerInfo.m_pLayer, bDelete);
}

void CDataViewer::updateDataListViewIndex(const QModelIndex& index)
{
    if(index.isValid())
    {
        if(isItemShownInListView(index))
            displayListView();

        m_pPathNavigator->addPath(index);
        m_pDataListView->updateIndex(index);
    }
    else
        m_pPathNavigator->clear();
}

void CDataViewer::updateDataListViewModel(QAbstractItemModel* pModel)
{
    m_pDataListView->setSourceModel(pModel);
}

void CDataViewer::updateVolumeImage(int index, QImage image, const QString &name, CViewPropertyIO *pViewProp)
{
    //At the current development point, we manage only 1 volume display
    index = 0;
    auto volumeDisplays = getVolumeDisplays();
    assert(volumeDisplays.size() == 1);

    auto pImageDisplay = volumeDisplays[index]->getImageDisplay();
    if(pImageDisplay == nullptr)
        return;

    pImageDisplay->setName(name);
    //Force zoom fit if there is no protocol view preferences
    bool bForceZoomFit = (pViewProp == nullptr);
    pImageDisplay->setImage(image, bForceZoomFit);
    pImageDisplay->setViewProperty(pViewProp);
}

void CDataViewer::initDisplay(const std::map<DisplayType, int> &mapTypeCount)
{
    if(mapTypeCount.empty())
        return;

    DisplayType type;
    QList<CDataDisplay*> displaysToRemove;
    std::map<DisplayType, int> displayCountPerType;

    //List all data displays
    for(auto&& it : m_pDataDisplay->getDataViews())
    {
        type = static_cast<DisplayType>(it->getTypeId());

        //Fill the map to count existing data displays per type
        auto itDisplayType = displayCountPerType.find(type);
        if(itDisplayType == displayCountPerType.end())
            displayCountPerType[type] = 1;
        else
            itDisplayType->second++;

        //Store all data displays to remove in a vector
        auto itType = mapTypeCount.find(type);
        if(itType == mapTypeCount.end())
        {
            //The display of this type is not useful anymore
            displaysToRemove.push_back(it);
        }
        else if(displayCountPerType[type] > itType->second)
        {
            //The number of displays for this type is reached
            displaysToRemove.push_back(it);
        }
    }

    for(int i=0; i<displaysToRemove.size(); ++i)
    {
        //Unsynchronize views
        type = static_cast<DisplayType>(displaysToRemove[i]->getTypeId());
        switch(type)
        {
            case DisplayType::IMAGE_DISPLAY:
                if(m_pImageViewSync)
                    m_pImageViewSync->unsyncView(static_cast<CImageDisplay*>(displaysToRemove[i]));
                break;
            case DisplayType::VIDEO_DISPLAY:
                if(m_pVideoViewSync)
                    m_pVideoViewSync->unsyncView(static_cast<CVideoDisplay*>(displaysToRemove[i]));
                break;

            default: break;
        }
    }

    //Remove unnecessary data displays
    m_pDataDisplay->removeDataViews(displaysToRemove, true);
}

void CDataViewer::displayListView()
{
    setActiveWidget(0);
    emit doDisplayListView();
}

void CDataViewer::displayImage(int index, CImageScene *pScene, QImage image, QString name, CViewPropertyIO* pViewProp)
{
    CImageDisplay* pDisplay = nullptr;
    auto displays = m_pDataDisplay->getDataViews();

    if(index >= displays.size())
    {
        pDisplay = createImageDisplay();
        m_pDataDisplay->addDataView(pDisplay);
    }
    else
    {
        if(displays[index]->getTypeId() == DisplayType::IMAGE_DISPLAY)
            pDisplay = static_cast<CImageDisplay*>(displays[index]);
        else
        {
            if(displays[index]->getTypeId() == DisplayType::VIDEO_DISPLAY && m_pVideoViewSync)
                m_pVideoViewSync->unsyncView(static_cast<CVideoDisplay*>(displays[index]));

            pDisplay = createImageDisplay();
            m_pDataDisplay->replaceDataView(displays[index], pDisplay);
            displays[index]->onClose();
        }
    }
    //Set main data display visible
    setActiveWidget(1);
    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->setViewProperty(pViewProp);
    pDisplay->setSelected(true);
    pDisplay->show();
    //Force zoom fit if there is no protocol view preferences
    bool bForceZoomFit = (pViewProp == nullptr);
    pDisplay->setImage(pScene, image, bForceZoomFit);
    pDisplay->setName(name);
}

void CDataViewer::displayVideo(const QModelIndex &modelIndex, int displayIndex, CImageScene *pScene, QImage image, QString name, bool bVideoChanged, CViewPropertyIO *pViewProp)
{
    m_modelIndexToDisplayIndex[modelIndex] = displayIndex;
    m_displayIndexToModelIndex[displayIndex] = modelIndex;

    CVideoDisplay* pDisplay = nullptr;
    auto displays = m_pDataDisplay->getDataViews();

    if(displayIndex >= displays.size())
    {
        pDisplay = createVideoDisplay();
        m_pDataDisplay->addDataView(pDisplay);
    }
    else
    {
        if(displays[displayIndex]->getTypeId() == DisplayType::VIDEO_DISPLAY)
            pDisplay = static_cast<CVideoDisplay*>(displays[displayIndex]);
        else
        {
            if(displays[displayIndex]->getTypeId() == DisplayType::IMAGE_DISPLAY && m_pImageViewSync)
                m_pImageViewSync->unsyncView(static_cast<CImageDisplay*>(displays[displayIndex]));

            pDisplay = createVideoDisplay();
            m_pDataDisplay->replaceDataView(displays[displayIndex], pDisplay);
            displays[displayIndex]->onClose();
        }
    }
    pDisplay->setViewProperty(pViewProp);
    //Set main data display visible
    setActiveWidget(1);
    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->show();    
    //Force zoom fit if it's a new video
    pDisplay->setImage(pScene, image, name, bVideoChanged);

    if(bVideoChanged)
        pDisplay->setSelected(true);
}

void CDataViewer::displayVolume(CImageScene *pScene, QImage image, QString name, bool bStackHasChanged, CViewPropertyIO* pViewProp)
{
    CVolumeDisplay* pDisplay = nullptr;
    auto displays = getVolumeDisplays();

    if(displays.size() > 0)
        pDisplay = displays[0];
    else
    {
        m_pDataDisplay->removeAll();
        pDisplay = createVolumeDisplay();
        m_pDataDisplay->addDataView(pDisplay);

        //We must initialize OpenGL and OpenCL after signal/slot connections
        //and after volume display is added to the data display
        //From Qt 5.14 virtual function initializeGL is called when QOpenGLWidget
        //is added to layout so we have to delay initCL call
        pDisplay->initGL();
    }

    //Set main data display visible
    setActiveWidget(1);
    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->setViewProperty(pViewProp);
    pDisplay->setSelected(true);
    pDisplay->show();
    pDisplay->setImage(pScene, image, name);

    if(bStackHasChanged)
        pDisplay->changeVolume();
}

void CDataViewer::displayPosition(CImageScene *pScene, QImage image, QString name, bool bStackHasChanged, CViewPropertyIO* pViewProp)
{
    CPositionDisplay* pDisplay = nullptr;
    auto displays = getPositionDisplays();

    if(displays.size() > 0)
        pDisplay = displays[0];
    else
    {
        m_pDataDisplay->removeAll();
        pDisplay = createPositionDisplay();
        m_pDataDisplay->addDataView(pDisplay);
    }

    //Set main data display visible
    setActiveWidget(1);
    //Set image after making the display visible to have a good fit in view behaviour
    pDisplay->setViewProperty(pViewProp);
    pDisplay->setSelected(true);
    pDisplay->show();
    pDisplay->setImage(pScene, image, name);
}

void CDataViewer::switchView()
{
    if(m_pStacked->currentIndex() == 0)
        setActiveWidget(1);
    else
        displayListView();
}

void CDataViewer::notifySceneDeleted()
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->notifySceneDeleted();
}

void CDataViewer::notifyResultViewVisible(bool bVisible)
{
    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
        views[i]->setViewSpaceShared(bVisible);
}

void CDataViewer::syncImageView(CImageDisplay *pDisplay)
{
    //The given CImageDisplay can be sync with either a CImageDisplay or a CVolumeDisplay

    //Get image displays
    if(m_pImageViewSync != nullptr)
    {
        auto imageDisplays = getImageDisplays();
        for(int i=0; i<imageDisplays.size(); ++i)
            m_pImageViewSync->syncView(imageDisplays[i], pDisplay);
    }

    //Get volume displays
    if(m_pVolumeViewSync != nullptr)
    {
        auto volumeDisplays = getVolumeDisplays();
        for(int i=0; i<volumeDisplays.size(); ++i)
            m_pVolumeViewSync->syncView(volumeDisplays[i], pDisplay);
    }
}

void CDataViewer::syncVideoView(CVideoDisplay *pDisplay)
{
    if(m_pVideoViewSync == nullptr)
        return;

    auto videoDisplays = getVideoDisplays();
    for(int i=0; i<videoDisplays.size(); ++i)
    {        
        if(pDisplay != videoDisplays[i])
        {
            m_pVideoViewSync->syncView(videoDisplays[i], pDisplay);
            videoDisplays[i]->emitVideoInfo();
        }
    }
}

void CDataViewer::syncVideoView(CVideoDisplay *pDisplay, const std::vector<int> &syncToIndices)
{
    if(m_pVideoViewSync == nullptr)
        return;

    //Clear previous connections
    unsyncVideoView(pDisplay);

    //Set new ones
    auto videoDisplays = getVideoDisplays();
    for(size_t i=0; i<syncToIndices.size(); ++i)
    {
        if(syncToIndices[i] < videoDisplays.size())
            m_pVideoViewSync->syncView(videoDisplays[syncToIndices[i]], pDisplay);
    }
}

void CDataViewer::unsyncView(CDataDisplay *pDisplay)
{
    switch(pDisplay->getTypeId())
    {
        case DisplayType::IMAGE_DISPLAY:
            unsyncImageView(static_cast<CImageDisplay*>(pDisplay));
            break;
        case DisplayType::VIDEO_DISPLAY:
            unsyncVideoView(static_cast<CVideoDisplay*>(pDisplay));
            break;
    }
}

void CDataViewer::unsyncImageView(CImageDisplay *pDisplay)
{
    //The given CImageDisplay must be unsync with CImageDisplay or CVolumeDisplay
    if(pDisplay == nullptr)
        return;

    if(m_pImageViewSync != nullptr)
    {
        auto imageDisplays = getImageDisplays();
        for(int i=0; i<imageDisplays.size(); ++i)
            m_pImageViewSync->unsyncView(imageDisplays[i], pDisplay);
    }

    if(m_pVolumeViewSync != nullptr)
    {
        auto volumeDisplays = getVolumeDisplays();
        for(int i=0; i<volumeDisplays.size(); ++i)
            m_pVolumeViewSync->unsyncView(volumeDisplays[i], pDisplay);
    }
}

void CDataViewer::unsyncVideoView(CVideoDisplay *pDisplay)
{
    if(m_pVideoViewSync == nullptr || pDisplay == nullptr)
        return;

    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::VIDEO_DISPLAY)
            m_pVideoViewSync->unsyncView(static_cast<CVideoDisplay*>(views[i]), pDisplay);
    }
}

void CDataViewer::clearAll()
{
    m_pDataDisplay->removeAll();
    auto pEmptyDysplay = createEmptyDisplay();
    m_pDataDisplay->addDataView(pEmptyDysplay);
    setActiveWidget(1);
}

void CDataViewer::onDisplayVolume(const QModelIndex& index)
{
    emit doDisplayImage(index);
}

void CDataViewer::onDisplayBinaryOverlay(const QImage& image)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->setOverlayImage(image);
}

void CDataViewer::onClearBinaryOverlay()
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->clearOverlay();
}

void CDataViewer::onSwitchView()
{
    switchView();
}

void CDataViewer::onUpdateIndex(const QModelIndex& index)
{
    emit doUpdateIndex(index);
}

void CDataViewer::onUpdateNbImage(int nb)
{
    auto imageDisplays = getAllImageDisplays();
    for(int i=0; i<imageDisplays.size(); ++i)
        imageDisplays[i]->setSliderRange(nb);
}

void CDataViewer::onUpdateCurrentImageIndex(int index)
{
    auto imageDisplays = getAllImageDisplays();
    for(int i=0; i<imageDisplays.size(); ++i)
        imageDisplays[i]->setCurrentImgIndex(index);
}

void CDataViewer::onUpdateRenderVolume()
{
    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::VOLUME_DISPLAY)
            (static_cast<CVolumeDisplay*>(views[i]))->updateRenderVolume();
    }
}

void CDataViewer::onEnableRender(bool bEnable)
{
    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
    {
        if(views[i]->getTypeId() == DisplayType::VOLUME_DISPLAY)
            (static_cast<CVolumeDisplay*>(views[i]))->get3dDisplay()->getGLWidget()->activateRender(bEnable);
    }
}

void CDataViewer::onChangeGraphicsActivationState(bool bActivate)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->activateGraphics(bActivate);
}

void CDataViewer::onSetCurrentGraphicsLayer(CGraphicsLayer *pLayer)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->setCurrentGraphicsLayer(pLayer);
}

void CDataViewer::onAddGraphicsItem(QGraphicsItem *pItem, bool bForceParent)
{
    auto imageViews = getImageViews();
    for(int i=0; i<imageViews.size(); ++i)
        imageViews[i]->addGraphicsItem(pItem, bForceParent);
}

void CDataViewer::onSetVideoSliderLength(int index, size_t length)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetSliderLength(length);
        m_pVideoViewSync->syncSliderLength(pDisplay, length);
    }
}

void CDataViewer::onSetVideoSliderPos(int index, size_t pos)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetSliderPos(pos);
        m_pVideoViewSync->syncSliderPos(pDisplay, pos);
    }
}

void CDataViewer::onSetVideoFPS(int index, double fps)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetFPS(fps);
        m_pVideoViewSync->syncFps(pDisplay, fps);
    }
}

void CDataViewer::onSetVideoTotalTime(int index, size_t totalTime)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetTotalTime(totalTime);
        m_pVideoViewSync->syncTotalTime(pDisplay, totalTime);
    }
}

void CDataViewer::onSetVideoCurrentTime(int index, size_t currentTime)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->onSetCurrentTime(currentTime);
        m_pVideoViewSync->syncCurrentTime(pDisplay, currentTime);
    }
}

void CDataViewer::onSetVideoSourceType(int index, CDataVideoBuffer::Type srcType)
{
    auto views = m_pDataDisplay->getDataViews();
    if(index < views.size() && views[index]->getTypeId() == DisplayType::VIDEO_DISPLAY)
    {
        auto pDisplay = static_cast<CVideoDisplay*>(views[index]);
        pDisplay->setSourceType(srcType);
        m_pVideoViewSync->syncSourceType(pDisplay, srcType);
    }
}

void CDataViewer::onStopVideo()
{
    auto videoDisplays = getVideoDisplays();
    for(int i=0; i<videoDisplays.size(); ++i)
        videoDisplays[i]->onStopVideo();
}

void CDataViewer::onStopRecordingVideo(const QModelIndex& modelIndex)
{
    auto videoDisplays = getVideoDisplays();
    int index = m_modelIndexToDisplayIndex[modelIndex];

    if(index < videoDisplays.size())
        videoDisplays[index]->onRecordVideo();
}

void CDataViewer::onStopVideoPlayer(const QModelIndex &modelIndex)
{
    auto videoDisplays = getVideoDisplays();
    int index = m_modelIndexToDisplayIndex[modelIndex];

    if(index < videoDisplays.size())
        videoDisplays[index]->stopPlayer();
}

void CDataViewer::onApplyViewProperty()
{
    auto views = m_pDataDisplay->getDataViews();
    for(int i=0; i<views.size(); ++i)
        views[i]->applyViewProperty();
}

void CDataViewer::onDataViewToggleMaximize(CDataDisplay *pData)
{
    if(m_pDataDisplay->getDataViewCount() == 1)
        emit doToggleMaximize();
    else
        m_pDataDisplay->toggleMaximizeView(pData);
}

#include "moc_CDataViewer.cpp"
