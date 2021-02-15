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

#ifndef CPROJECTIMAGEPROXYMODEL_H
#define CPROJECTIMAGEPROXYMODEL_H

#include <QSortFilterProxyModel>

/*
 * Proxy model of CMultiProjectModel
 * Filter the source model so that only images from current project are kept
*/
class CProjectDataProxyModel : public QSortFilterProxyModel
{
    public:

        CProjectDataProxyModel(const QModelIndex &rootProjectIndex, const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters);

        void            setProxyParameters(const QModelIndex& index, const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension> &filters);

        Qt::ItemFlags   flags(const QModelIndex &index) const override;
        QVariant        data(const QModelIndex &index, int role) const override;

    protected:

        bool            filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:

        bool            isSameProject(const QModelIndex& index) const;
        bool            isAscendantType(const TreeItemType &dataType) const;
        bool            isValidDataType(const TreeItemType &dataType) const;
        bool            isValidDataset(const QModelIndex& wrapIndex) const;
        bool            isValidFolder(const QModelIndex& wrapIndex) const;

    private:

        QPersistentModelIndex       m_rootIndex;
        std::vector<TreeItemType>   m_dataTypes;
        std::vector<DataDimension>  m_dataFilters;
};

#endif // CPROJECTIMAGEPROXYMODEL_H
