#ifndef CLIVESTREAMITEMDBMGR_H
#define CLIVESTREAMITEMDBMGR_H

#include "Model/Project/CProjectDbMgrInterface.hpp"

class CLiveStreamItem;

class CLiveStreamItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CLiveStreamItemDbMgr();
        CLiveStreamItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>      load(const QSqlQuery& q, QModelIndex &previousIndex) override;
        void                        save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                        batchSave() override;

    private:

        void                        createTables();

        void                        loadPaths();
        void                        loadProtocolVideoMap();

    private:

        bool                    m_bPathLoaded = false;
        bool                    m_bProtocolVideoMapLoaded = false;
        QMap<int, QString>      m_mapPaths;
        QMap<int, QVector<int>> m_mapProtocolIds;
};

class CLiveStreamItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CLiveStreamItemDbMgrFactory()
        {
            m_type = TreeItemType::LIVE_STREAM;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CLiveStreamItemDbMgr>(path, connection);
        }
};
#endif // CLIVESTREAMITEMDBMGR_H
