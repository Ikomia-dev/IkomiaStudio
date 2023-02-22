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

#ifndef CSTOREDBMANAGER_H
#define CSTOREDBMANAGER_H

#include <QSqlDatabase>
#include "Task/CTaskFactory.hpp"
#include "Model/User/CUser.h"

class CStoreDbManager
{
    public:

        CStoreDbManager();

        void            initDb();

        QSqlDatabase    getServerPluginsDatabase() const;
        QSqlDatabase    getLocalPluginsDatabase() const;
        QString         getAllServerPluginsQuery() const;
        QString         getAllLocalPluginsQuery() const;
        QString         getLocalSearchQuery(const QString& searchText) const;
        QString         getServerSearchQuery(const QString& searchText) const;

        void            setLocalPluginServerInfo(int pluginId, const QString name, int serverId, const CUser& user);

        void            insertPlugins(const QJsonArray& plugins);
        void            insertPlugin(int serverId, const CTaskInfo& procInfo, const CUser &user);

        void            removeRemotePlugin(const QString& pluginName);
        void            removeLocalPlugin(const QString& pluginName);

        void            updateLocalPluginModifiedDate(int pluginId);
        void            updateMemoryLocalPluginsInfo();

        void            clearServerRecords();

    private:

        void            createServerPluginsDb();

        int             getLocalIdFromServerId(const QSqlDatabase& db, int serverId) const;

        bool            checkPluginCompatibility(const QJsonObject& plugin) const;

    private:

        QString m_serverConnectionName = "ServerStoreConnection";
        QString m_name = ":memory:";
        QString m_type = "QSQLITE";
        int     m_currentOS = -1;
};

#endif // CSTOREDBMANAGER_H
