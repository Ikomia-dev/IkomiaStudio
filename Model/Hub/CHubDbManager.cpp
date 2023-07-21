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

#include "CHubDbManager.h"
#include <QSqlError>
#include "CException.h"
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"
#include "Main/CoreTools.hpp"

CHubDbManager::CHubDbManager()
{
    m_currentOS = Utils::OS::getCurrent();
}

void CHubDbManager::initDb()
{
    createServerPluginsDb(m_hubConnectionName);
    createServerPluginsDb(m_workspaceConnectionName);
}

QSqlDatabase CHubDbManager::getPluginsDatabase(CPluginModel::Type type) const
{
    auto db = Utils::Database::connect(m_name, getDbConnectionName(type));
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    return db;
}

QString CHubDbManager::getAllPluginsQuery(CPluginModel::Type type) const
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

QString CHubDbManager::getSearchQuery(CPluginModel::Type type, const QString &searchText) const
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

void CHubDbManager::insertPlugins(CPluginModel* pModel)
{
    auto db = Utils::Database::connect(m_name, getDbConnectionName(pModel->getType()));
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Retrieve plugins information from JSON
    QVariantList names, shortDescriptions, descriptions, keywords, users,
            authors, articles, articleUrls, journals, years, createdDates, modifiedDates,
            versions, minIkVersions, maxIkVersions, minPyVersions, maxPyVersions,
            languages, osList, iconPaths,
            licenses, repositories, originalRepositories, algoTypes, algoTasks;

    QJsonArray plugins = pModel->getJsonPlugins();
    for(int i=0; i<plugins.size(); ++i)
    {
        QJsonObject plugin = plugins[i].toObject();
        QJsonArray packages = plugin["packages"].toArray();
        QJsonObject package = packages[0].toObject();

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

        // Contributor
        QJsonObject contributor = plugin["author"].toObject();
        users << contributor["name"].toString();

        // Paper
        QJsonObject jsonPaper = plugin["paper"].toObject();
        authors << jsonPaper["authors"].toString();
        articles << jsonPaper["title"].toString();
        articleUrls << jsonPaper["link"].toString();
        journals << jsonPaper["journal"].toString();
        years << jsonPaper["year"].toInt();

        // TODO: missing fields
        //docLinks << plugin["docLink"].toString();

        // Dates
        createdDates << plugin["created_at"].toString();
        modifiedDates << plugin["updated_at"].toString();

        // Version
        if (plugin.contains("version"))
            versions << plugin["version"].toString();
        else
            versions << "";

        // Ikomia min and max versions of last package
        minIkVersions << package["ikomia_min_version"].toString();
        maxIkVersions << plugin["ikomia_max_version"].toString();

        // Python min and max versions of last package
        minPyVersions << package["python_min_version"].toString();
        maxPyVersions << plugin["python_max_version"].toString();

        // Language
        languages << pModel->getLanguageFromString(plugin["language"].toString());

        // OS
        QJsonObject platform = package["platform"].toObject();
        QJsonArray os = platform["os"].toArray();
        auto itLinux = std::find(os.begin(), os.end(), "LINUX");
        auto itWin = std::find(os.begin(), os.end(), "WINDOWS");

        if (itLinux != os.end() && itWin != os.end())
            osList << OSType::ALL;
        else if (itLinux != os.end())
            osList << OSType::LINUX;
        else
            osList << OSType::WIN;

        // License
        if (plugin.contains("license"))
            licenses << plugin["license"].toString();
        else
            licenses << "";

        // Icon path
        iconPaths << plugin["icon_path"].toString();

        // Implementation repository
        repositories << plugin["repository"].toString();
        originalRepositories << plugin["original_implementation_repository"].toString();

        // Algo type
        algoTypes << (int)(Utils::Plugin::getAlgoTypeFromString(plugin["algo_type"].toString().toStdString()));

        // Algo tasks
        algoTasks << plugin["algo_task"].toString();

        // TODO: missing fields
        //certifications << plugin["certification"].toInt();
        //votes << plugin["votes_count"].toInt();
    }

    //Insert to serverPlugins table
    QSqlQuery q(db);
    if(!q.prepare(QString("INSERT INTO serverPlugins ("
                          "name, shortDescription, description, keywords, user, "
                          "authors, article, articleUrl, journal, year, createdDate, modifiedDate, "
                          "version, minIkomiaVersion, maxIkomiaVersion, minPythonVersion, maxPythonVersion, "
                          "language, os, iconPath, "
                          "license, repository,  originalRepository, algoType, algoTasks) "
                          "VALUES ("
                          "?, ?, ?, ?, ?, "
                          "?, ?, ?, ?, ?, ?, ?, "
                          "?, ?, ?, ?, ?,"
                          "?, ? ,?,"
                          "?, ?, ?, ?, ?);")))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    q.addBindValue(names);
    q.addBindValue(shortDescriptions);
    q.addBindValue(descriptions);
    q.addBindValue(keywords);
    q.addBindValue(users);
    q.addBindValue(authors);
    q.addBindValue(articles);
    q.addBindValue(articleUrls);
    q.addBindValue(journals);
    q.addBindValue(years);
    //q.addBindValue(docLinks);
    q.addBindValue(createdDates);
    q.addBindValue(modifiedDates);
    q.addBindValue(versions);
    q.addBindValue(minIkVersions);
    q.addBindValue(maxIkVersions);
    q.addBindValue(minPyVersions);
    q.addBindValue(maxPyVersions);
    q.addBindValue(languages);
    q.addBindValue(osList);
    q.addBindValue(iconPaths);
    q.addBindValue(licenses);
    q.addBindValue(repositories);
    q.addBindValue(originalRepositories);
    q.addBindValue(algoTypes);
    q.addBindValue(algoTasks);
    //q.addBindValue(certifications);
    //q.addBindValue(votes);
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

void CHubDbManager::insertPlugin(const CTaskInfo &procInfo, const CUser &user)
{
    if(procInfo.m_os != OSType::ALL && m_currentOS != procInfo.m_os)
        throw CException(CoreExCode::INVALID_PARAMETER, "This algorithm ("+ procInfo.m_name +") is not built for your platform.",  __func__, __FILE__, __LINE__);

    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    auto strQuery = QString("INSERT INTO process "
                            "(name, shortDescription, description, keywords, user, "
                            "authors, article, articleUrl, journal, year, docLink, createdDate, modifiedDate, "
                            "version, minIkomiaVersion, maxIkomiaVersion, minPythonVersion, maxPythonVersion, "
                            "language, os, iconPath, "
                            "license, repository,  originalRepository, algoType, algoTasks) "
                            "VALUES ('%1', '%2', '%3', '%4', '%5', "
                            "'%6', '%7', '%8', '%9', %10, '%11', '%12', '%13', "
                            "'%14', '%15', %16, '%17', '%18', "
                            "%19, %20, '%21'"
                            "'%22', '%23', '%24', %25, '%26') "
                            "ON CONFLICT(name) DO UPDATE SET "
                            "shortDescription = excluded.shortDescription, "
                            "description = excluded.description, "
                            "keywords = excluded.keywords, "
                            "user = COALESCE(excluded.user, user), "
                            "authors = excluded.authors, "
                            "article = excluded.article, "
                            "articleUrl = excluded.articleUrl, "
                            "journal = excluded.journal, "
                            "year = excluded.year, "
                            "docLink = excluded.docLink, "
                            "createdDate = excluded.createdDate, "
                            "modifiedDate = excluded.modifiedDate, "
                            "version = excluded.version, "
                            "minIkomiaVersion = excluded.minIkomiaVersion, "
                            "maxIkomiaVersion = excluded.maxIkomiaVersion, "
                            "minPythonVersion = excluded.minPythonVersion, "
                            "maxPythonVersion = excluded.maxPythonVersion, "
                            "language = excluded.language, "
                            "os = excluded.os, "
                            "iconPath = excluded.iconPath, "
                            "license = excluded.license, "
                            "repository = excluded.repository, "
                            "originalRepository = excluded.originalRepository, "
                            "algoType = excluded.algoType, "
                            "algoTasks = excluded.algoTasks;")
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_name)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_shortDescription)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_description)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_keywords)))
            .arg(QString::fromStdString(Utils::String::dbFormat("")))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_authors)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_article)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_articleUrl)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_journal)))
            .arg(procInfo.m_year)
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_docLink)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_createdDate)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_modifiedDate)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_version)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_minIkomiaVersion)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_maxIkomiaVersion)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_minPythonVersion)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_maxPythonVersion)))
            .arg(procInfo.m_language)
            .arg(procInfo.m_os)
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_iconPath)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_license)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_repo)))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_originalRepo)))
            .arg((int)(procInfo.m_algoType))
            .arg(QString::fromStdString(Utils::String::dbFormat(procInfo.m_algoTasks)));

    if(!q.exec(strQuery))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CHubDbManager::updateLocalPluginModifiedDate(const QString pluginName)
{
    QString modifiedDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString queryStr = QString("UPDATE process SET modifiedDate='%1' WHERE name='%2'")
            .arg(modifiedDate)
            .arg(pluginName);

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

void CHubDbManager::updateMemoryLocalPluginsInfo()
{
    //Get server certification, votes and user reputation for all plugins
    auto dbServer = Utils::Database::connect(m_name, m_hubConnectionName);

    QSqlQuery q1(dbServer);
    if(!q1.exec("SELECT name, certification, votes, userReputation FROM serverPlugins;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList serverNames, serverCertifications, serverVotes, userReputations;
    while(q1.next())
    {
        serverNames << q1.value("name");
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
                       "WHERE name = ?;"));

    q2.addBindValue(serverCertifications);
    q2.addBindValue(serverVotes);
    q2.addBindValue(userReputations);
    q2.addBindValue(serverNames);

    if(!q2.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CHubDbManager::clearServerRecords(CPluginModel::Type type)
{
    auto db = getPluginsDatabase(type);

    QSqlQuery q(db);
    if(!q.exec("DELETE FROM serverPlugins"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!q.exec("DELETE FROM serverPluginsFTS"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CHubDbManager::createServerPluginsDb(const QString &connectionName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(m_type, connectionName);
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    db.setDatabaseName(m_name);

    if(!db.open())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec("CREATE TABLE serverPlugins ("
               "id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, shortDescription TEXT, description TEXT, keywords TEXT, user TEXT, "
               "authors TEXT, article TEXT, articleUrl TEXT, journal TEXT, year INTEGER, docLink TEXT, createdDate TEXT, modifiedDate TEXT, "
               "version TEXT, minIkomiaVersion TEXT, maxIkomiaVersion TEXT, minPythonVersion TEXT, maxPythonVersion TEXT, "
               "language INTEGER, os INTEGER, iconPath TEXT, certification INTEGER, votes INTEGER, "
               "license TEXT, repository TEXT, originalRepository TEXT, algoType INTEGER, algoTasks TEXT, userReputation INTEGER);"))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(!q.exec("CREATE VIRTUAL TABLE serverPluginsFTS USING fts5(name, shortDescription, description, keywords, authors, article, journal);"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

QString CHubDbManager::getDbConnectionName(CPluginModel::Type type) const
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
