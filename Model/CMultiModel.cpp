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

#include "CMultiModel.h"
#include <memory>
#include <QDebug>

CMultiModel::CMultiModel( QObject * pParent ) : QAbstractItemModel( pParent )
    ,	m_MaxCols( 1 )
{
    m_pRootItem = new TreeItem( this, -1, 0 );
}

CMultiModel::~CMultiModel()
{
    delete m_pRootItem;
}

CMultiModel::TreeItem::TreeItem( QAbstractItemModel * pModel /* = 0 */, int nRow /* = -1 */, TreeItem * pParent /* = 0 */ )
    :	m_pModel( pModel )
    ,	m_nRow( nRow )
    ,	m_pParent( pParent )
{
}

CMultiModel::TreeItem::~TreeItem()
{
    QVector< TreeItem * >::iterator	it = m_children.begin();
    QVector< TreeItem * >::iterator	eIt = m_children.end();
    while ( it != eIt )
        delete *it++;
}

void CMultiModel::TreeItem::clearChildren()
{
    for(auto& it : m_children)
    {
        it->clearChildren();
        delete it;
    }
    // release memory but keep capacity
    m_children.clear();
    // release capacity
    QVector<TreeItem*> v;
    m_children.swap(v);
    assert(m_children.capacity() == 0);
}

void CMultiModel::TreeItem::insertChild(CMultiModel::TreeItem *pItem, int position)
{
    assert(pItem);
    if(position < m_children.size())
    {
        pItem->m_nRow = position;
        m_children.insert(m_children.begin() + position, pItem);
    }
    else
    {
        pItem->m_nRow = m_children.size();
        m_children.push_back(pItem);
    }
}

void CMultiModel::TreeItem::eraseChild(int position)
{
    if(position < m_children.size())
    {
        m_children.erase(m_children.begin() + position);
        for(int i=position; i<m_children.size(); ++i)
            m_children[i]->m_nRow--;
    }
}

// Adding a new model:  This method uses lazy evaluation to wrap a new model in the
// MultiModel's tree.  It first creates a top-level tree item that uses "this" as the
// model pointer, then adds the top-level items from the wrapped model using the wrapped 
// model's pointer and their own row numbers
void CMultiModel::addModel( QAbstractItemModel * pModel )
{
	if ( pModel )
	{
		// mpRootItem->mChildren.size() is the next available row number at the top-most
		// level in the tree		
        int	newRow = m_pRootItem->m_children.size();
        int	oldCols	= m_MaxCols - 1;

		// Expand the column count if needed to accommodate the new model's requirements
		int	nCols = pModel->columnCount();
        if ( nCols > m_MaxCols )
            m_MaxCols = nCols;

        int	newCols = m_MaxCols - 1;
		if ( oldCols != newCols )
			beginInsertColumns( QModelIndex(), oldCols, newCols );

        beginInsertRows( QModelIndex(), newRow, newRow );

        addModelToTree( m_pRootItem, pModel );

		// Tell any listeners that the model has changed
        endInsertRows();

		if ( oldCols != newCols )
			endInsertColumns();
    }
}

void CMultiModel::clear()
{
    beginResetModel();
    m_pRootItem->clearChildren();
    endResetModel();
}

CMultiModel::modelList CMultiModel::getAllModels()
{
    QList<QAbstractItemModel*> modelList;
    auto childList = m_pRootItem->m_children;
    for(auto& it : childList)
        modelList.append(it->m_pModel);

    return modelList;
}

// Note that in this method, the "index" argument is a model index in a wrapped model.
void CMultiModel::addSubModelToTree( TreeItem * pParentItem, QAbstractItemModel * pModel, const QModelIndex & index /* = QModelIndex() */ )
{
	if ( pModel )
	{
        int	nKids = pModel->rowCount( index );
		if ( nKids )
		{
            for ( int nKid = 0; nKid < nKids; ++nKid )
			{
                // Test if already present in tree (because this function is called from addModel and from rowCount)
                bool bIsPresent = false;
                for(auto& it : pParentItem->m_children)
                {
                    if(it->m_nRow == nKid && pParentItem != m_pRootItem)
                        bIsPresent = true;
                }
                if(bIsPresent == false)
                {
                    TreeItem *	pChildItem = new TreeItem( pModel, nKid, pParentItem );
                    pParentItem->m_children.push_back( pChildItem );
                }
			}
        }
    }
}

void CMultiModel::addModelToTree(CMultiModel::TreeItem* pParentItem, QAbstractItemModel* pModel)
{
    if ( pModel && pParentItem)
    {
        TreeItem *	pChildItem = new TreeItem( pModel, pParentItem->m_children.size(), pParentItem );
        pParentItem->m_children.push_back( pChildItem );
    }
}

QModelIndex CMultiModel::index( int row, int col, const QModelIndex & parent /*= QModelIndex() */ ) const
{
	QModelIndex	index;

    if ( row < 0 || col < 0 )
		return index;

	if ( parent.isValid() )
	{
        TreeItem* pTreeItem = static_cast<TreeItem*>(parent.internalPointer());
        //Call to rowCount() mandatory here to ensure children lazy evaluation
        if ( pTreeItem && row < rowCount(parent))
            index = createIndex( row, col, pTreeItem->m_children[ row ] );
	}
	else
    {
        if ( row < m_pRootItem->m_children.size())
            index = createIndex( row, col, m_pRootItem->m_children[ row ] );
    }
	return index;
}

QModelIndex CMultiModel::parent( const QModelIndex & index ) const
{
	QModelIndex	parentIndex;

	if ( !index.isValid() )
        return parentIndex;

	TreeItem *	pTreeItem = (TreeItem *)index.internalPointer();
	if ( pTreeItem )
	{
        if ( pTreeItem->m_pParent == m_pRootItem )
            return parentIndex;
		else
        {
            TreeItem *	pTreeParentItem = pTreeItem->m_pParent;
			if ( pTreeParentItem )
                parentIndex = createIndex( pTreeParentItem->m_nRow, 0, pTreeParentItem );
        }
	}
    return parentIndex;
}

int CMultiModel::rowCount( const QModelIndex & index ) const
{
	int	nRows = 0;
	// If we have a valid index, then we need to retrieve the row count for
	// the appropriate wrapped model, not our own.  If it isn't valid, then we
	// return the number of top-level rows in "this" model.
	if ( index.isValid() )
	{
        TreeItem* pTreeItem = (TreeItem*)index.internalPointer();
		if ( pTreeItem )
		{
			// Lazy evaluation here.  If the items have already been retrieved from the
			// wrapped model, then the tree will already be populated with child information
			// (row count > 0).  If not, then we go into the wrapped model and retrieve the
			// next level and add it to the tree.  For some models that also use lazy evaluation
			// to populate themselves, we must tell them to fetch what they need so we can add
			// the correct information to the tree.

            nRows = pTreeItem->m_children.size();
			if ( 0 == nRows )
			{
                QAbstractItemModel*	pModel = pTreeItem->m_pModel;
				if ( pModel )
				{
					// To retrieve from the wrapped model, we have to translate our model index into 
					// a model index for the wrapped model.

					QModelIndex	wrapped = wrappedIndex( pTreeItem, pModel, index );
					if ( pModel->hasChildren( wrapped ) )
					{
						while( pModel->canFetchMore( wrapped ) )
							pModel->fetchMore( wrapped );

						nRows = pModel->rowCount( wrapped );
                        if ( nRows > 0)
						{
							// Ugly, but unfortunately necessary since rowCount is a const method.
							// Modifying the model while retrieving the row count is a questionable
							// action, but since the rows are being added as children of the current index,
							// it doesn't seem to cause harm.  Be aware that this could be a cause of
							// problems.
                            CMultiModel*	pThis = const_cast< CMultiModel* >( this );
                            pThis->addSubModelToTree( pTreeItem, pModel, wrapped );
						}
					}
				}
			}
		}
	}
	else
        nRows = m_pRootItem->m_children.size();

	return nRows;
}

int CMultiModel::columnCount( const QModelIndex & index ) const
{
    int	nCols = m_MaxCols;
	if ( index.isValid() )
	{
		TreeItem *	pTreeItem = (TreeItem *)index.internalPointer();
		if ( pTreeItem )
		{
            QAbstractItemModel *	pModel = pTreeItem->m_pModel;
			if ( pModel && pModel != this )
			{
				QModelIndex	wrapped = wrappedIndex( pTreeItem, pModel, index );
				nCols = pModel->columnCount( wrapped );
			}
		}
	}
	return nCols;
}

QVariant CMultiModel::data( const QModelIndex & index, int role ) const
{
	QVariant	v;
    if ( index.isValid() )
	{
		TreeItem *	pTreeItem = (TreeItem *)index.internalPointer();
		if ( pTreeItem )
		{
            QAbstractItemModel* pModel = pTreeItem->m_pModel;
			if ( pModel )
			{
                QModelIndex wrapped = wrappedIndex( pTreeItem, pModel, index );
                v = pModel->data( wrapped, role );
			}
		}
	}
    return v;
}

QModelIndex CMultiModel::wrappedIndex( TreeItem * pTreeItem, QAbstractItemModel * pModel, const QModelIndex & index  ) const
{
	// We need to map our index into the appropriate index in the wrapped model
	// To do this, we recurse up the tree until we get to ourself (MultiModel),
	// keeping track of the row numbers in the wrapped model.  We then go back 
	// down through the wrapped model to retrieve -its- index that corresponds
	// to the same location in its tree

	// Row numbers are pushed onto the -front- of the vector as we work our way
	// bottom-up through the tree, so that when we iterate over them, they come 
	// off in top-down order.
	// 
    QVector< int >	rows;
    // If parent is root, we set 0 because a project index always start at row 0
    if(pTreeItem->m_pParent == m_pRootItem)
        rows.push_front(0);
    else
    {
        rows.push_front( pTreeItem->m_nRow );

        TreeItem *	pTreeParent = pTreeItem->m_pParent;
        while ( pTreeParent->m_pParent != m_pRootItem )
        {
            rows.push_front( pTreeParent->m_nRow );
            pTreeParent = pTreeParent->m_pParent;
        }
        // We stop when parent is root and we set 0 because we always start at 0 index in submodel
        rows.push_front(0);
    }

	// Now, work down the wrapped model tree to the correct index
	QModelIndex	wrapped;
	QVector< int >::const_iterator	it = rows.begin();
	QVector< int >::const_iterator	eIt = rows.end();
	while ( it != eIt )
		wrapped = pModel->index( *it++, index.column(), wrapped );

	return wrapped;
}

void CMultiModel::printTree() const
{
    printTree( m_pRootItem, 0 );
}

void CMultiModel::printTree( TreeItem * pItem, int level ) const
{
	if ( pItem )
	{
        qDebug() << "Level " << level << " row " << pItem->m_nRow << " model " << pItem->m_pModel << " item " << pItem << " parent " << pItem->m_pParent;

        QVector< TreeItem * >::const_iterator	it = pItem->m_children.begin();
        QVector< TreeItem * >::const_iterator	eIt = pItem->m_children.end();
		while ( it != eIt )
			printTree( *it++, level + 1 );
	}
}
