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
