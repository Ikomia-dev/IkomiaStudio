#ifndef CGRAPHICSDBMANAGER_H
#define CGRAPHICSDBMANAGER_H

#include "Model/Project/CProjectDbMgrInterface.hpp"

class CGraphicsLayer;

using GraphicsDbInfo = std::unordered_multimap<int, std::pair<int, QByteArray>>;

class CGraphicsDbManager: public CProjectItemBaseDbMgr
{
    public:

        CGraphicsDbManager();
        CGraphicsDbManager(const QString& connectionName);
        CGraphicsDbManager(const QString& path, const QString& connectionName);

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;

        GraphicsDbInfo          loadGraphics(size_t typeId, const QString& path);

        void                    remove(std::vector<int> layerIds);

    private:

        QSqlDatabase            connectDB();

        void                    createTables();

        int                     getLayerChildsCount(int layerId);

        void                    replaceLayer(CGraphicsLayer* pLayer, int oldLayerId, int newLayerId);

        void                    updateProjectId(int oldId, int newId);
};

class CGraphicsDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CGraphicsDbMgrFactory()
        {
            m_type = TreeItemType::GRAPHICS_LAYER;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CGraphicsDbManager>(path, connection);
        }
};

#endif // CGRAPHICSDBMANAGER_H
