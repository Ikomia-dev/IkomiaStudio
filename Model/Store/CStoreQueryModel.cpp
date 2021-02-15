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

#include "CStoreQueryModel.h"

CStoreQueryModel::CStoreQueryModel(QObject *parent)
    :QSqlQueryModel(parent)
{
}

QVariant CStoreQueryModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DecorationRole)
    {
        auto pModel = static_cast<const CStoreQueryModel*>(index.model());
        QString iconPathStr = pModel->record(index.row()).value("iconPath").toString();
        if(iconPathStr.isEmpty())
            return QIcon(":/Images/default-process.png");
        else
            return QIcon(iconPathStr);
    }
    return QSqlQueryModel::data(index, role);
}

void CStoreQueryModel::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

CUser CStoreQueryModel::getCurrentUser() const
{
    return m_currentUser;
}
