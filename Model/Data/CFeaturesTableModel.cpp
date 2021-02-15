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
