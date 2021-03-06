// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CPROJECTITEMDBMANAGER_HPP
#define CPROJECTITEMDBMANAGER_HPP

#include "DesignPattern/CAbstractFactory.hpp"
#include "Model/Project/CProjectModel.h"

class CItem;

class CProjectItemBaseDbMgr
{
    public:

        CProjectItemBaseDbMgr()
        {
        }
        CProjectItemBaseDbMgr(const QString& path, const QString& connection)
        {
            m_dbPath = path;
            m_connection = connection;
        }
        virtual ~CProjectItemBaseDbMgr() {}

        //Method to create smart pointer of CItem based object from database (intervallic representation of graph)
        virtual std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) = 0;
        //Method to save CItem based object (limited to project template declaration) into database or memory structure (optimization)
        virtual void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) = 0;
        //Method use to save full set of attributes of item list filled in the save function (optional - optimization)
        virtual void                    batchSave() = 0;

        //Method to store project properties: current path, original path and export state
        void                            setProjectProperties(const QString& path, const QString& originalPath, bool bExported)
        {
            m_projectPath = path;
            m_projectOriginalPath = originalPath;
            m_bProjectExported = bExported;
        }

        bool                            isPathUpdateRequired() const
        {
            return (m_bProjectExported == true && m_projectPath != m_projectOriginalPath);
        }

    protected:

        QString m_connection = "ProjectMemoryDB";
        QString m_dbPath = ":memory:";
        bool    m_bTablesCreated = false;
        QString m_projectPath;
        QString m_projectOriginalPath;
        bool    m_bProjectExported = false;
};

using ProjectItemDbMgrPtr = std::shared_ptr<CProjectItemBaseDbMgr>;

class CProjectDbMgrFactory
{
    public:

        CProjectDbMgrFactory(){}
        virtual ~CProjectDbMgrFactory(){}

        TreeItemType                getType() const
        {
            return m_type;
        }

        virtual ProjectItemDbMgrPtr create(const QString& path, const QString& connection) = 0;

    protected:

        TreeItemType m_type = TreeItemType::PROJECT;
};

using ProjectDbMgrFactories = std::vector<std::shared_ptr<CProjectDbMgrFactory>>;

class CProjectDbMgrAbstractFactory: public CAbstractFactory<TreeItemType, ProjectItemDbMgrPtr, const QString, const QString>
{
    public:

        ProjectDbMgrFactories& getList()
        {
            return m_factories;
        }

    private:

        ProjectDbMgrFactories m_factories;
};


#endif // CPROJECTITEMDBMANAGER_HPP
