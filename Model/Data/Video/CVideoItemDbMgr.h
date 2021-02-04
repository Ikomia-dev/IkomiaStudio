#ifndef CVIDEOITEMDBMGR_H
#define CVIDEOITEMDBMGR_H

#include "Model/Project/CProjectDbMgrInterface.hpp"

class CVideoItem;

class CVideoItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CVideoItemDbMgr();
        CVideoItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex &previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;

    private:

        void                    createTables();

        void                    loadPaths();
        void                    loadProtocolVideoMap();

        void                    updatePath(QSqlDatabase& db);

    private:

        bool                    m_bPathLoaded = false;
        bool                    m_bProtocolVideoMapLoaded = false;
        QMap<int, QString>      m_mapPaths;
        QMap<int, QVector<int>> m_mapProtocolIds;
};

class CVideoItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CVideoItemDbMgrFactory()
        {
            m_type = TreeItemType::VIDEO;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CVideoItemDbMgr>(path, connection);
        }
};

#endif // CVIDEOITEMDBMGR_H
