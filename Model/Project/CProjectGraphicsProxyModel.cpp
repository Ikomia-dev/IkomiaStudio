#include "CProjectGraphicsProxyModel.h"
#include "Model/Project/CMultiProjectModel.h"

CProjectGraphicsProxyModel::CProjectGraphicsProxyModel(const std::vector<QModelIndex> &indicesFrom)
    : QSortFilterProxyModel()
{
    copyIndices(indicesFrom);
}

void CProjectGraphicsProxyModel::setReferenceIndices(const std::vector<QModelIndex> &indicesFrom)
{
    copyIndices(indicesFrom);
    invalidateFilter();
}

Qt::ItemFlags CProjectGraphicsProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    auto wrapIndex = pSourceModel->getWrappedIndex(mapToSource(index));
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

    if(pItem->getTypeId() == TreeItemType::GRAPHICS_LAYER)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    else
        return Qt::ItemIsEnabled;
}

QVariant CProjectGraphicsProxyModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role == Qt::CheckStateRole)
        return QVariant();

    return QSortFilterProxyModel::data(index, role);
}

bool CProjectGraphicsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    QModelIndex srcIndex = pSourceModel->index(sourceRow, 0, sourceParent);
    auto wrapIndex = pSourceModel->getWrappedIndex(srcIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    TreeItemType type = static_cast<TreeItemType>(pItem->getTypeId());

    if(type == TreeItemType::GRAPHICS_LAYER)
        return true;
    else if(type == TreeItemType::IMAGE)
    {
        if(isReferenceIndex(wrapIndex) == false)
            return false;
        else
            return hasChildGraphicsLayer(wrapIndex);
    }
    else if(type == TreeItemType::VIDEO)
    {
        if(isReferenceIndex(wrapIndex) == false)
            return false;
        else
            return hasChildGraphicsLayer(wrapIndex);
    }
    else if(type == TreeItemType::LIVE_STREAM)
    {
        if(isReferenceIndex(wrapIndex) == false)
            return false;
        else
            return hasChildGraphicsLayer(wrapIndex);
    }
    else
    {
        if(isAscendantOfReferences(wrapIndex) == false)
            return false;
        else
            return hasChildGraphicsLayer(wrapIndex);
    }
}

bool CProjectGraphicsProxyModel::isAscendantOfReferences(const QModelIndex &index) const
{
    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    for(size_t i=0; i<m_refIndices.size(); ++i)
    {
        auto wrapRefIndex = pSourceModel->getWrappedIndex(m_refIndices[i]);
        if(isAscendantOf(wrapRefIndex, index) == true)
            return true;
    }
    return false;
}

bool CProjectGraphicsProxyModel::isAscendantOf(const QModelIndex& refIndex, const QModelIndex &ascIndex) const
{
    if(refIndex.isValid() == false)
        return false;

    auto parentIndex = refIndex.parent();
    while(parentIndex.isValid() && parentIndex != ascIndex)
        parentIndex = parentIndex.parent();

    return parentIndex.isValid();
}

bool CProjectGraphicsProxyModel::isReferenceIndex(const QModelIndex &index) const
{
    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    for(size_t i=0; i<m_refIndices.size(); ++i)
    {
        auto wrapRefIndex = pSourceModel->getWrappedIndex(m_refIndices[i]);
        if(index == wrapRefIndex)
            return true;
    }
    return false;
}

bool CProjectGraphicsProxyModel::hasChildGraphicsLayer(const QModelIndex &index) const
{
    bool bOk = false;
    auto pModel = index.model();
    int childCount = pModel->rowCount(index);

    for(int i=0; i<childCount; ++i)
    {
        auto indexTmp = pModel->index(i, 0, index);
        auto pChild = static_cast<ProjectTreeItem*>(indexTmp.internalPointer());

        if(pChild->getTypeId() == TreeItemType::GRAPHICS_LAYER)
            return true;
        else
            bOk |= hasChildGraphicsLayer(indexTmp);
    }
    return bOk;
}

void CProjectGraphicsProxyModel::copyIndices(const std::vector<QModelIndex>& indices)
{
    m_refIndices.clear();
    for(size_t i=0; i<indices.size(); ++i)
        m_refIndices.push_back(indices[i]);
}
