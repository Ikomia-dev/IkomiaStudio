#ifndef CDATASETDBMGR_H
#define CDATASETDBMGR_H

#include "CProjectDbMgrInterface.hpp"

class CDatasetItemDbMgr: public CProjectItemBaseDbMgr
{
    public:

        CDatasetItemDbMgr();
        CDatasetItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;

    private:

        void                    createTables();

        void                    loadDataTypes();

    private:

        bool            m_bDataTypeLoaded = false;
        QMap<int, int>  m_mapTypes;
};

class CDatasetItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CDatasetItemDbMgrFactory()
        {
            m_type = TreeItemType::DATASET;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CDatasetItemDbMgr>(path, connection);
        }
};

#endif // CDATASETDBMGR_H
