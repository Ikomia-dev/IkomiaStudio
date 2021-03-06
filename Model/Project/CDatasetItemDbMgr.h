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
