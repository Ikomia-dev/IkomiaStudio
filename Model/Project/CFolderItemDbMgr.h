#ifndef CFOLDERITEMDBMGR_H
#define CFOLDERITEMDBMGR_H

#include "CProjectDbMgrInterface.hpp"

class CFolderItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CFolderItemDbMgr();
        CFolderItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;
};

class CFolderItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CFolderItemDbMgrFactory()
        {
            m_type = TreeItemType::FOLDER;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CFolderItemDbMgr>(path, connection);
        }
};

#endif // CFOLDERITEMDBMGR_H
