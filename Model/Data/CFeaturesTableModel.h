#ifndef CFEATURESTABLEMODEL_H
#define CFEATURESTABLEMODEL_H

#include <QAbstractTableModel>

class CFeaturesTableModel : public QAbstractTableModel
{
    public:

        using StringVector = std::vector<std::string>;
        using VectorOfStringVector = std::vector<std::vector<std::string>>;

        CFeaturesTableModel(QObject* pParent = nullptr);

        void        insertData(const VectorOfStringVector& values, const VectorOfStringVector& valueLabels, const StringVector& headerLabels);

        int         rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int         columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant    data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


    private:

        int                     m_rowCount = 0;
        VectorOfStringVector    m_values;
        VectorOfStringVector    m_valueLabels;
        StringVector            m_headerLabels;
};

#endif // CFEATURESTABLEMODEL_H
