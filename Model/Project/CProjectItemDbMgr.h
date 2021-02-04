#ifndef CPROJECTITEMDBMGR_H
#define CPROJECTITEMDBMGR_H

#include "CProjectDbMgrInterface.hpp"

class CProjectItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CProjectItemDbMgr();
        CProjectItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;
};

class CProjectItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CProjectItemDbMgrFactory()
        {
            m_type = TreeItemType::PROJECT;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CProjectItemDbMgr>(path, connection);
        }
};

#endif // CPROJECTITEMDBMGR_H
