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
 * @file      CDataListViewDelegate.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDataListViewDelegate
 *
 * @details   Details
 */

#include "CDataListViewDelegate.h"
#include "Model/Project/CMultiProjectModel.h"
#include <QFileSystemModel>

CDataListViewDelegate::CDataListViewDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void CDataListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect r = option.rect;
    QPainterPath path;
    path.addRoundedRect(r, 10, 10);

    if(option.state & QStyle::State_Selected)
    {
        QBrush brush = option.palette.highlight();
        QColor color = brush.color();
        color.setAlpha(125);
        brush.setColor(color);
        painter->fillPath(path, brush);
    }
    else
    {
        QBrush brush = option.palette.base();
        painter->fillPath(path, brush);
    }

    if(option.state & QStyle::State_MouseOver)
    {
        painter->setPen(QPen(option.palette.highlight().color(), 2));
    }
    else
    {
        painter->setPen(QPen(Qt::lightGray, 2));
    }

    //GET TITLE, DESCRIPTION AND ICON
    QIcon icon = getDataIcon(index);
    QString title = getDataName(index);
    if (!icon.isNull()) {
        //Paint icon in path item
        r = option.rect.adjusted(5, 10, -10, -10);
        icon.paint(painter, r, Qt::AlignHCenter | Qt::AlignTop);
    }

    painter->drawPath(path);
    painter->restore();

    //TITLE
    //r = option.rect.adjusted(10, 5, -10, -30);

    QFont font = qApp->font();
    font.setBold(true);
    font.setPointSize(8);
    QFontMetrics fontM(font);
    QString shortTitle = fontM.elidedText(title, Qt::TextElideMode::ElideRight, r.width()-5);
    painter->setFont(font);
    painter->drawText(r.topLeft().x()+10, r.topLeft().y()+160, shortTitle);
}

QSize CDataListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QSize result(200, 200);
    return result;
}

QIcon CDataListViewDelegate::getDataIcon(const QModelIndex& index) const
{
    QIcon ic = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    return ic;
}

QString CDataListViewDelegate::getDataName(const QModelIndex& index) const
{
    QString title = "Name : " + index.data(Qt::DisplayRole).toString();
    return title;
}
