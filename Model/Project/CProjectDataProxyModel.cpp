#include "CProjectDataProxyModel.h"
#include "Model/Project/CMultiProjectModel.h"
#include "Model/Project/CProjectUtils.hpp"

CProjectDataProxyModel::CProjectDataProxyModel(const QModelIndex& rootProjectIndex, const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension> &filters) : QSortFilterProxyModel()
{
    m_rootIndex = rootProjectIndex;
    m_dataTypes = dataTypes;
    m_dataFilters = filters;
}

void CProjectDataProxyModel::setProxyParameters(const QModelIndex &index, const std::vector<TreeItemType> &dataTypes, const std::vector<DataDimension>& filters)
{
    if(index != m_rootIndex || dataTypes != m_dataTypes || m_dataFilters != filters)
    {
        m_rootIndex = index;
        m_dataTypes = dataTypes;
        m_dataFilters = filters;
        invalidateFilter();
    }
}

Qt::ItemFlags CProjectDataProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    auto wrapIndex = pSourceModel->getWrappedIndex(mapToSource(index));
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    auto typeId = static_cast<TreeItemType>(pItem->getTypeId());

    if(isValidDataType(typeId) == true)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    else
        return Qt::ItemIsEnabled;
}

QVariant CProjectDataProxyModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role == Qt::CheckStateRole)
        return QVariant();

    return QSortFilterProxyModel::data(index, role);
}

bool CProjectDataProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto pSourceModel = static_cast<CMultiProjectModel*>(sourceModel());
    QModelIndex srcIndex = pSourceModel->index(sourceRow, 0, sourceParent);
    auto wrapIndex = pSourceModel->getWrappedIndex(srcIndex);
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    auto typeId = static_cast<TreeItemType>(pItem->getTypeId());

    //We want to accept only items from project pointed by m_rootIndex
    if(isSameProject(srcIndex) == false)
        return false;

    if(isAscendantType(typeId))
        return true;

    if(isValidDataType(typeId) == false)
        return false;

    if(typeId == TreeItemType::DATASET)
    {
        auto it = std::find(m_dataTypes.begin(), m_dataTypes.end(), TreeItemType::DATASET);
        if(it != m_dataTypes.end())
            return isValidDataset(wrapIndex);
    }

    if(typeId == TreeItemType::FOLDER)
    {
        auto it = std::find(m_dataTypes.begin(), m_dataTypes.end(), TreeItemType::FOLDER);
        if(it != m_dataTypes.end())
            return isValidFolder(wrapIndex);
    }
    return true;
}

bool CProjectDataProxyModel::isSameProject(const QModelIndex &index) const
{
    if(index == m_rootIndex)
        return true;

    auto parentIndex = index.parent();
    if(parentIndex.isValid() == false)
        return false;

    QModelIndex tmpIndex;
    while(parentIndex.isValid() == true)
    {
        tmpIndex = parentIndex;
        parentIndex = tmpIndex.parent();
    }
    return tmpIndex == m_rootIndex;
}

bool CProjectDataProxyModel::isAscendantType(const TreeItemType &dataType) const
{
    for(size_t i=0; i<m_dataTypes.size(); ++i)
    {
        if(CProjectUtils::isAscendantItemType(dataType, m_dataTypes[i]))
            return true;
    }
    return false;
}

bool CProjectDataProxyModel::isValidDataType(const TreeItemType& dataType) const
{
    auto it = std::find(m_dataTypes.begin(), m_dataTypes.end(), dataType);
    return it != m_dataTypes.end();
}

bool CProjectDataProxyModel::isValidDataset(const QModelIndex &wrapIndex) const
{
    if(m_dataFilters.empty())
        return true;

    auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex);
    for(size_t i=0; i<m_dataFilters.size(); ++i)
    {
        if(pDataset->hasDimension(m_dataFilters[i]))
            return true;
    }
    return false;
}

bool CProjectDataProxyModel::isValidFolder(const QModelIndex &wrapIndex) const
{
    Q_UNUSED(wrapIndex);
    return true;
}
