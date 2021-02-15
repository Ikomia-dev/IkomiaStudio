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
