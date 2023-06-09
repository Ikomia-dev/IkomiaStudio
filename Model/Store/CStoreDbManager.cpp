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

#include "CStoreDbManager.h"
#include <QSqlError>
#include "CException.h"
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"

CStoreDbManager::CStoreDbManager()
{
    m_currentOS = Utils::OS::getCurrent();
}

void CStoreDbManager::initDb()
{
    createServerPluginsDb(m_hubConnectionName);
    createServerPluginsDb(m_workspaceConnectionName);
}

QSqlDatabase CStoreDbManager::getPluginsDatabase(CPluginModel::Type type) const
{
    auto db = Utils::Database::connect(m_name, getDbConnectionName(type));
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    return db;
}

QString CStoreDbManager::getAllPluginsQuery(CPluginModel::Type type) const
{
    QString query;
    switch(type)
    {
        case CPluginModel::Type::HUB:
        case CPluginModel::Type::WORKSPACE:
            query = "SELECT * FROM serverPlugins;";
            break;
        case CPluginModel::Type::LOCAL:
            query = "SELECT * FROM process WHERE isInternal=False;";
            break;
    }
    return query;
}

QString CStoreDbManager::getSearchQuery(CPluginModel::Type type, const QString &searchText) const
{
    QString query;
    QString searchKey = Utils::Database::getFTSKeywords(searchText);

    switch(type)
    {
        case CPluginModel::Type::HUB:
        case CPluginModel::Type::WORKSPACE:
            query = QString("SELECT sp.* FROM serverPlugins sp INNER JOIN serverPluginsFTS spFts ON sp.name = spFts.name "
                            "WHERE serverPluginsFTS MATCH '%1';").arg(searchKey);
            break;
        case CPluginModel::Type::LOCAL:
            query = QString("SELECT p.* FROM process p INNER JOIN processFTS pFts ON p.name = pFts.name "
                            "WHERE p.isInternal=False AND processFTS MATCH '%1';").arg(searchKey);
            break;
    }
    return query;
}

void CStoreDbManager::insertPlugins(CPluginModel* pModel)
{
    auto db = Utils::Database::connect(m_name, getDbConnectionName(pModel->getType()));
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Retrieve plugins information from JSON
    /*QVariantList names, shortDescriptions, descriptions, keywords, userNames, authors, articles,
            journals, years, docLinks, createdDates, modifiedDates, versions, ikomiaVersions, languages,
            licenses, repositories, iconPaths, certifications, userIds, userReputations, votes;*/
    QVariantList names, shortDescriptions, descriptions, keywords, authors, articles,
            journals, years, versions, languages, licenses, repositories, iconPaths;

    QJsonArray plugins = pModel->getJsonPlugins();
    for(int i=0; i<plugins.size(); ++i)
    {
        QJsonObject plugin = plugins[i].toObject();

        // Name
        names << plugin["name"].toString();
        // Short description
        shortDescriptions << plugin["short_description"].toString();
        // Description
        descriptions << plugin["description"].toString();

        // Keywords
        QJsonArray jsonKeywords = plugin["keywords"].toArray();
        if (jsonKeywords.size() > 0)
        {
            QString strKeywords = jsonKeywords[0].toString();
            for (int j=1; j<jsonKeywords.size(); ++j)
                strKeywords += "," + jsonKeywords[j].toString();

            keywords << strKeywords;
        }

        // Paper
        QJsonObject jsonPaper = plugin["paper"].toObject();
        authors << jsonPaper["authors"].toString();
        articles << jsonPaper["title"].toString();
        journals << jsonPaper["journal"].toString();
        years << jsonPaper["year"].toInt();

        // TODO: missing fields
        //docLinks << plugin["docLink"].toString();
        //createdDates << plugin["createdDate"].toString();
        //modifiedDates << plugin["modifiedDate"].toString();

        // Version
        if (plugin.contains("version"))
            versions << plugin["version"].toString();
        else
            versions << "";

        // TODO: to remove -> check compatibility
        //ikomiaVersions << plugin["ikomiaVersion"].toString();

        // Language
        languages << pModel->getLanguageFromString(plugin["language"].toString());

        // License
        if (plugin.contains("license"))
            licenses << plugin["license"].toString();
        else
            licenses << "";

        // Implementation repository
        repositories << plugin["repository"].toString();

        // Icon path
        iconPaths << plugin["icon_path"].toString();

        // TODO: missing fields
        //certifications << plugin["certification"].toInt();
        //votes << plugin["votes_count"].toInt();
    }

    //Insert to serverPlugins table
    QSqlQuery q(db);
    /*if(!q.prepare(QString("INSERT INTO serverPlugins ("
                          "name, shortDescription, description, keywords, user, authors, article, journal, "
                          "year, docLink, createdDate, modifiedDate, version, ikomiaVersion, language, license, "
                          "repository, iconPath, certification, votes, userId, userReputation) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);")))*/
    if(!q.prepare(QString("INSERT INTO serverPlugins ("
                          "name, shortDescription, description, keywords, authors, article, journal, "
                          "year, version, language, license, repository, iconPath) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);")))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    q.addBindValue(names);
    q.addBindValue(shortDescriptions);
    q.addBindValue(descriptions);
    q.addBindValue(keywords);
    q.addBindValue(authors);
    q.addBindValue(articles);
    q.addBindValue(journals);
    q.addBindValue(years);
    //q.addBindValue(docLinks);
    //q.addBindValue(createdDates);
    //q.addBindValue(modifiedDates);
    q.addBindValue(versions);
    //q.addBindValue(ikomiaVersions);
    q.addBindValue(languages);
    q.addBindValue(licenses);
    q.addBindValue(repositories);
    q.addBindValue(iconPaths);
    //q.addBindValue(certifications);
    //q.addBindValue(votes);
    //q.addBindValue(userIds);
    //q.addBindValue(userReputations);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Insert to FTS table
    QSqlQuery qFts(db);
    if(!qFts.prepare(QString("INSERT INTO serverPluginsFTS "
                             "(name, shortDescription, description, keywords, authors, article, journal) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?);")))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, qFts.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    qFts.addBindValue(names);
    qFts.addBindValue(shortDescriptions);
    qFts.addBindValue(descriptions);
    qFts.addBindValue(keywords);
    qFts.addBindValue(authors);
    qFts.addBindValue(articles);
    qFts.addBindValue(journals);

    if(!qFts.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, qFts.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CStoreDbManager::insertPlugin(const CTaskInfo &procInfo, const CUser &user)
{
    if(procInfo.m_os != OSType::ALL && m_currentOS != procInfo.m_os)
        throw CException(CoreExCode::INVALID_PARAMETER, "This algorithm ("+ procInfo.m_name +") is not built for your platform.",  __func__, __FILE__, __LINE__);

    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    auto strQuery = QString("INSERT INTO process "
                            "(name, shortDescription, description, keywords, user, authors, article, journal, year, docLink, "
                            "createdDate, modifiedDate, version, ikomiaVersion, minPythonVersion, language, license, repository, os, isInternal, iconPath) "
                            "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', %9, '%10', '%11', '%12', '%13', '%14', '%15', %16, '%17', '%18', %19, %20, '%21') "
                            "ON CONFLICT(name) DO UPDATE SET "
                            "shortDescription = excluded.shortDescription, "
                            "description = excluded.description, "
                            "keywords = excluded.keywords, "
                            "user = COALESCE(excluded.user, user), "
                            "authors = excluded.authors, "
                            "article = excluded.article, "
                            "journal = excluded.journal, "
                            "year = excluded.year, "
                            "docLink = excluded.docLink, "
                            "modifiedDate = excluded.modifiedDate, "
                            "version = excluded.version, "
                            "ikomiaVersion = excluded.ikomiaVersion, "
                            "minPythonVersion = excluded.minPythonVersion, "
                            "license = excluded.license, "
                            "repository = excluded.repository, "
                            "iconPath = excluded.iconPath;")
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_name)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_shortDescription)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_description)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_keywords)))
            .arg(QString::fromStdString(Utils::String::dbFormat(user.m_firstName.toStdString())))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_authors)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_article)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_journal)))
            .arg(procInfo.m_year)
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_docLink)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_createdDate)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_modifiedDate)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_version)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_minIkomiaVersion)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_minPythonVersion)))
            .arg(procInfo.m_language)
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_license)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_repo)))
            .arg(procInfo.m_os)
            .arg(false)
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_iconPath)));

    if(!q.exec(strQuery))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CStoreDbManager::removeRemotePlugin(const QString& pluginName)
{
    auto db = getPluginsDatabase(CPluginModel::Type::WORKSPACE);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    // Delete plugin from server database
    QSqlQuery q1(db);
    if(!q1.exec(QString("DELETE FROM serverPlugins WHERE name='%1'").arg(pluginName)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CStoreDbManager::removeLocalPlugin(const QString& pluginName)
{
    auto dbMemory = Utils::Database::connect(m_name, Utils::Database::getProcessConnectionName());
    if(dbMemory.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, dbMemory.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Update plugin in memory database with serverId -1 -> allows display in storeDlg
    QSqlQuery q1(dbMemory);
    if(!q1.exec(QString("UPDATE process SET serverId=-1 WHERE name='%1'").arg(pluginName)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    //Delete plugin in file database
    auto dbFile = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(dbFile.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, dbFile.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q2(dbFile);
    if(!q2.exec(QString("DELETE FROM process WHERE name='%1'").arg(pluginName)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CStoreDbManager::updateLocalPluginModifiedDate(int pluginId)
{
    QString modifiedDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString queryStr = QString("UPDATE process SET modifiedDate='%1' WHERE id=%2")
            .arg(modifiedDate)
            .arg(pluginId);

    //Update memory database
    auto dbMemory = Utils::Database::connect(m_name, Utils::Database::getProcessConnectionName());
    if(dbMemory.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, dbMemory.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q1(dbMemory);
    if(!q1.exec(queryStr))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Update file database
    auto dbFile = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(dbFile.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, dbFile.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q2(dbFile);
    if(!q2.exec(queryStr))
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CStoreDbManager::updateMemoryLocalPluginsInfo()
{
    //Get server certification, votes and user reputation for all plugins
    auto dbServer = Utils::Database::connect(m_name, m_hubConnectionName);

    QSqlQuery q1(dbServer);
    if(!q1.exec("SELECT id, certification, votes, userReputation FROM serverPlugins;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList serverIds, serverCertifications, serverVotes, userReputations;
    while(q1.next())
    {
        serverIds << q1.value("id");
        serverCertifications << q1.value("certification");
        serverVotes << q1.value("votes");
        userReputations << q1.value("userReputation");
    }

    //Update certification for installed plugins
    auto dbLocal = Utils::Database::connect(m_name, Utils::Database::getProcessConnectionName());

    QSqlQuery q2(dbLocal);
    q2.prepare(QString("UPDATE process SET "
                       "certification = IFNULL(?, certification), "
                       "votes = IFNULL(?, votes), "
                       "userReputation = IFNULL(?, userReputation) "
                       "WHERE serverId = ?;"));

    q2.addBindValue(serverCertifications);
    q2.addBindValue(serverVotes);
    q2.addBindValue(userReputations);
    q2.addBindValue(serverIds);

    if(!q2.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CStoreDbManager::clearServerRecords(CPluginModel::Type type)
{
    auto db = getPluginsDatabase(type);

    QSqlQuery q(db);
    if(!q.exec("DELETE FROM serverPlugins"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!q.exec("DELETE FROM serverPluginsFTS"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CStoreDbManager::createServerPluginsDb(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(m_type, connectionName);
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    db.setDatabaseName(m_name);

    if(!db.open())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec("CREATE TABLE serverPlugins ("
               "id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, shortDescription TEXT, description TEXT, keywords TEXT, "
               "authors TEXT, article TEXT, journal TEXT, year INTEGER, docLink TEXT, createdDate TEXT, "
               "modifiedDate TEXT, version TEXT, ikomiaVersion TEXT, license TEXT, repository TEXT, language INTEGER, "
               "os INTEGER, iconPath TEXT, certification INTEGER, votes INTEGER, userReputation INTEGER);"))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(!q.exec("CREATE VIRTUAL TABLE serverPluginsFTS USING fts5(name, shortDescription, description, keywords, authors, article, journal);"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

QString CStoreDbManager::getDbConnectionName(CPluginModel::Type type) const
{
    QString dbConnectionName;
    switch(type)
    {
        case CPluginModel::Type::HUB: dbConnectionName = m_hubConnectionName; break;
        case CPluginModel::Type::WORKSPACE: dbConnectionName = m_workspaceConnectionName; break;
        case CPluginModel::Type::LOCAL: dbConnectionName = Utils::Database::getProcessConnectionName(); break;
    }
    return dbConnectionName;
}

int CStoreDbManager::getLocalIdFromServerId(const QSqlDatabase &db, int serverId) const
{
    QSqlQuery q(db);
    if(!q.exec(QString("SELECT id FROM process WHERE serverId=%1").arg(serverId)))
        return -1;

    if(q.first())
        return q.value(0).toInt();
    else
        return -1;
}
