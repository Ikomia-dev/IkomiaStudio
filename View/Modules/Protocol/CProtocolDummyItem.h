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

#ifndef CPROTOCOLDUMMYITEM_H
#define CPROTOCOLDUMMYITEM_H

#include <QGraphicsPathItem>
#include "Main/forwards.hpp"
#include "Main/AppDefine.hpp"
#include "View/Common/CGraphicsItemTools.hpp"

class CProtocolDummyItem : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + ProtocolGraphicsItem::DUMMY };

        CProtocolDummyItem(QGraphicsItem* parent = Q_NULLPTR);

        int     type() const override;

        void    setColor(QColor color);
        void    setTextMargin(int margin);
        void    setLineSize(int size);
        void    setSize(QSize itemSize);

        void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:

        void    hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    private:

        void    init();

    private:

        int                         m_textMargin = 5;
        int                         m_lineSize = 2;
        QSize                       m_size = QSize(80, 80);
        QColor                      m_color = Qt::white;
        CGraphicsInactiveTextItem   m_textItem;
        QPainterPath                m_painterPath;
};

#endif // CPROTOCOLDUMMYITEM_H
