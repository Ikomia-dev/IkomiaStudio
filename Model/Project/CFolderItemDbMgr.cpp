#include "CFolderItemDbMgr.h"
#include "Model/Project/CFolderItem.hpp"

CFolderItemDbMgr::CFolderItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CFolderItemDbMgr::CFolderItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CFolderItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);
    auto name = q.record().value("name").toString().toStdString();
    return std::make_shared<CFolderItem>(name);
}

void CFolderItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(itemPtr);
    Q_UNUSED(dbId);
}

void CFolderItemDbMgr::batchSave()
{
}
