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
