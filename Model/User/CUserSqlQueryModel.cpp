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

#include "CUserSqlQueryModel.h"
#include "Main/AppTools.hpp"

CUserSqlQueryModel::CUserSqlQueryModel(QObject *parent) : QSqlQueryModel(parent)
{
}

QVariant CUserSqlQueryModel::data(const QModelIndex &index, int role) const
{
    auto value = QSqlQueryModel::data(index, role);
    if(role == Qt::DisplayRole)
    {
        //User role -> switch enum value to QString
        if(index.column() == 4)
        {

            int userRole = value.toInt();
            QVariant userRoleValue = QVariant::fromValue(Utils::User::getUserRoleName(userRole));
            return userRoleValue;
        }
    }
    return value;
}
