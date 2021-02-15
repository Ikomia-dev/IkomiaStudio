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

#include "CProtocolIOArea.h"
#include <QToolButton>
#include "CProtocolPortItem.h"
#include "CProtocolInputTypeDlg.h"
#include "CProtocolScene.h"
#include "CGraphicsDeletableButton.h"
#include "View/Common/CSvgButton.h"
#include "View/Common/CSvgButton.h"
#include <QtSvg>

CProtocolIOArea::CProtocolIOArea(QString label, bool bInput, QGraphicsItem *parent) : CProtocolLabelArea(label, parent)
{
    m_bInput = bInput;
    m_id = boost::graph_traits<ProtocolGraph>::null_vertex();
    setAcceptHoverEvents(true);
    setZValue(1.0);
}

void CProtocolIOArea::setTaskId(const ProtocolVertex &id)
{
    m_id = id;
}

void CProtocolIOArea::setProjectDataProxyModel(CProjectDataProxyModel *pModel)
{
    if(m_pInputChoiceDlg)
        m_pInputChoiceDlg->setModel(pModel);
}

void CProtocolIOArea::setPortInfo(const CDataInfoPtr &info, int index)
{
    if(index < 0 || index >= m_ports.size())
        return;

    m_ports[index]->setInfo(info);
}

int CProtocolIOArea::type() const
{
    return Type;
}

int CProtocolIOArea::getPortCount() const
{
    return m_ports.size();
}

int CProtocolIOArea::getConnectionCount() const
{
    int nb = 0;
    for(int i=0; i<m_ports.size(); ++i)
        nb += m_ports[i]->getConnectionCount();

    return nb;
}

CProtocolPortItem *CProtocolIOArea::getPort(int index) const
{
    if(index >= 0 && index < m_ports.size())
        return m_ports[index];
    else
        return nullptr;
}

bool CProtocolIOArea::isInput() const
{
    return m_bInput;
}

void CProtocolIOArea::activateActions(bool bActivate)
{
    m_bActionsActivated = bActivate;
}

void CProtocolIOArea::addPort(const ProtocolTaskIOPtr& inputPtr, bool isBatch)
{
    bool bInput = !m_bInput;
    QPointF center = getNextPortCenter();

    //Add port
    qreal xPort;
    QRectF rect = boundingRect();

    if(m_bInput)
        xPort = rect.right();
    else
        xPort = rect.left();

    QColor portColor = Qt::white;
    IODataType type = IODataType::NONE;

    if(inputPtr != nullptr)
    {
        type = inputPtr->getDataType();
        portColor = Utils::Protocol::getPortColor(type);
    }

    auto pPort = new CProtocolPortItem(m_ports.size(), bInput, m_id, type, portColor, this);
    pPort->setPos(xPort, center.y());
    m_ports.push_back(pPort);

    //Add thumbnail associated with the port
    auto pThumbnail = createThumbnail(inputPtr, isBatch);
    auto iconSize = static_cast<QToolButton*>(pThumbnail->widget())->iconSize();
    pThumbnail->setPos(center.x() - iconSize.width()/2, center.y() - iconSize.height()/2);
    pThumbnail->show();
    m_portThumbnails.push_back(pThumbnail);

    auto nextPos = getNextPortCenter();
    if(nextPos.ry() > m_size.height())
    {
        m_size.setHeight(m_size.height()+m_thumbnailSize.height() + m_portSpacing*2);
        emit doIOAreaChanged();
    }
}

void CProtocolIOArea::updatePort(int index, const ProtocolTaskIOPtr &inputPtr, bool isBatch)
{
    if(index < 0 || index >= m_ports.size())
        return;

    //Update port
    QColor portColor = Qt::white;
    IODataType type = IODataType::NONE;

    if(inputPtr != nullptr)
    {
        type = inputPtr->getDataType();
        portColor = Utils::Protocol::getPortColor(type);
    }

    m_ports[index]->setColor(portColor);
    m_ports[index]->setDataType(type);
    m_ports[index]->updateConnections();

    //Update port thumbnail
    auto pBtn = static_cast<QToolButton*>(m_portThumbnails[index]->widget());
    auto pixmap = getThumbnailIcon(inputPtr, isBatch);
    pBtn->setIcon(QIcon(pixmap));

    // Calculate new position depending on the  thumbnail size
    QPointF center;
    QRectF rect = boundingRect();
    qreal portAreaHeight = m_thumbnailSize.height() + (2 * m_portSpacing);
    center.setX(rect.center().x());
    center.setY(m_topContent + m_portSpacing + (index * portAreaHeight) + portAreaHeight/2.0);
    m_portThumbnails[index]->setPos(center.x() - pixmap.width()/2, center.y() - pixmap.height()/2);
}

void CProtocolIOArea::removePort(int index)
{
    auto pScene = static_cast<CProtocolScene*>(scene());
    auto connections = m_ports[index]->getConnections();

    for(int j=0; j<connections.size(); ++j)
        pScene->deleteConnection(connections[j], false, false, false);

    pScene->removeItem(m_ports[index]);
    pScene->removeItem(m_portThumbnails[index]);
    delete m_ports[index];
    //We can enter this function from the delete button of thumbnails
    //so we have to take care of pending message and call deleteLater instead of delete
    m_portThumbnails[index]->deleteLater();
    m_ports.erase(m_ports.begin() + index);
    m_portThumbnails.erase(m_portThumbnails.begin() + index);

    //Update positions
    qreal portAreaHeight = m_thumbnailSize.height() + (2 * m_portSpacing);
    qreal halfThumbnailSize = m_thumbnailSize.height()/2.0;

    for(int i=index; i<m_ports.size(); ++i)
    {
        auto portPos = m_ports[i]->pos();
        auto thumbnailPos = m_portThumbnails[i]->pos();
        qreal y = m_topContent + m_portSpacing + (i * portAreaHeight) + portAreaHeight/2.0;
        m_ports[i]->setPos(portPos.x(), y);
        m_ports[i]->setIndex((size_t)i);
        m_ports[i]->updateConnections();
        m_portThumbnails[i]->setPos(thumbnailPos.x(), y - halfThumbnailSize);
    }

    // Reduce IOArea size
    m_size.setHeight(m_size.height() - m_thumbnailSize.height() - m_portSpacing*2);
    emit doIOAreaChanged();
}

void CProtocolIOArea::clear()
{
    auto pScene = static_cast<CProtocolScene*>(scene());
    for(int i=0; i<m_ports.size(); ++i)
    {
        auto connections = m_ports[i]->getConnections();
        for(int j=0; j<connections.size(); ++j)
            pScene->deleteConnection(connections[j], false, false, false);

        pScene->removeItem(m_ports[i]);
        delete m_ports[i];
    }
    m_ports.clear();
}

void CProtocolIOArea::onAddPort()
{
    createInputTypeDlg();

    if(m_pInputChoiceDlg->exec() == QDialog::Accepted)
    {
        CProtocolInput input = m_pInputChoiceDlg->getInput();
        if(input.isValid())
        {
            auto pScene = static_cast<CProtocolScene*>(scene());
            emit pScene->doAddInput(input);
        }
    }
    delete m_pInputChoiceDlg;
    m_pInputChoiceDlg = nullptr;
}

void CProtocolIOArea::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if(m_bActionsActivated)
        showAddPortWidget();
}

void CProtocolIOArea::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if(m_bActionsActivated)
        hideAddPortWidget();
}

QGraphicsProxyWidget* CProtocolIOArea::createThumbnail(const std::shared_ptr<CProtocolTaskIO> &inputPtr, bool isBatch)
{
    auto pixmap = getThumbnailIcon(inputPtr, isBatch);
    auto pBtn = new QToolButton;
    pBtn->setIcon(QIcon(pixmap));
    pBtn->setIconSize(m_thumbnailSize);

    QString strStyleSheet = "border: none; background: transparent; padding-left: 0px; padding-right: 0px; padding-top: 0px; padding-bottom: 0px;";
    pBtn->setStyleSheet(strStyleSheet);

    if(m_portThumbnails.size() > 0)
    {
        auto pThumbnail = new CGraphicsDeletableButton(this);
        pThumbnail->setWidget(pBtn);

        connect(pBtn, &QToolButton::clicked, [this, pThumbnail]
        {
            auto index = getThumbnailIndex(pThumbnail);
            if(index != -1)
                changeInput(index);
        });

        connect(pThumbnail, &CGraphicsDeletableButton::doDelete, [this, pThumbnail]
        {
            auto index = getThumbnailIndex(pThumbnail);
            if(index != -1)
                deleteInput(index);
        });
        return pThumbnail;
    }
    else
    {
        auto pThumbnail = new QGraphicsProxyWidget(this);
        pThumbnail->setWidget(pBtn);

        connect(pBtn, &QToolButton::clicked, [this, pThumbnail]
        {
            //First input is reserved to current data
            auto index = getThumbnailIndex(pThumbnail);
            if(index > 0)
                changeInput(index);
        });
        return pThumbnail;
    }
}

void CProtocolIOArea::createInputTypeDlg()
{
    m_pInputChoiceDlg = new CProtocolInputTypeDlg;
    connect(m_pInputChoiceDlg, &CProtocolInputTypeDlg::doQueryProjectDataProxyModel, [&](const TreeItemType& inputType, const std::vector<DataDimension>& filters)
    {
        auto pScene = static_cast<CProtocolScene*>(scene());
        pScene->queryProjectDataProxyModel(inputType, filters);
    });
}

QPointF CProtocolIOArea::getNextPortCenter() const
{
    QPointF center;
    QRectF rect = boundingRect();
    qreal portAreaHeight = m_thumbnailSize.height() + (2 * m_portSpacing);
    center.setX(rect.center().x());
    center.setY(m_topContent + m_portSpacing + (m_ports.size() * portAreaHeight) + portAreaHeight/2.0);
    return center;
}

QPixmap CProtocolIOArea::getThumbnailIcon(const std::shared_ptr<CProtocolTaskIO> &inputPtr, bool isBatch)
{
    if(inputPtr == nullptr)
        return QPixmap(":/Images/help.png").scaled(m_thumbnailSize);

    switch(inputPtr->getDataType())
    {
        case IODataType::IMAGE:
        case IODataType::IMAGE_LABEL:
        case IODataType::IMAGE_BINARY:
        case IODataType::VOLUME:
        case IODataType::VOLUME_LABEL:
        case IODataType::VOLUME_BINARY:
        case IODataType::VIDEO:
        case IODataType::VIDEO_LABEL:
        case IODataType::VIDEO_BINARY:
            return getImageIcon(std::dynamic_pointer_cast<CImageProcessIO>(inputPtr), isBatch);

        case IODataType::LIVE_STREAM:
        case IODataType::LIVE_STREAM_LABEL:
        case IODataType::LIVE_STREAM_BINARY:
            return QPixmap(":/Images/cam.png").scaled(m_thumbnailSize);

        case IODataType::FOLDER:
        case IODataType::FOLDER_PATH:
            return QPixmap(":/Images/folder.png").scaled(m_thumbnailSize);

        case IODataType::FILE_PATH:
            return QPixmap(":/Images/project.png").scaled(m_thumbnailSize);

        case IODataType::NONE:
        case IODataType::BLOB_VALUES:
        case IODataType::NUMERIC_VALUES:
        case IODataType::INPUT_GRAPHICS:
        case IODataType::OUTPUT_GRAPHICS:
        case IODataType::WIDGET:
        case IODataType::DESCRIPTORS:
            return QPixmap(":/Images/help.png").scaled(m_thumbnailSize);
    }
    return QPixmap();
}

QPixmap CProtocolIOArea::getImageIcon(const std::shared_ptr<CImageProcessIO> &inputImgPtr, bool isBatch)
{
    if(inputImgPtr == nullptr)
        return QPixmap(":/Images/help.png").scaled(m_thumbnailSize);

    CMat img = inputImgPtr->getImage();
    if(img.empty() == true)
        return QPixmap(":/Images/help.png").scaled(m_thumbnailSize);

    if(isBatch)
    {
        //Resource image: 128 x 128
        QImage back(":/Images/image-batch.png");
        back = back.scaled(m_thumbnailSize);

        //Draw input image on background image (hard coded position and size)
        float rw = (float)(m_thumbnailSize.width()) / 128.0;
        float rh = (float)(m_thumbnailSize.height()) / 128.0;
        float x = 10 * rw;
        float y = 22 * rh;
        int w = (int)(76 * rw);
        int h = (int)(50 * rh);

        CMat smallImg;
        cv::resize(img, smallImg, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
        QPainter painter(&back);
        painter.drawImage(QPointF(x, y), CDataConversion::CMatToQImage(smallImg));
        return QPixmap::fromImage(back);
    }
    else
    {
        CMat smallImg;
        auto rw = (float)img.cols/(float)img.rows;
        auto rh = 1.0f/rw;
        auto ratioSize = QSize(std::min(m_thumbnailSize.width(), (int)(m_thumbnailSize.width()*rw)), std::min(m_thumbnailSize.height(), int(m_thumbnailSize.height()*rh)));
        cv::resize(img, smallImg, cv::Size(ratioSize.width(), ratioSize.height()), 0, 0, cv::INTER_LINEAR);
        QImage qImg = CDataConversion::CMatToQImage(smallImg);
        return QPixmap::fromImage(qImg);
    }
}

int CProtocolIOArea::getThumbnailIndex(QGraphicsProxyWidget *pThumbnail)
{
    if(pThumbnail == nullptr)
        return -1;

    for(int i=0; i<m_portThumbnails.size(); ++i)
        if(m_portThumbnails[i] == pThumbnail)
            return i;

    return -1;
}

QString CProtocolIOArea::createSvgThumbnails(QPixmap pixmap, int index)
{
    QSize pixSize(128, 128);
    QSvgGenerator generator;
    QDir().mkpath(Utils::IkomiaApp::getQAppFolder() + "/tmp");
    auto path = QString(Utils::IkomiaApp::getQAppFolder() + "/tmp/tmp%1.svg").arg(index);
    auto newPix = pixmap.scaled(pixSize, Qt::KeepAspectRatio);
    generator.setFileName(path);
    generator.setSize(pixSize);
    generator.setViewBox(QRect(QPoint(0, 0), pixSize));

    QBitmap map(newPix.size());
    map.fill(Qt::color0);
    QPainter p(&map);
    p.setBrush(Qt::color1);
    auto max = std::max(newPix.size().width(), newPix.size().height());
    p.drawRoundedRect(newPix.rect(), max*0.1, max*0.1);

    QPainter painter;
    painter.begin(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    pixmap.setMask(map);
    painter.drawPixmap(QPoint(0, 0), newPix);
    painter.end();

    return path;
}

void CProtocolIOArea::changeInput(int index)
{
    createInputTypeDlg();

    if(m_pInputChoiceDlg->exec() == QDialog::Accepted)
    {
        CProtocolInput input = m_pInputChoiceDlg->getInput();
        auto pScene = static_cast<CProtocolScene*>(scene());
        emit pScene->doSetInput(index, input);
    }
    delete m_pInputChoiceDlg;
    m_pInputChoiceDlg = nullptr;
}

void CProtocolIOArea::deleteInput(int index)
{
    auto pScene = static_cast<CProtocolScene*>(scene());
    assert(pScene);
    emit pScene->doDeleteInput(index);
}

void CProtocolIOArea::showAddPortWidget()
{
    if(m_pAddPortWidget == nullptr)
    {
        CSvgButton* pBtn = new CSvgButton(":/Images/add-input.svg", true);
        pBtn->setToolTip(tr("Add new input"));
        connect(pBtn, &CSvgButton::clicked, this, &CProtocolIOArea::onAddPort);
        m_pAddPortWidget = new QGraphicsProxyWidget(this);
        m_pAddPortWidget->setWidget(pBtn);
    }

    qreal x;
    QPointF center = getNextPortCenter();
    QRectF rect = boundingRect();
    QRectF rectBtn = m_pAddPortWidget->boundingRect();

    if(m_bInput)
        x = rect.right() - rectBtn.width()/2;
    else
        x = rect.left() - rectBtn.width()/2;

    m_pAddPortWidget->setPos(x, center.y() - rectBtn.height()/2);
    m_pAddPortWidget->show();
}

void CProtocolIOArea::hideAddPortWidget()
{
    m_pAddPortWidget->hide();
}

#include "moc_CProtocolIOArea.cpp"
