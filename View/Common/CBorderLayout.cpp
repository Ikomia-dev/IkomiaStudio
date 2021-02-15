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
 * @file      CBorderLayout.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CBorderLayout
 *
 * @details   Details
 */

#include "CBorderLayout.h"
#include <QSize>

CBorderLayout::CBorderLayout(QWidget *parent, int margin, int spacing)
    : QLayout(parent)
{
    setMargin(margin);
    setSpacing(spacing);
}

CBorderLayout::CBorderLayout(int spacing)
{
    setSpacing(spacing);
}

CBorderLayout::~CBorderLayout()
{
    for(int i=0 ; i<m_list.size(); ++i)
    {
        if(m_list[i])
        {
            delete m_list[i]->m_item;
            delete m_list[i];
        }
    }
}

void CBorderLayout::addItem(QLayoutItem *item)
{
    add(item, West);
}

void CBorderLayout::addWidget(QWidget *widget, Position position)
{
    add(new QWidgetItem(widget), position);
}

Qt::Orientations CBorderLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

bool CBorderLayout::hasHeightForWidth() const
{
    return false;
}

int CBorderLayout::count() const
{
    return m_list.size();
}

QLayoutItem *CBorderLayout::itemAt(int index) const
{
    ItemWrapper *wrapper = m_list.value(index);
    if (wrapper)
        return wrapper->m_item;
    else
        return 0;
}

QSize CBorderLayout::minimumSize() const
{
    return calculateSize(MinimumSize);
}

void CBorderLayout::setGeometry(const QRect &rect)
{
    ItemWrapper *center = 0;
    int eastWidth = 0;
    int westWidth = 0;
    int northHeight = 0;
    int southHeight = 0;
    int centerHeight = 0;
    int i;

    QLayout::setGeometry(rect);

    for (i = 0; i < m_list.size(); ++i)
    {
        ItemWrapper *wrapper = m_list.at(i);
        QLayoutItem *item = wrapper->m_item;
        Position position = wrapper->m_position;

        if (position == North)
        {
            item->setGeometry(QRect(rect.x(), northHeight, rect.width(), item->sizeHint().height()));
            northHeight += item->geometry().height() + spacing();
        }
        else if (position == South)
        {
            item->setGeometry(QRect(item->geometry().x(),
                                    item->geometry().y(), rect.width(),
                                    item->sizeHint().height()));

            southHeight += item->geometry().height() + spacing();

            item->setGeometry(QRect(rect.x(),
                                    rect.y() + rect.height() - southHeight + spacing(),
                                    item->geometry().width(),
                                    item->geometry().height()));
        }
        else if (position == Center)
            center = wrapper;
    }

    centerHeight = rect.height() - northHeight - southHeight;

    for (i = 0; i < m_list.size(); ++i)
    {
        ItemWrapper *wrapper = m_list.at(i);
        QLayoutItem *item = wrapper->m_item;
        Position position = wrapper->m_position;

        if (position == West)
        {
            item->setGeometry(QRect(rect.x() + westWidth, northHeight, item->sizeHint().width(), centerHeight));
            westWidth += item->geometry().width() + spacing();
        }
        else if (position == East)
        {
            item->setGeometry(QRect(item->geometry().x(), item->geometry().y(),
                                    item->sizeHint().width(), centerHeight));

            eastWidth += item->geometry().width() + spacing();

            item->setGeometry(QRect(rect.x() + rect.width() - eastWidth + spacing(),
                                    northHeight, item->geometry().width(),
                                    item->geometry().height()));
        }
    }

    if (center)
        center->m_item->setGeometry(QRect(westWidth, northHeight, rect.width() - eastWidth - westWidth, centerHeight));
}

QRect CBorderLayout::getBorderRect(CBorderLayout::Position pos) const
{
    QRect rc;
    for(int i=0; i<m_list.size(); ++i)
    {
        ItemWrapper* pWrapper = m_list.at(i);
        if(pWrapper->m_position == pos)
        {
            auto itemRect = pWrapper->m_item->widget()->geometry();
            auto topLeft = pWrapper->m_item->widget()->mapToGlobal(itemRect.topLeft());
            auto bottomRight = pWrapper->m_item->widget()->mapToGlobal(itemRect.bottomRight());
            QRect screenItemRect(topLeft, bottomRight);

            if(rc.isEmpty() == true)
                rc = screenItemRect;
            else
                rc = rc.united(screenItemRect);
        }
    }
    return rc;
}

QSize CBorderLayout::sizeHint() const
{
    return calculateSize(SizeHint);
}

QLayoutItem *CBorderLayout::takeAt(int index)
{
    if (index >= 0 && index < m_list.size())
    {
        ItemWrapper *layoutStruct = m_list.takeAt(index);
        return layoutStruct->m_item;
    }
    return 0;
}

void CBorderLayout::add(QLayoutItem *item, Position position)
{
    m_list.append(new ItemWrapper(item, position));
}

QSize CBorderLayout::calculateSize(SizeType sizeType) const
{
    QSize totalSize(0, 0);

    for (int i = 0; i < m_list.size(); ++i)
    {
        ItemWrapper *wrapper = m_list.at(i);
        Position position = wrapper->m_position;
        QSize itemSize;

        if (sizeType == MinimumSize)
            itemSize = wrapper->m_item->minimumSize();
        else // (sizeType == SizeHint)
            itemSize = wrapper->m_item->sizeHint();

        if (position == North || position == South || position == Center)
            totalSize.rheight() += itemSize.height();

        if (position == West || position == East || position == Center)
            totalSize.rwidth() += itemSize.width();
    }
    return totalSize;
}

#include "moc_CBorderLayout.cpp"
