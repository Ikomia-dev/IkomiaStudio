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

#ifndef CDBMIGRATIONMANAGER_H
#define CDBMIGRATIONMANAGER_H

#include <string>
#include <QString>
#include <QMap>
#include <QSqlDatabase>

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
