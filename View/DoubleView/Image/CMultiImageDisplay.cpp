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
    m_typeId = DisplayType::MULTI_IMAGE_DISPLAY;
}

void CMultiImageDisplay::initLayout()
{
    m_pListView = new CImageListView(this);

    m_pImageDisplay = new CImageDisplay(this);
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
}

void CMultiImageDisplay::loadImage(const QModelIndex& index)
{
    if(index.isValid() == false)
        return;

    try
    {
        int imgIndex = index.row();
        auto pModel = static_cast<const CMultiImageModel*>(index.model());

        //Image
        CImageIO io(pModel->getPath(imgIndex).toStdString());
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
            CImageIO ioOvr(overlayPath.toStdString());
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
    //Random colors
    CMat colormap(256, 1, CV_8UC3, cv::Scalar(0));
    std::srand(std::time(nullptr));

    for(int i=1; i<256; ++i)
    {
        for(int j=0; j<3; ++j)
            colormap.at<cv::Vec3b>(i, 0)[j] = (uchar)((double)std::rand() / (double)RAND_MAX * 255.0);
    }

    CMat ovrImg = Utils::Image::createOverlayMask(img, colormap);
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

void CMultiImageDisplay::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Left)
        loadPreviousImage();
    else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_Right)
        loadNextImage();
    else
        CDataDisplay::keyPressEvent(event);
}
