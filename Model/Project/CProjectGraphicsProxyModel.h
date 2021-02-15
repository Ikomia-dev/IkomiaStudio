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

#ifndef CPROJECTGRAPHICSPROXYMODEL_H
#define CPROJECTGRAPHICSPROXYMODEL_H

#include <QSortFilterProxyModel>

class CProjectGraphicsProxyModel: public QSortFilterProxyModel
{
    public:

        CProjectGraphicsProxyModel(const std::vector<QModelIndex>& indicesFrom);

        void            setReferenceIndices(const std::vector<QModelIndex>& indicesFrom);

        Qt::ItemFlags   flags(const QModelIndex &index) const override;
        QVariant        data(const QModelIndex &index, int role) const override;

    protected:

        bool            filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:

        bool            isAscendantOfReferences(const QModelIndex& index) const;
        bool            isAscendantOf(const QModelIndex &refIndex, const QModelIndex& ascIndex) const;
        bool            isReferenceIndex(const QModelIndex& index) const;
        bool            hasChildGraphicsLayer(const QModelIndex& index) const;

        void            copyIndices(const std::vector<QModelIndex> &indices);

    private:

        std::vector<QPersistentModelIndex>   m_refIndices;
};

#endif // CPROJECTGRAPHICSPROXYMODEL_H
