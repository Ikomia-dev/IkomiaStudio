/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
