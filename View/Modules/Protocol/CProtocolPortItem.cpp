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

#include "CProtocolPortItem.h"
#include "CProtocolItem.h"
#include "CProtocolConnection.h"
#include "CProtocolScene.h"
#include "Main/AppTools.hpp"

CProtocolPortItem::CProtocolPortItem(size_t index, bool bInput, ProtocolVertex taskId, IODataType dataType, QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    m_index = index;
    m_bInput = bInput;
    m_taskId = taskId;
    m_dataType = dataType;
    initPainter();
    setAcceptHoverEvents(true);
    auto pal = qApp->palette();
    m_highlightColor = pal.highlight().color();
}

CProtocolPortItem::CProtocolPortItem(size_t index, bool bInput, ProtocolVertex taskId, IODataType dataType, QColor color, QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    m_index = index;
    m_bInput = bInput;
    m_taskId = taskId;
    m_dataType = dataType;
    m_color = color;
    m_borderColor = m_color.darker();
    initPainter();
    setAcceptHoverEvents(true);
    auto pal = qApp->palette();
    m_highlightColor = pal.highlight().color();
}

CProtocolPortItem::~CProtocolPortItem()
{
}

int CProtocolPortItem::type() const
{
    return Type;
}

size_t CProtocolPortItem::getIndex() const
{
    return m_index;
}

float CProtocolPortItem::getRadius() const
{
    return m_baseRadius;
}

ProtocolVertex CProtocolPortItem::getTaskId() const
{
    return m_taskId;
}

int CProtocolPortItem::getConnectionCount() const
{
    return m_connections.size();
}

QVector<CProtocolConnection*> CProtocolPortItem::getConnections() const
{
    return m_connections;
}

QColor CProtocolPortItem::getColor() const
{
    return m_color;
}

IODataType CProtocolPortItem::getDataType() const
{
    return m_dataType;
}

bool CProtocolPortItem::isInput() const
{
    return m_bInput;
}

bool CProtocolPortItem::isConnectedTo(const ProtocolVertex &id) const
{
    for(int i=0; i<m_connections.size(); ++i)
    {
        if(m_connections[i]->contains(m_taskId, id, false))
            return true;
    }
    return false;
}

bool CProtocolPortItem::isClickable() const
{
    return Utils::Protocol::isPortClickable(m_dataType, m_bInput);
}

void CProtocolPortItem::setIndex(size_t index)
{
    m_index = index;
}

void CProtocolPortItem::setColor(QColor color)
{
    m_color = color;
    initPainter();
}

void CProtocolPortItem::setAssigned(bool bAssign)
{
    m_bAssigned = bAssign;
}

void CProtocolPortItem::setRadius(float radius)
{
    m_baseRadius = radius;
    m_radius = radius;
    initPainter();
}

void CProtocolPortItem::setInfo(const CDataInfoPtr &info)
{
    QString strDataType = QString("<b>Data: </b>%1").arg(Utils::Protocol::getIODataName(m_dataType));
    QString text = strDataType;

    if(info)
    {
        VectorPairString infoList = info->getStringList();
        for(size_t i=0; i<infoList.size(); ++i)
        {
            QString value = QString::fromStdString(infoList[i].second);
            if(value.isEmpty() == false)
            {
                text += QString("<br><b>%1: </b>%2")
                        .arg(QString::fromStdString(infoList[i].first))
                        .arg(value);
            }
        }
    }
    setToolTip(text);
}

void CProtocolPortItem::setDataType(IODataType type)
{
    m_dataType = type;
}

bool CProtocolPortItem::connectionExists(CProtocolPortItem *pPort) const
{
    assert(pPort);

    for(int i=0; i<m_connections.size(); ++i)
    {
        if( (m_connections[i]->getSourcePort() == this && m_connections[i]->getTargetPort() == pPort) ||
            (m_connections[i]->getSourcePort() == pPort && m_connections[i]->getTargetPort() == this))
        {
            return true;
        }
    }
    return false;
}

void CProtocolPortItem::addInputConnection(CProtocolConnection *pConnection)
{
    // Remove old connection before replacing by current connection
    if(m_connections.size() > 0)
    {
        for(auto it : m_connections)
            it->onDelete();
    }
    m_bAssigned = false;
    m_connections.push_back(pConnection);
}

void CProtocolPortItem::addOutputConnection(CProtocolConnection *pConnection)
{
    m_connections.push_back(pConnection);
}

void CProtocolPortItem::updateConnections()
{
    for(int i=0; i<m_connections.size(); ++i)
        m_connections[i]->updatePath();
}

void CProtocolPortItem::updateShape(QPointF pos)
{
    if(pos.isNull())
    {
        m_radius = m_baseRadius;
    }
    else
    {
        auto diff = this->scenePos() - pos;
        auto d = std::sqrt(QPointF::dotProduct(diff, diff));
        auto thresh = m_baseRadius*2.5;
        auto r = d < thresh ? 3 * std::exp(-d/thresh) : 1;
        m_radius = m_baseRadius * r;
    }
    initPainter();
}

void CProtocolPortItem::removeConnection(CProtocolConnection *pConnection)
{
    for(int i=m_connections.size() - 1; i>=0; --i)
    {
        if(m_connections[i] == pConnection)
        {
            m_connections.erase(m_connections.begin() + i);
            return;
        }
    }
}

void CProtocolPortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Draw main port look
    QPen pen(m_borderColor);
    QBrush brush(m_color);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawPath(m_mainPath);

    // If graphics are assigned to this port
    if(m_bAssigned == true)
    {
        brush.setColor(m_highlightColor);
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);
        painter->drawPath(m_assignedPath);
    }
}

void CProtocolPortItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(isClickable() == true)
    {
        m_color = m_color.lighter(125);
        initPainter();
    }
    retrieveIOInfo();
    m_borderColor = m_highlightColor;
    QGraphicsItem::hoverEnterEvent(event);
}

void CProtocolPortItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    QGraphicsItem::hoverMoveEvent(event);
}

void CProtocolPortItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(isClickable() == true)
    {
        m_color = m_color.darker(125);
        initPainter();
    }
    m_borderColor = m_color.darker();
    QGraphicsItem::hoverLeaveEvent(event);
}

void CProtocolPortItem::clearConnections()
{
    m_connections.clear();
}

void CProtocolPortItem::retrieveIOInfo()
{
    auto pScene = static_cast<CProtocolScene*>(scene());
    if(pScene)
        pScene->retrieveIOInfo(m_taskId, m_index, m_bInput);
}

void CProtocolPortItem::initPainter()
{
    // Init main port look
    QPainterPath p1;
    p1.setFillRule(Qt::WindingFill);
    p1.addEllipse(-m_radius, -m_radius, 2*m_radius, 2*m_radius);
    m_mainPath = p1;
    setPath(m_mainPath);

    // Init graphics port check point
    QPainterPath p2;
    p2.setFillRule(Qt::WindingFill);
    p2.addEllipse(-m_radius/2, -m_radius/2, m_radius, m_radius);
    m_assignedPath = p2;
}


