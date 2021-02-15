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

#ifndef CPROTOCOLLABELAREA_H
#define CPROTOCOLLABELAREA_H

#include <QGraphicsPathItem>
#include <QLinearGradient>
#include "Main/AppDefine.hpp"

class CProtocolLabelArea : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + ProtocolGraphicsItem::LABEL_AREA };

        CProtocolLabelArea(QString label, QGraphicsItem* parent = Q_NULLPTR);

        //Getters
        int     type() const override;
        QSize   getSize() const;

        //Setters
        void    setSize(QSize size);
        void    setTextColor(QColor color);
        void    setColorBg(QColor color);
        void    setLineColor(QColor color);

        void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

        void    update();

    private:

        void    initText();
        void    initArea();

    protected:

        QString         m_label;
        QSize           m_size = QSize(80,120);
        int             m_topMargin = 1;
        int             m_topContent = 1;
        int             m_lineSize = 2;
        QPainterPath    m_areaPath;
        QPainterPath    m_textPath;
        QColor          m_textColor = Qt::white;
        QColor          m_bckColor = Qt::darkBlue;
        QColor          m_lineColor = Qt::blue;
        QColor          m_lineSelected;
        QLinearGradient m_areaGradient;
};

#endif // CPROTOCOLLABELAREA_H
