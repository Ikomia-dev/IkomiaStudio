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

#include "CWorkflowConnection.h"
#include <QApplication>
#include "CWorkflowPortItem.h"
#include "CWorkflowScene.h"
#include <QGraphicsDropShadowEffect>
#include "View/Common/CSvgButton.h"

CWorkflowConnection::CWorkflowConnection(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    setAcceptHoverEvents(true);
    init();
}

CWorkflowConnection::CWorkflowConnection(WorkflowEdge id, QGraphicsItem* parent) : QGraphicsPathItem(parent)
{
    m_id = id;
    setAcceptHoverEvents(true);
    init();
}

CWorkflowConnection::~CWorkflowConnection()
{
    if(m_pSrcPort)
        m_pSrcPort->removeConnection(this);
    if(m_pDstPort)
        m_pDstPort->removeConnection(this);
}

int CWorkflowConnection::type() const
{
    return Type;
}

WorkflowEdge CWorkflowConnection::getId() const
{
    return m_id;
}

CWorkflowPortItem *CWorkflowConnection::getSourcePort() const
{
    return m_pSrcPort;
}

CWorkflowPortItem *CWorkflowConnection::getTargetPort() const
{
    return m_pDstPort;
}

void CWorkflowConnection::setSourcePort(CWorkflowPortItem *pPort)
{
    m_pSrcPort = pPort;
    if(m_pSrcPort)
        m_pSrcPort->addOutputConnection(this);
}

void CWorkflowConnection::setTargetPort(CWorkflowPortItem *pPort)
{
    m_pDstPort = pPort;
    if(m_pDstPort)
        m_pDstPort->addInputConnection(this);
}

void CWorkflowConnection::updatePath()
{
    if(m_pSrcPort == nullptr || m_pDstPort == nullptr)
        return;

    QPainterPath path;
    QPointF srcPos = m_pSrcPort->scenePos();
    QPointF dstPos = m_pDstPort->scenePos();
    path.moveTo(srcPos);

    qreal dx = dstPos.x() - srcPos.x();
    /*qreal dy = dstPos.y() - srcPos.y();

    QPointF ctr1(srcPos.x() + dx * 0.5, srcPos.y() + dy * 0.1);
    QPointF ctr2(srcPos.x() + dx * 0.5, srcPos.y() + dy * 0.9);*/

    // Try other formula
    double defaultOffset = 200;
    double minimum = qMin(defaultOffset, std::abs(dx));
    double verticalOffset = 0;
    double ratio1 = 0.5;

    if (dx <= 0)
    {
      verticalOffset = -minimum;
      ratio1 = 1.0;
    }

    QPointF ctr1(srcPos.x() + minimum * ratio1, srcPos.y() + verticalOffset);
    QPointF ctr2(dstPos.x() - minimum * ratio1, dstPos.y() + verticalOffset);
    path.cubicTo(ctr1, ctr2, dstPos);

    setPen(createPen(srcPos, dstPos));
    setPath(path);
}

void CWorkflowConnection::updatePath(QPointF currentPos)
{
    setZValue(1);

    QPainterPath path;
    QPointF srcPos = m_pSrcPort->scenePos();
    path.moveTo(srcPos);
    qreal dx = currentPos.x() - srcPos.x();
    /*qreal dy = currentPos.y() - srcPos.y();
    QPointF ctr1(srcPos.x() + dx * 0.5, srcPos.y() + dy * 0.1);
    QPointF ctr2(srcPos.x() + dx * 0.5, srcPos.y() + dy * 0.9);*/

    // Try other formula
    double defaultOffset = 200;
    double minimum = qMin(defaultOffset, std::abs(dx));
    double verticalOffset = 0;
    double ratio1 = 0.5;

    if (dx <= 0)
    {
      verticalOffset = -minimum;
      ratio1 = 1.0;
    }

    QPointF ctr1(srcPos.x() + minimum * ratio1, srcPos.y() + verticalOffset);
    QPointF ctr2(currentPos.x() - minimum * ratio1, currentPos.y() + verticalOffset);
    auto pos = currentPos;
    pos.setX(pos.x() - m_portRadius);
    path.cubicTo(ctr1, ctr2, pos);
    path.addEllipse(currentPos, m_portRadius, m_portRadius);
    path.addEllipse(currentPos, 1, 1);

    QPen pen = createPen(srcPos, currentPos);
    pen.setStyle(Qt::DashLine);
    setPen(pen);

    setPath(path);
}

bool CWorkflowConnection::contains(const WorkflowVertex &id1, const WorkflowVertex &id2, bool bDirected)
{
    assert(m_pSrcPort && m_pDstPort);

    if(bDirected == false)
    {
        return ((m_pSrcPort->getTaskId() == id1 && m_pDstPort->getTaskId() == id2) ||
                (m_pSrcPort->getTaskId() == id2 && m_pDstPort->getTaskId() == id1));
    }
    else
        return m_pSrcPort->getTaskId() == id1 && m_pDstPort->getTaskId() == id2;
}

void CWorkflowConnection::prepareRemove()
{
    //Workaround to prevent QT bug when removing from the scene a QGraphicsItem with QGraphicsDropShadowEffect
    prepareGeometryChange();
}

void CWorkflowConnection::onDelete()
{
    prepareRemove();

    if(m_pSrcPort)
        m_pSrcPort->removeConnection(this);

    if(m_pDstPort)
        m_pDstPort->removeConnection(this);

    auto pScene = static_cast<CWorkflowScene*>(scene());
    pScene->deleteConnection(this, true, true, true);
}

void CWorkflowConnection::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    auto pal = qApp->palette();
    QPen pen(pal.highlight().color(), m_penSize);
    setPen(pen);
    showDeleteWidget();
}

void CWorkflowConnection::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setPen(createPen(m_pSrcPort->scenePos(), m_pDstPort->scenePos()));
    hideDeleteWidget();
}

void CWorkflowConnection::init()
{
    auto pShadow = new QGraphicsDropShadowEffect;
    pShadow->setBlurRadius(9.0);
    pShadow->setColor(QColor(0, 0, 0, 160));
    pShadow->setOffset(4.0);
    setGraphicsEffect(pShadow);
}

void CWorkflowConnection::showDeleteWidget()
{
    QPointF srcPos = m_pSrcPort->scenePos();
    QPointF dstPos = m_pDstPort->scenePos();
    QPointF pos = (srcPos + dstPos) / 2;

    if(m_pDeleteWidget == nullptr)
    {
        CSvgButton* pBtn = new CSvgButton(":/Images/close-connect.svg", true);
        connect(pBtn, &CSvgButton::clicked, this, &CWorkflowConnection::onDelete);
        m_pDeleteWidget = new QGraphicsProxyWidget(this);
        m_pDeleteWidget->setWidget(pBtn);
    }

    QPoint offset(m_pDeleteWidget->widget()->width()/2, m_pDeleteWidget->widget()->height()/2);
    pos -= offset;
    m_pDeleteWidget->setPos(pos);
    m_pDeleteWidget->show();
}

void CWorkflowConnection::hideDeleteWidget()
{
    m_pDeleteWidget->hide();
}

QPen CWorkflowConnection::createPen(const QPointF& pt1, const QPointF& pt2)
{
    QColor srcColor = m_pSrcPort->getColor();

    QColor dstColor;
    if(m_pDstPort)
        dstColor = m_pDstPort->getColor();
    else
        dstColor = srcColor;

    if(srcColor == dstColor)
        return QPen(srcColor, m_penSize);
    else
    {
        QLinearGradient gradient;
        gradient.setStart(pt1);
        gradient.setFinalStop(pt2);
        gradient.setColorAt(0.0, srcColor);
        gradient.setColorAt(1.0, dstColor);
        return QPen(gradient, m_penSize);
    }
}

#include "moc_CWorkflowConnection.cpp"
