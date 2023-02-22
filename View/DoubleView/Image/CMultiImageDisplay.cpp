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

#include "CMultiImageDisplay.h"
#include "Model/Data/CMultiImageModel.h"
#include "View/DoubleView/Image/CImageDisplay.h"
#include "Data/CDataConversion.h"
#include "DataProcessTools.hpp"
#include "CImageDataManager.h"

CMultiImageDisplay::CMultiImageDisplay(QWidget* pParent) : CDataDisplay(pParent, "", CDataDisplay::NONE)
{
    initLayout();
    initConnections();
    initOverlayColors();
    m_typeId = DisplayType::MULTI_IMAGE_DISPLAY;
}

void CMultiImageDisplay::initLayout()
{
    m_pListView = new CImageListView(this);

    m_pImageDisplay = new CImageDisplay(this, "", CImageDisplay::DEFAULT);
    m_pImageDisplay->enableImageMoveByKey(false);

    m_pStackedWidget = new QStackedWidget;
    m_pStackedWidget->addWidget(m_pListView);
    m_pStackedWidget->addWidget(m_pImageDisplay);
    m_pStackedWidget->setCurrentIndex(0);

    m_pLayout->addWidget(m_pStackedWidget);
}

void CMultiImageDisplay::initConnections()
{
    connect(m_pListView, &CImageListView::doubleClicked, this, &CMultiImageDisplay::onShowImage);
    connect(m_pImageDisplay, &CImageDisplay::doDoubleClicked, [&]{ m_pStackedWidget->setCurrentIndex(0); });
    connect(m_pImageDisplay, &CImageDisplay::doExport, this, &CMultiImageDisplay::onExportImage);
}

void CMultiImageDisplay::initOverlayColors()
{
    std::srand(RANDOM_COLOR_SEED);
    m_overlayColormap = CMat(256, 1, CV_8UC3, cv::Scalar(0));

    //Random colors
    for(int i=1; i<256; ++i)
    {
        for(int j=0; j<3; ++j)
            m_overlayColormap.at<cv::Vec3b>(i, 0)[j] = (uchar)((double)std::rand() / (double)RAND_MAX * 255.0);
    }
}

void CMultiImageDisplay::loadImage(const QModelIndex& index)
{
    if(index.isValid() == false)
        return;

    try
    {
        int imgIndex = index.row();
        auto pModel = static_cast<const CMultiImageModel*>(index.model());
        m_currentIndex = index;

        //Image
        CImageDataIO io(pModel->getPath(imgIndex).toStdString());
        CMat img = io.read();
        m_pImageDisplay->setImage(CDataConversion::CMatToQImage(img), true);
        m_pImageDisplay->setName(pModel->getFileName(imgIndex));

        //Graphics
        if(m_pCurrentLayer)
            m_pImageDisplay->getView()->removeGraphicsLayer(m_pCurrentLayer, false);

        m_pCurrentLayer = pModel->getGraphicsLayer(imgIndex);
        m_pImageDisplay->getView()->addGraphicsLayer(m_pCurrentLayer, true);

        //Overlay
        auto overlayPath = pModel->getOverlayPath(imgIndex);
        if(!overlayPath.isEmpty())
        {
            CImageDataIO ioOvr(overlayPath.toStdString());
            CMat img = ioOvr.read();
            CMat ovrImg = createDisplayOverlay(img);
            m_pImageDisplay->getView()->setOverlayImage(CDataConversion::CMatToQImage(ovrImg));
        }
        else
            m_pImageDisplay->getView()->clearOverlay();

        m_pStackedWidget->setCurrentIndex(1);
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString::fromStdString(e.what());
    }
}

void CMultiImageDisplay::loadPreviousImage()
{
    QModelIndex current = m_pListView->currentIndex();
    int imgIndex = current.row() - 1;
    QModelIndex previous = current.siblingAtRow(imgIndex);

    if(previous.isValid())
    {
        loadImage(previous);
        m_pListView->setCurrentIndex(previous);
    }
}

void CMultiImageDisplay::loadNextImage()
{
    QModelIndex current = m_pListView->currentIndex();
    int imgIndex = current.row() + 1;
    QModelIndex next = current.siblingAtRow(imgIndex);

    if(next.isValid())
    {
        loadImage(next);
        m_pListView->setCurrentIndex(next);
    }
}

CMat CMultiImageDisplay::createDisplayOverlay(const CMat &img)
{
    CMat ovrImg = Utils::Image::createOverlayMask(img, m_overlayColormap);
    return ovrImg;
}

void CMultiImageDisplay::setModel(CMultiImageModel *pModel)
{
    m_pListView->setModel(pModel);
    m_pStackedWidget->setCurrentIndex(0);
}

void CMultiImageDisplay::onShowImage(const QModelIndex &index)
{
    loadImage(index);
}

void CMultiImageDisplay::onExportImage(const QString &path, bool bWithGraphics)
{
    if(!m_currentIndex.isValid())
        return;

    auto pView = m_pImageDisplay->getView();
    if(!pView)
        return;

    try
    {
        int imgIndex = m_currentIndex.row();
        auto pModel = static_cast<const CMultiImageModel*>(m_currentIndex.model());

        //Image
        CImageDataIO io(pModel->getPath(imgIndex).toStdString());
        CMat img = io.read();

        //Mask
        CMat mask;
        auto maskPath = pModel->getOverlayPath(imgIndex);
        if(!maskPath.isEmpty())
        {
            CImageDataIO ioMask(maskPath.toStdString());
            mask = ioMask.read();
            img = Utils::Image::mergeColorMask(img, mask, m_overlayColormap, 0.7, true);
        }

        if(bWithGraphics)
            emit doExportImage(path, img, m_pCurrentLayer);
        else
            emit doExportImage(path, img, nullptr);
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString::fromStdString(e.what());
    }
}

void CMultiImageDisplay::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Left)
        loadPreviousImage();
    else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Right)
        loadNextImage();
    else
        CDataDisplay::keyPressEvent(event);
}
