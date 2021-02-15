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

#ifndef CDATALISTVIEWPROXYMODEL_H
#define CDATALISTVIEWPROXYMODEL_H

/**
 * @file      CDataListViewProxyModel.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListViewProxyModel definition
 *
 * @details   Details
 */

#include <QSortFilterProxyModel>

/**
 * @brief
 *
 */
class CProjectViewProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        /**
         * @brief
         *
         * @param parent
         */
        CProjectViewProxyModel(QObject * parent = nullptr);

        /**
         * @brief
         *
         * @param index
         * @param role
         * @return QVariant
         */
        QVariant    data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    signals:        

        void        doCreateIcon(const QPersistentModelIndex& index, QSize size) const;

    public slots:

        void        onCreateIcon(const QPersistentModelIndex& index, QSize size);

    private:

        QMap<QString, QIcon> m_iconMap; /**< TODO: describe */
};

#endif // CDATALISTVIEWPROXYMODEL_H
