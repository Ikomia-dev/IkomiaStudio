#include "CMultiProjectModel.h"

CMultiProjectModel::CMultiProjectModel( QObject * pParent ) : CMultiModel( pParent )
{
    connect(this, &CMultiProjectModel::dataChanged, this, &CMultiProjectModel::onDataChanged);
}

void CMultiProjectModel::removeItem(const QModelIndex& index)
{
    if(index.isValid())
    {
        auto pTreeItem = static_cast<TreeItem*>(index.internalPointer());
        auto pParentItem = pTreeItem->m_pParent;
        if(pTreeItem && pParentItem)
        {
            auto pModel = static_cast<CProjectModel*>(pTreeItem->m_pModel);
            auto itemIndex = wrappedIndex(pTreeItem, pModel, index);

            // Start notify row removal
            auto parentIndex = index.parent();
            beginRemoveRows(parentIndex, index.row(), index.row());
                // Remove item in model
                pModel->removeItem(itemIndex);
                // Remove item from parent childlist
                pParentItem->eraseChild(pTreeItem->m_nRow);
                //pTreeItem->m_pParent->m_children.removeOne( pTreeItem );
                // Remove all children
                //pTreeItem->clearChildren();
            // End notify row removal
            endRemoveRows();

            // Reorder row number inside TreeItem objects
            /*int i = 0;
            for(auto& it : pTreeItem->m_pParent->m_children)
                it->m_nRow = i++;*/

            emit dataChanged(parentIndex, parentIndex);
        }
    }
}

bool CMultiProjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;

    bool bRet = false;
    auto item = static_cast<TreeItem*>(index.internalPointer());

    if(item && item->m_pModel != nullptr)
    {
        auto wrapIndex = wrappedIndex(item, item->m_pModel, index);
        bRet = item->m_pModel->setData(wrapIndex, value, role);
    }
    emit dataChanged(index, index);
    return bRet;
}

Qt::ItemFlags CMultiProjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto pTreeItem = static_cast<TreeItem*>(index.internalPointer());
    if(pTreeItem)
    {
        auto pModel = static_cast<CProjectModel*>(pTreeItem->m_pModel);
        auto itemIndex = wrappedIndex(pTreeItem, pModel, index);
        return pModel->flags(itemIndex);
    }
    return Qt::NoItemFlags;
}

void CMultiProjectModel::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>())
{
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    if(!topLeft.isValid())
        return;

    auto pItem = static_cast<TreeItem*>(topLeft.internalPointer());
    assert(pItem);
    auto pModel = static_cast<CProjectModel*>(pItem->m_pModel);
    assert(pModel);
    const bool projectModified = pModel->isModified();
    auto name = QString::fromStdString(pModel->getRoot()->getChild(0)->getName());

    int row = findProjectRow(topLeft);

    if(projectModified)
    {
        if (!name.endsWith('*'))
            setData(index(row, 0), name + "*");
    }
    else if (name.endsWith('*'))
    {
        name.chop(1);
        setData(index(row, 0), name);
    }
}

QModelIndex CMultiProjectModel::findItemFromName(const std::string &name, QModelIndex startIndex) const
{
    if(!startIndex.isValid())
        return QModelIndex();

    auto layerName = data(startIndex, Qt::DisplayRole).toString().toStdString();

    if(layerName == name)
        return startIndex;
    else
    {
        //Iterate through all child layers
        int childCount = rowCount(startIndex);
        for(int i=0; i<childCount; ++i)
        {
            auto childIndex = index(i, 0, startIndex);
            auto resIndex = findItemFromName(name, childIndex);

            if(resIndex.isValid())
                return resIndex;
        }
    }
    return QModelIndex();
}

int CMultiProjectModel::findProjectRow(const QModelIndex& index)
{
    auto currentIndex = index;
    while(currentIndex.parent() != QModelIndex())
        currentIndex = currentIndex.parent();

    return currentIndex.row();
}

QModelIndex CMultiProjectModel::getWrappedIndex(const QModelIndex& index) const
{
    if(index.isValid() == false)
        return QModelIndex();

    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    return wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
}
