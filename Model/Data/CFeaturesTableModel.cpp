#include "CFeaturesTableModel.h"

CFeaturesTableModel::CFeaturesTableModel(QObject *pParent): QAbstractTableModel(pParent)
{
}

void CFeaturesTableModel::insertData(const CFeaturesTableModel::VectorOfStringVector &values, const CFeaturesTableModel::VectorOfStringVector &valueLabels, const CFeaturesTableModel::StringVector &headerLabels)
{
    m_rowCount = 0;
    m_values = values;
    m_valueLabels = valueLabels;
    m_headerLabels = headerLabels;

    for(size_t i=0; i<m_values.size(); ++i)
        m_rowCount = std::max(m_rowCount, (int)m_values[i].size());
}

int CFeaturesTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rowCount;
}

int CFeaturesTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return (int)(m_values.size() + m_valueLabels.size());
}

QVariant CFeaturesTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int col = index.column();
    int row = index.row();

    if(m_values.size() == m_valueLabels.size())
    {
        if(col % 2 == 0)
            return QString::fromStdString(m_valueLabels[col / 2][row]);
        else
            return QString::fromStdString(m_values[col / 2][row]);
    }
    else if(m_valueLabels.size() == 1)
    {
        if(index.column() == 0)
            return QString::fromStdString(m_valueLabels[0][row]);
        else
            return QString::fromStdString(m_values[col - 1][row]);
    }
    else
        return QString::fromStdString(m_values[col][row]);
}

QVariant CFeaturesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if(m_values.size() == m_valueLabels.size())
        {
            int headerIndex = section / 2;
            if(section%2 != 0 && headerIndex < (int)m_headerLabels.size())
                return QString::fromStdString(m_headerLabels[headerIndex]);
        }
        else if(m_valueLabels.size() == 1)
        {
            int headerIndex = section - 1;
            if(section > 0 && headerIndex < (int)m_headerLabels.size())
                return QString::fromStdString(m_headerLabels[headerIndex]);
        }
        else
        {
            if(section < (int)m_headerLabels.size())
                return QString::fromStdString(m_headerLabels[section]);
        }
    }
    return QVariant();
}
