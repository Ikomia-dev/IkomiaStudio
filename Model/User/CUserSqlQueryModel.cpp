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
