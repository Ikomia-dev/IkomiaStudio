#ifndef CDBMIGRATIONMANAGER_H
#define CDBMIGRATIONMANAGER_H

#include <string>
#include <QString>

class CDbManager
{
    public:

        CDbManager();

        void        init();

    private:

        void        initTables();

        QString     getVersionNumber(const QString &component);

        void        makeMigrations();

        void        storeCurrentVersion();

        void        executeSqlFile(const QString& path, QSqlDatabase& db);

    private:

        bool                        m_bFirstLaunch = true;
        const QMap<QString,QString> m_migrationMap = { {"0.3.0" , "migration0001.sql"} };
};

#endif // CDBMIGRATIONMANAGER_H
