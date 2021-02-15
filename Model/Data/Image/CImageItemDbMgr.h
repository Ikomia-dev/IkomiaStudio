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

#ifndef CIMAGEITEMDBMGR_H
#define CIMAGEITEMDBMGR_H

#include "Model/Project/CProjectDbMgrInterface.hpp"

class CImageItem;

class CImageItemDbMgr : public CProjectItemBaseDbMgr
{
    public:

        CImageItemDbMgr();
        CImageItemDbMgr(const QString& path, const QString& connection);

        std::shared_ptr<CItem>      load(const QSqlQuery& q, QModelIndex &previousIndex) override;
        void                        save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                        batchSave() override;

    private:

        void                        createTables();

        void                        loadPaths();
        void                        loadProtocolImageMap();

        std::shared_ptr<CImageItem> addImagesToDataset(QModelIndex &datasetItemIndex, const QString &file);
        std::shared_ptr<CImageItem> addImagesToDimension(QModelIndex &itemIndex, const QString &file);

        void                        updatePath(QSqlDatabase &db);

    private:

        bool                        m_bPathLoaded = false;
        bool                        m_bProtocolImageMapLoaded = false;
        QMap<int, QString>          m_mapPaths;
        QMap<int, QVector<int>>     m_mapProtocolIds;
};

class CImageItemDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CImageItemDbMgrFactory()
        {
            m_type = TreeItemType::IMAGE;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CImageItemDbMgr>(path, connection);
        }
};

#endif // CIMAGEITEMDBMGR_H
