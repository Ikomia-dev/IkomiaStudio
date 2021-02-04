#include <QSqlRecord>
#include "CProjectItemDbMgr.h"
#include "CProjectItem.hpp"

CProjectItemDbMgr::CProjectItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CProjectItemDbMgr::CProjectItemDbMgr(const QString& path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CProjectItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);
    auto name = q.record().value("name").toString().toStdString();
    return std::make_shared<CProjectItem>(name);
}

void CProjectItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(itemPtr);
    Q_UNUSED(dbId);
}

void CProjectItemDbMgr::batchSave()
{
}


