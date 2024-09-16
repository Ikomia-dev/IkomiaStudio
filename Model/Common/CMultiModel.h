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

#ifndef CMULTIMODEL_H
#define CMULTIMODEL_H

#include <QAbstractItemModel>
#include <QVector>

class CMultiModel :	public QAbstractItemModel
{
        Q_OBJECT

    public:

        using modelList = QList<QAbstractItemModel*>;

        // The private TreeItem class holds the concatenated trees of all the wrapped models
        class TreeItem
        {
            public:
                TreeItem( QAbstractItemModel * pModel = 0, int nRow = -1, TreeItem * pParent = 0 );
                ~TreeItem();

                void    clearChildren();
                void    insertChild(TreeItem* pItem, int position);
                void    eraseChild(int position);

                QAbstractItemModel *		m_pModel = nullptr;     // Wrapped model or "this", depending on level in tree
                int							m_nRow;                 // Row number of item with respect to level
                TreeItem *					m_pParent = nullptr;	// Back pointer to parent of this item
                QVector< TreeItem * >       m_children;             // Forward pointers to children of this item
        };

        CMultiModel( QObject * pParent = 0 );
        virtual ~CMultiModel();

        // Overrides / pure virtual method implementations
        virtual QModelIndex index( int row, int col, const QModelIndex & parent = QModelIndex() ) const override;
        virtual QModelIndex parent( const QModelIndex & index ) const override;
        virtual int         rowCount( const QModelIndex & index ) const override;
        virtual int         columnCount( const QModelIndex & index ) const override;
        virtual QVariant    data( const QModelIndex & index, int role ) const override;

        virtual void        addModel( QAbstractItemModel * pModel );
        virtual void        clear();
        virtual modelList   getAllModels();

        // Utility function to print the tree on qDebug()
        virtual void        printTree() const;
        // Retrieve the actual index from the wrapped model
        virtual QModelIndex wrappedIndex( TreeItem * pTreeItem, QAbstractItemModel * pModel, const QModelIndex & index ) const;

    protected:

        void                addSubModelToTree( TreeItem * pParentItem, QAbstractItemModel * pModel, const QModelIndex & index = QModelIndex() );
        void                addModelToTree(TreeItem * pParentItem, QAbstractItemModel * pModel);
        void                printTree( TreeItem * pItem, int level ) const;

    protected:

        TreeItem *	m_pRootItem = nullptr;                        // Root of the tree; initially empty
        int			m_MaxCols;                                   // Maximum column count for all wrapped models
};

#endif // CMULTIMODEL_H
