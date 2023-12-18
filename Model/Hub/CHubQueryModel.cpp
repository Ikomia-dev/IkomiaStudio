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

#include "CHubQueryModel.h"

CHubQueryModel::CHubQueryModel(QObject *parent)
    :QSqlQueryModel(parent)
{
}

QVariant CHubQueryModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DecorationRole)
    {
        auto pModel = static_cast<const CHubQueryModel*>(index.model());
        QString iconPathStr = pModel->record(index.row()).value("iconPath").toString();
        if(iconPathStr.isEmpty())
            return QIcon(":/Images/default-process.png");
        else
            return QIcon(iconPathStr);
    }
    return QSqlQueryModel::data(index, role);
}

void CHubQueryModel::setCurrentUser(const CUser &user)
{
    m_user = user;
}

CUser CHubQueryModel::getCurrentUser() const
{
    return m_user;
}

QModelIndex CHubQueryModel::getAlgorithmIndex(const QString &name) const
{
    for (int i=0; i<rowCount(); ++i)
    {
        QString algoName = record(i).value("name").toString();
        if (algoName == name)
            return index(i, 0);
    }
    return QModelIndex();
}
