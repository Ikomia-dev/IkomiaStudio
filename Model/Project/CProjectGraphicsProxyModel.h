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
