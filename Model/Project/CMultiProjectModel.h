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

#ifndef CMULTIPROJECTMODEL_H
#define CMULTIPROJECTMODEL_H

#include "../CMultiModel.h"
#include "CProjectModel.h"

class CMultiProjectModel : public CMultiModel
{
    Q_OBJECT

    public:

        CMultiProjectModel( QObject * pParent = Q_NULLPTR );

        template<typename T>
        QModelIndex     addItem(const QModelIndex &parentIndex, T&& t)
        {
            if(!parentIndex.isValid())
                return QModelIndex();

            auto pTreeItem = static_cast<TreeItem*>(parentIndex.internalPointer());
            if(!pTreeItem)
                return QModelIndex();

            auto pModel = static_cast<CProjectModel*>(pTreeItem->m_pModel);
            auto itemIndex = wrappedIndex(pTreeItem, pModel, parentIndex);

            beginInsertRows(parentIndex, pTreeItem->m_children.size(), pTreeItem->m_children.size());
                // Add item under index at the end
                auto pChildItem = new TreeItem(pModel, pModel->rowCount(itemIndex), pTreeItem);
                pTreeItem->m_children.push_back(pChildItem);
                // Add item to model
                pModel->emplace_back(itemIndex, std::forward<T>(t));
            endInsertRows();

            emit dataChanged(parentIndex, parentIndex);
            return index(rowCount(parentIndex) - 1, 0, parentIndex);
        }

        template<typename T>
        bool            moveItem(const QModelIndex& index, const QModelIndex& targetIndex, int targetPosition)
        {
            if(!index.isValid() || !targetIndex.isValid())
                return false;

            auto pTreeItem = static_cast<TreeItem*>(index.internalPointer());
            auto pParentItem = pTreeItem->m_pParent;
            auto pTargetTreeItem = static_cast<TreeItem*>(targetIndex.internalPointer());

            if(pTreeItem && pParentItem && pTargetTreeItem)
            {
                auto pModel = static_cast<CProjectModel*>(pTreeItem->m_pModel);
                auto pTargetModel = static_cast<CProjectModel*>(pTargetTreeItem->m_pModel);

                if(pModel == pTargetModel)
                {
                    beginMoveRows(index.parent(), pTreeItem->m_nRow, pTreeItem->m_nRow, targetIndex, targetPosition);
                        //Move into model
                        auto wrapItemIndex = wrappedIndex(pTreeItem, pModel, index);
                        auto wrapTargetIndex = wrappedIndex(pTargetTreeItem, pModel, targetIndex);
                        pModel->moveRow<T>(wrapItemIndex, wrapTargetIndex, targetPosition);
                        //Update multi-model
                        pParentItem->eraseChild(pTreeItem->m_nRow);
                        pTreeItem->m_pParent = pTargetTreeItem;
                        pTargetTreeItem->insertChild(pTreeItem, targetPosition);
                    endMoveRows();
                    return true;
                }
            }
            return false;
        }

        void            removeItem(const QModelIndex& index);

        bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        Qt::ItemFlags   flags(const QModelIndex &index) const;

        QModelIndex     findItemFromName(const std::string &name, QModelIndex startIndex) const;

        QModelIndex     getWrappedIndex(const QModelIndex& index) const;

    private slots:

        void            onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

    private:

        int             findProjectRow(const QModelIndex& index);
};

#endif // CMULTIPROJECTMODEL_H
