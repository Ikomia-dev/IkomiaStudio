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

#include "CWorkflowDummyItem.h"

CWorkflowDummyItem::CWorkflowDummyItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    init();
    setAcceptHoverEvents(true);
}

int CWorkflowDummyItem::type() const
{
    return Type;
}

void CWorkflowDummyItem::setColor(QColor color)
{
    m_color = color;
}

void CWorkflowDummyItem::setTextMargin(int margin)
{
    m_textMargin = margin;
    init();
}

void CWorkflowDummyItem::setLineSize(int size)
{
    m_lineSize = size;
}

void CWorkflowDummyItem::setSize(QSize itemSize)
{
    m_size = itemSize;
}

void CWorkflowDummyItem::init()
{
    //Adjust text
    QFont font = m_textItem.font();
    font.setPointSize(8);
    QString text = QObject::tr("Click here to add new process");
    m_textItem.setTextWidth(m_size.width() - 2*m_textMargin);
    m_textItem.setFont(font);
    m_textItem.setPlainText(text);
    m_textItem.setParentItem(this);
    QRectF rcText = m_textItem.boundingRect();
    m_textItem.setPos(m_size.width()/2 - rcText.width()/2, m_size.height()/2 - rcText.height()/2);

    //Rounded rectangle
    QPainterPath p;
    p.addRoundedRect(0, 0, m_size.width(), m_size.height(), m_size.width()*0.1, m_size.width()*0.1, Qt::AbsoluteSize);
    m_painterPath = p;
    setPath(m_painterPath);
}

void CWorkflowDummyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QLinearGradient gradientBody(QPointF( m_size.width()/2, 0), QPointF( m_size.width()/2, m_size.height()));

    gradientBody.setColorAt(0.0, QColor(70,70,70,204));
    gradientBody.setColorAt(0.03, QColor(50,50,50,204));
    gradientBody.setColorAt(0.20, QColor(24,24,24,204));
    gradientBody.setColorAt(0.80, QColor(24,24,24,204));
    gradientBody.setColorAt(0.97, QColor(50,50,50,204));
    gradientBody.setColorAt(1.0, QColor(70,70,70,204));

    QPen pen;
    pen.setWidth(m_lineSize);
    pen.setColor(m_color);
    painter->setPen(pen);
    painter->setBrush(gradientBody);
    painter->drawPath(m_painterPath);

    m_textItem.setDefaultTextColor(m_color);
}

void CWorkflowDummyItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverEnterEvent(event);
}
