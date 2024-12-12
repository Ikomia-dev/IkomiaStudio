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

#ifndef CHUBDBMANAGER_H
#define CHUBDBMANAGER_H

#include <QSqlDatabase>
#include "Model/User/CUser.h"
#include "Task/CTaskInfo.h"
#include "CPluginModel.h"

class CHubDbManager
{
    public:

        CHubDbManager();

        void            initDb();

        QSqlDatabase    getPluginsDatabase(CPluginModel::Type type) const;
        QString         getAllPluginsQuery(CPluginModel::Type type) const;
        QString         getSearchQuery(CPluginModel::Type type, const QString& searchText) const;

        void            insertPlugins(CPluginModel *pModel);
        void            insertPlugin(const CTaskInfo& procInfo, const CUser &user);

        void            updateLocalPluginModifiedDate(const QString pluginName);
        void            updateMemoryLocalPluginsInfo();

        void            clearServerRecords(CPluginModel::Type type);

    private:

        void            createServerPluginsDb(const QString& connectionName);

        QString         getDbConnectionName(CPluginModel::Type type) const;

    private:

        QString m_hubConnectionName = "HubConnection";
        QString m_workspaceConnectionName = "WorkspaceConnection";
        QString m_name = ":memory:";
        QString m_type = "QSQLITE";
        OSType  m_currentOS = OSType::UNDEFINED;
};

#endif // CHUBDBMANAGER_H
