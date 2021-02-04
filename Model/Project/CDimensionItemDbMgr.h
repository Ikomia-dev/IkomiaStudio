#ifndef CDIMENSIONITEMDBMGR_H
#define CDIMENSIONITEMDBMGR_H

#include "CProjectDbMgrInterface.hpp"

class CDimensionItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CDimensionItemDbMgr();
        CDimensionItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;

    private:

        void                    createTables();

        void                    loadTypes();

    private:

        bool            m_bTypesLoaded = false;
        QMap<int, int>  m_mapTypes;

};

class CDimensionItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CDimensionItemDbMgrFactory()
        {
            m_type = TreeItemType::DIMENSION;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CDimensionItemDbMgr>(path, connection);
        }
};

#endif // CDIMENSIONITEMDBMGR_H
