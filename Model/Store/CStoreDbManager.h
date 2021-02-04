#ifndef CSTOREDBMANAGER_H
#define CSTOREDBMANAGER_H

#include <QSqlDatabase>
#include "Core/CProcessFactory.hpp"
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
        void            insertPlugin(int serverId, const CProcessInfo& procInfo, const CUser &user);

        void            removeRemotePlugin(const QString& pluginName);
        void            removeLocalPlugin(const QString& pluginName);

        void            updateLocalPluginModifiedDate(int pluginId);
        void            updateMemoryLocalPluginsInfo();

        void            clearServerRecords();

    private:

        void            createServerPluginsDb();

        int             getLocalIdFromServerId(const QSqlDatabase& db, int serverId) const;

    private:

        QString m_serverConnectionName = "ServerStoreConnection";
        QString m_name = ":memory:";
        QString m_type = "QSQLITE";
        int     m_currentOS = -1;
};

#endif // CSTOREDBMANAGER_H
