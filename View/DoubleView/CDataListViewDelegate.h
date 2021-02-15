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

#ifndef CDATALISTVIEWDELEGATE_H
#define CDATALISTVIEWDELEGATE_H

/**
 * @file      CDataListViewDelegate.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListViewDelegate definition
 *
 * @details   Details
 */

#include <QStyledItemDelegate>

/**
 * @brief
 *
 */
class CDataListViewDelegate : public QStyledItemDelegate
{
    public:
        /**
         * @brief
         *
         * @param parent
         */
        CDataListViewDelegate(QObject* parent = nullptr);

    protected:
        /**
         * @brief
         *
         * @param painter
         * @param option
         * @param index
         */
        void    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        /**
         * @brief
         *
         * @param option
         * @param index
         * @return QSize
         */
        QSize   sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    private:
        /**
         * @brief
         *
         * @param index
         * @return QIcon
         */
        QIcon   getDataIcon(const QModelIndex& index) const;
        /**
         * @brief
         *
         * @param index
         * @return QString
         */
        QString getDataName(const QModelIndex& index) const;

};

#endif // CDATALISTVIEWDELEGATE_H
