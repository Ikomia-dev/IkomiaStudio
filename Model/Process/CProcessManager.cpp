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

#include "CProcessManager.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "UtilsTools.hpp"
#include <QTreeView>
#include <QDialog>
#include "Model/Plugin/CPluginManager.h"
#include "Model/Protocol/CProtocolManager.h"

CProcessManager::CProcessManager()
{  
}

CProcessManager::~CProcessManager()
{
}

void CProcessManager::init()
{
    //Python must be initialize before this call
    initModel();
    createModel();
}

void CProcessManager::setManagers(CPluginManager *pPluginMgr, CProtocolManager* pProtocolMgr)
{
    m_pPluginMgr = pPluginMgr;
    m_pProtocolMgr = pProtocolMgr;
}

void CProcessManager::setCurrentUser(const CUser &user)
{
    Q_UNUSED(user);
    reloadAll();
}

CProcessModel* CProcessManager::getProcessModel(CProcessManager::ID id)
{
    return m_processList[id].get();
}

CProcessInfo CProcessManager::getProcessInfo(const std::string &processName) const
{
    CProcessInfo info;

    auto db = Utils::Database::connect(m_dbName, Utils::Database::getProcessConnectionName());
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT * FROM process WHERE name='%1';")
               .arg(QString::fromStdString(Utils::String::dbFormat(processName)))))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(q.next())
    {
        info.m_id = q.value("id").toInt();
        info.m_name = q.value("name").toString().toStdString();
        info.m_shortDescription = q.value("shortDescription").toString().toStdString();
        info.m_description = q.value("description").toString().toStdString();
        info.m_docLink = q.value("docLink").toString().toStdString();
        info.m_iconPath = q.value("iconPath").toString().toStdString();
        info.m_keywords = q.value("keywords").toString().toStdString();
        info.m_authors = q.value("authors").toString().toStdString();
        info.m_article = q.value("article").toString().toStdString();
        info.m_journal = q.value("journal").toString().toStdString();
        info.m_version = q.value("version").toString().toStdString();
        info.m_ikomiaVersion = q.value("ikomiaVersion").toString().toStdString();
        info.m_license = q.value("license").toString().toStdString();
        info.m_repo = q.value("repository").toString().toStdString();
        info.m_createdDate = q.value("createdDate").toString().toStdString();
        info.m_modifiedDate = q.value("modifiedDate").toString().toStdString();
        info.m_year = q.value("year").toInt();
        info.m_language = q.value("language").toInt();
        info.m_bInternal = q.value("isInternal").toInt();
        info.m_userId = q.value("userId").toInt();
        info.m_os = q.value("os").toInt();
    }
    return info;
}

void CProcessManager::notifyViewShow()
{
    notifyModelUpdate();
}

void CProcessManager::notifyModelUpdate()
{  
    if(m_proxyList[m_processId])
        m_proxyList[m_processId]->deleteLater();

    m_proxyList[m_processId] = new CProcessProxyModel;
    m_proxyList[m_processId]->setSourceModel(m_processList[m_processId].get());

    emit doSetProcessModel(m_proxyList[m_processId]);
}

void CProcessManager::notifyTableModelUpdate()
{
    if(m_pTableProxyModel)
        m_pTableProxyModel->deleteLater();

    m_pTableProxyModel = new CProcessTableProxyModel;
    m_pTableProxyModel->setProcessModel(m_pModel);
    m_pTableProxyModel->setSourceModel(m_processTableList[m_processId].get());

    emit doSetTableModel(m_pTableProxyModel);
}

void CProcessManager::updateTableModelQuery()
{
    for(const auto& id : m_viewIds)
        m_processTableList[id]->select();
}

void CProcessManager::updateProcessInfo(const CProcessInfo &info)
{
    makeCurrentDb();
    QSqlQuery q(m_db);

    //Internal Process or plugin
    bool bInternal = info.isInternal();
    //Get process language C++ or Python
    int language = info.getLanguage();

    //Icon path
    QString iconPath = QString::fromStdString(info.getIconPath());
    if(iconPath.isEmpty() == false && bInternal == false)
    {
        QString pluginDir = QString::fromStdString(Utils::CPluginTools::getDirectory(info.getName(), language));
        iconPath = pluginDir + "/" + iconPath;
    }

    auto name = QString::fromStdString(Utils::String::dbFormat(info.getName()));
    auto description = QString::fromStdString(Utils::String::dbFormat(info.getDescription()));
    auto keywords = QString::fromStdString(Utils::String::dbFormat(info.getKeywords()));
    auto authors = QString::fromStdString(Utils::String::dbFormat(info.getAuthors()));
    auto article = QString::fromStdString(Utils::String::dbFormat(info.getArticle()));
    auto journal = QString::fromStdString(Utils::String::dbFormat(info.getJournal()));
    auto docLink = QString::fromStdString(Utils::String::dbFormat(info.getDocumentationLink()));
    auto version = QString::fromStdString(Utils::String::dbFormat(info.getVersion()));
    auto license = QString::fromStdString(Utils::String::dbFormat(info.getLicense()));
    auto repo = QString::fromStdString(Utils::String::dbFormat(info.getRepository()));

    if (!q.exec(QString("UPDATE process "
                        "SET description='%1', keywords='%2', authors='%3', article='%4', journal='%5', "
                        "year=%6, docLink='%7', version='%8', iconPath='%9', license='%10', repository='%11' "
                        "WHERE name = '%12';")
                .arg(description).arg(keywords).arg(authors).arg(article).arg(journal)
                .arg(info.getYear()).arg(docLink).arg(version).arg(iconPath).arg(license).arg(repo).arg(name)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if (!q.exec(QString("UPDATE processFTS "
                        "SET description='%1', keywords='%2', authors='%3', article='%4', journal='%5' "
                        "WHERE name = '%6';")
                .arg(description).arg(keywords).arg(authors).arg(article).arg(journal).arg(name)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

ProtocolTaskPtr CProcessManager::createObject(const std::string &processName, const ProtocolTaskParamPtr& pParam)
{
    ProtocolTaskPtr taskPtr = nullptr;

    try
    {
        auto factory = m_processRegistrator.getProcessFactory();
        taskPtr = factory.createObject(processName, std::move(pParam));
    }
    catch(boost::python::error_already_set&)
    {
        qCCritical(logProcess).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
    }
    catch(std::exception& e)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(e.what());
    }
    return taskPtr;
}

ProtocolTaskWidgetPtr CProcessManager::createWidget(const std::string &processName, const ProtocolTaskParamPtr &pParam)
{
    ProtocolTaskWidgetPtr widgetPtr = nullptr;

    try
    {
        auto factory = m_processRegistrator.getWidgetFactory();
        widgetPtr = factory.createObject(processName, std::move(pParam));
    }
    catch(boost::python::error_already_set&)
    {
        qCCritical(logProcess).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
    }
    catch(std::exception& e)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(e.what());
    }
    return widgetPtr;
}

void CProcessManager::resetModel()
{
    m_id = 0;
    m_processRegistrator.reset();
    m_ioRegistrator.reset();

    for(const auto& id : m_viewIds)
    {
        m_processList[id] = std::make_shared<CProcessModel>();
        m_processList[id]->insertIcon(static_cast<size_t>(TreeItemType::FOLDER), QIcon(":/Images/folder.png"));
        m_processList[id]->insertIcon(static_cast<size_t>(TreeItemType::PROCESS), QIcon(":/Images/default-process.png"));
    }
    setModel(m_processList[m_processId].get());
}

void CProcessManager::reloadAll()
{
    assert(m_pPluginMgr);
    assert(m_pProtocolMgr);

    resetModel();
    m_pPluginMgr->loadProcessPlugins();
    createModel();
    notifyModelUpdate();
    notifyTableModelUpdate();
    m_pPluginMgr->notifyPluginsLoaded();
    m_pProtocolMgr->onAllProcessReloaded();
    emit doOnAllProcessReloaded();
}

void CProcessManager::onReloadAllPlugins()
{
    reloadAll();
}

void CProcessManager::onReloadPlugin(const QString pluginName, int language)
{
    bool bExist = m_pPluginMgr->isProcessExists(pluginName);

    try
    {
        if(bExist == false)
            reloadAll();
        else
        {
            ProcessFactoryPtr factoryPtr = m_pPluginMgr->loadProcessPlugin(pluginName, language);
            if(factoryPtr)
            {
                assert(m_pPluginMgr);
                assert(m_pProtocolMgr);
                updateProcessInfo(factoryPtr->getInfo());
                notifyModelUpdate();
                notifyTableModelUpdate();
                m_pPluginMgr->notifyPluginsLoaded();
                m_pProtocolMgr->onProcessReloaded(pluginName);
                emit doOnProcessReloaded(pluginName);
            }
        }
    }
    catch(std::exception& e)
    {
        qCCritical(logProcess).noquote() << e.what();
    }
}

void CProcessManager::onSearchProcess(const QString& text)
{
    try
    {
        updateModelFromSearch(text);
    }
    catch(std::exception& e)
    {
        qCCritical(logProcess).noquote() << e.what();
    }
}

void CProcessManager::onSetCurrentProcessModel(CProcessManager::ID id)
{
    setCurrentProcessModel(id);
}

void CProcessManager::onUpdateTableModel(const QModelIndex& index)
{
    auto pModel = static_cast<const CProcessProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pItem = static_cast<CProcessModel::TreeItem*>(srcIndex.internalPointer());

    if(pItem->getTypeId() == TreeItemType::PROCESS)
    {
        srcIndex = srcIndex.parent();
        pItem = static_cast<CProcessModel::TreeItem*>(srcIndex.internalPointer());
    }
    auto id = pItem->getDbId();

    // Set correct DB before query
    makeCurrentDb();

    QSqlQuery q(m_db);
    try
    {
        getAllLeafFrom(q, id);
    }
    catch(std::exception& e)
    {
        qCCritical(logProcess).noquote() << e.what();
    }

    QString req = "id in (";
    while(q.next())
        req += q.value(0).toString() + ",";

    req.chop(1);
    req += ")";

    // We must take into account when one searches process in order to update the process listView popup
    if(m_bIsSearching)
        req += " AND " + m_searchReq;

    m_processTableList[m_processId]->setFilter(req);
    m_processTableList[m_processId]->select();
    notifyTableModelUpdate();
}

void CProcessManager::onSearchTableProcess(const QString& text)
{
    if(text.isEmpty())
    {
        m_processTableList[m_processId]->setTable("process");
        m_processTableList[m_processId]->select();
        m_bIsSearching = false;
    }
    else
    {
        QString txtKey = Utils::Database::getFTSKeywords(text);
        m_searchReq = QString("id in (select id from processFTS where processFTS MATCH '%1');").arg(txtKey);
        m_processTableList[m_processId]->setFilter(m_searchReq);
        m_processTableList[m_processId]->select();
        m_bIsSearching = true;
    }
    notifyTableModelUpdate();
}

void CProcessManager::onQueryWidgetInstance(const std::string &processName)
{
    auto widgetPtr = createWidget(processName, nullptr);
    if(widgetPtr)
        emit doSetWidgetInstance(processName, widgetPtr);
}

void CProcessManager::onQueryProcessInfo(const std::string &processName)
{
    auto info = getProcessInfo(processName);
    emit doSetProcessInfo(info);
}

void CProcessManager::onUpdateProcessInfo(bool bFullEdit, const CProcessInfo &info)
{
    //Db string format check
    QString shortDescription = QString::fromStdString(Utils::String::dbFormat(info.m_shortDescription));
    QString description = QString::fromStdString(Utils::String::dbFormat(info.m_description));
    QString keywords = QString::fromStdString(Utils::String::dbFormat(info.m_keywords));
    QString authors = QString::fromStdString(Utils::String::dbFormat(info.m_authors));
    QString article = QString::fromStdString(Utils::String::dbFormat(info.m_article));
    QString journal = QString::fromStdString(Utils::String::dbFormat(info.m_journal));
    QString docLink = QString::fromStdString(Utils::String::dbFormat(info.m_docLink));
    QString version = QString::fromStdString(Utils::String::dbFormat(info.m_version));
    QString ikomiaVersion = QString::fromStdString(Utils::String::dbFormat(info.m_ikomiaVersion));
    QString iconPath = QString::fromStdString(Utils::String::dbFormat(info.m_iconPath));
    QString license = QString::fromStdString(Utils::String::dbFormat(info.m_license));
    QString repository = QString::fromStdString(Utils::String::dbFormat(info.m_repo));

    //Update memory database
    if(bFullEdit == true)
    {
        QSqlQuery q(m_db);

        //Table process
        if(!q.exec(QString("UPDATE process SET "
                           "shortDescription = '%1', description = '%2', keywords = '%3', authors = '%4', article = '%5', "
                           "journal = '%6', year = %7, docLink = '%8', version = '%9', iconPath = '%10', os = %11 , license = '%12', "
                           "repository = '%13', ikomiaVersion = %14 "
                           "WHERE id = %15;")
                .arg(shortDescription).arg(description).arg(keywords).arg(authors).arg(article).arg(journal).arg(info.m_year)
                .arg(docLink).arg(version).arg(iconPath).arg(info.m_os).arg(license).arg(repository).arg(ikomiaVersion).arg(info.m_id)))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        //Table processFTS (Full Text Search)
        if(!q.exec(QString("UPDATE processFTS SET "
                           "shortDescription = '%1', description = '%2', keywords = '%3', authors = '%4', article = '%5', journal = '%6' "
                           "WHERE id = %7;")
                .arg(shortDescription).arg(description).arg(keywords).arg(authors).arg(article).arg(journal).arg(info.m_id)))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }
    else
    {
        QSqlQuery q(m_db);

        //Table process
        if(!q.exec(QString("UPDATE process SET keywords = '%1' WHERE id = %2;")
                .arg(keywords).arg(info.m_id)))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        //Table processFTS (Full Text Search)
        if(!q.exec(QString("UPDATE processFTS SET keywords = '%1' WHERE id = %2;")
                .arg(keywords).arg(info.m_id)))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }

    //Insert or update in file database
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QString strQuery;
    if(bFullEdit == true)
    {
        strQuery = QString("INSERT INTO process "
                           "(name, shortDescription, description, keywords, authors, article, journal, year, docLink, version, ikomiaVersion, iconPath, os, license, repository) "
                           "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', %8, '%9', '%10', '%11', %12, '%13', '%14', '%15') "
                           "ON CONFLICT(name) DO UPDATE SET "
                           "shortDescription = excluded.shortDescription, "
                           "description = excluded.description, "
                           "keywords = excluded.keywords, "
                           "authors = excluded.authors, "
                           "article = excluded.article, "
                           "journal = excluded.journal, "
                           "year = excluded.year, "
                           "docLink = excluded.docLink, "
                           "version = excluded.version, "
                           "ikomiaVersion = excluded.ikomiaVersion, "
                           "iconPath = excluded.iconPath, "
                           "os = excluded.os, "
                           "license = excluded.license, "
                           "repository = excluded.repository;")
                .arg(QString::fromStdString(info.m_name)).arg(description).arg(keywords).arg(authors)
                .arg(article).arg(journal).arg(info.m_year).arg(docLink).arg(version).arg(ikomiaVersion)
                .arg(iconPath).arg(info.m_os).arg(license).arg(repository);
    }
    else
    {
        strQuery = QString("INSERT INTO process "
                           "(name, keywords) "
                           "VALUES ('%1', '%2') "
                           "ON CONFLICT(name) DO UPDATE SET "
                           "keywords = excluded.keywords;")
                .arg(QString::fromStdString(info.m_name)).arg(keywords);
    }

    QSqlQuery q2(db);
    if(!q2.exec(strQuery))
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    updateTableModelQuery();
}

void CProcessManager::fillModel(QSqlQuery query)
{
    while (query.next())
    {
        int typeId = query.record().value("typeid").toInt();
        switch(typeId)
        {
            case static_cast<int>(TreeItemType::NONE):
                addCustomTreeItem<CFolderItem, CFolderItemImpl>(query);
                break;

            case static_cast<int>(TreeItemType::FOLDER):
                addCustomTreeItem<CFolderItem, CFolderItemImpl>(query);
                break;

            case static_cast<int>(TreeItemType::PROCESS):
                addCustomTreeItem<CProcessItem, CProcessItemImpl>(query);
                break;

            default:
                break;
        }
    }
}

void CProcessManager::updateModelFromSearch(const QString& text)
{
    // Reset tree model before filling it with new research
    m_id = 0;
    m_processList[ID::PROCESS_TREE] = std::make_shared<CProcessModel>();
    m_processList[ID::PROCESS_TREE]->insertIcon(static_cast<size_t>(TreeItemType::FOLDER), QIcon(":/Images/folder.png"));
    m_processList[ID::PROCESS_TREE]->insertIcon(static_cast<size_t>(TreeItemType::PROCESS), QIcon(":/Images/default-process.png"));

    setModel(m_processList[m_processId].get());

    // Set correct DB before query
    makeCurrentDb();
    QSqlQuery q(m_db);
    QList<int> idList;
    bool bSearch = false;

    if(text.isEmpty() == false)
    {
        bSearch = true;
        QString txtKey = Utils::Database::getFTSKeywords(text);

        if(!q.exec(QString("SELECT id FROM processFTS WHERE processFTS MATCH '%1';").arg(txtKey)))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while (q.next())
        {
            int id = q.value(0).toInt();
            idList.append(id);
        }
    }   

    if(!q.exec(QString("SELECT name, level, id, typeid, parentid FROM %1 ORDER BY left;").arg(m_currentTreeName)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    while (q.next())
    {
        int typeId = q.record().value("typeid").toInt();
        switch(typeId)
        {
            case static_cast<int>(TreeItemType::NONE):
                addCustomTreeItem<CFolderItem, CFolderItemImpl>(q);
                break;

            case static_cast<int>(TreeItemType::FOLDER):
                addCustomTreeItem<CFolderItem, CFolderItemImpl>(q);
                break;

            case static_cast<int>(TreeItemType::PROCESS):
            {
                if(bSearch == true)
                {
                    int id = q.record().value("id").toInt();
                    if(idList.contains(id))
                        addCustomTreeItem<CProcessItem, CProcessItemImpl>(q);
                }
                else
                    addCustomTreeItem<CProcessItem, CProcessItemImpl>(q);
                break;
            }

            default:
                break;
        }
    }
    notifyModelUpdate();
}

void CProcessManager::createCustomTable()
{
    makeCurrentDb();
    createProcessTable(m_db);
    createFolderTable(m_db);
}

void CProcessManager::fillDatabase(std::shared_ptr<ProcessTreeItem> root)
{
    Q_UNUSED(root);
}

void CProcessManager::setCurrentProcessModel(CProcessManager::ID id)
{
    // Set current process
    m_processId = id;
    // Set current model
    setModel(m_processList[m_processId].get());

    notifyTableModelUpdate();
    notifyModelUpdate();
}

void CProcessManager::initModel()
{
    assert(m_pPluginMgr);

    // Create 2 models, one for treeview and one for popup dialog
    for(const auto& id : m_viewIds)
    {
        // Process models
        m_processList.insert(std::make_pair(id, std::make_shared<CProcessModel>()));
        m_processList[id]->insertIcon(static_cast<size_t>(TreeItemType::FOLDER), QIcon(":/Images/folder.png"));
        m_processList[id]->insertIcon(static_cast<size_t>(TreeItemType::PROCESS), QIcon(":/Images/default-process.png"));

        m_proxyList.insert(std::make_pair(id, new CProcessProxyModel(this)));
        m_proxyList[id]->setSourceModel(m_processList[id].get());
    }
}

void CProcessManager::createProcessTable(QSqlDatabase &db)
{
    QSqlQuery q(db);
    if(!q.exec("CREATE TABLE process ("
               "id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, shortDescription TEXTE, description TEXT, keywords TEXT, user TEXT, "
               "authors TEXT, article TEXT, journal TEXT, year INTEGER, docLink TEXT, createdDate TEXT, modifiedDate TEXT, version TEXT, "
               "ikomiaVersion TEXT, language INTEGER, os INTEGER, isInternal INTEGER, iconPath TEXT, certification INTEGER DEFAULT 0, "
               "votes INTEGER DEFAULT 0, license TEXT, repository TEXT, "
               "folderId INTEGER, serverId INTEGER DEFAULT -1, userId INTEGER DEFAULT -1, userReputation INTEGER DEFAULT 0);"))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(!q.exec("CREATE VIRTUAL TABLE processFTS USING fts5(id, name, shortDescription, description, keywords, user, authors, article, journal);"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CProcessManager::createFolderTable(QSqlDatabase &db)
{
    QSqlQuery q(db);

    if(!q.exec("CREATE TABLE processFolder ("
               "id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, processCount INTEGER, iconPath TEXT);"))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CProcessManager::createCustomTreeFolders()
{
    // Core folder
    addCustomPathIcon("Core", ":/Images/folder-core.png");
    addCustomPathIcon("Core/Measurements", ":/Images/folder-measure.png");
    addCustomPathIcon("Core/Utils", ":/Images/folder-utils.png");
    addCustomPathIcon("Core/Plots", ":/Images/folder-plots.png");
    addCustomPathIcon("Core/Segmentation", ":/Images/folder-seg.png");

    // OpenCV folder
    addCustomPathIcon("OpenCV", ":/Images/folder-opencv.png");

    // Gmic folder
    addCustomPathIcon("Gmic", ":/Images/folder-gmic.png");

    // Plugins folder
    addCustomPathIcon("Plugins", ":/Images/folder-plugin.png");
    addCustomPathIcon("Plugins/C++", ":/Images/folder-c++.png");
    addCustomPathIcon("Plugins/Python", ":/Images/folder-python.png");
}

void CProcessManager::createModel()
{
    try
    {
        m_pathMap.clear();
        m_iconMap.clear();
        m_connectionName = Utils::Database::getProcessConnectionName();
        createDb(m_dbType, m_dbName);
        createTree("ProcessLibrary");
        createCustomTable();
        setTreeRoot("All process", m_id++, TreeItemType::FOLDER);

        // Set current model as tree view
        m_processId = ID::PROCESS_TREE;
        setModel(m_processList[m_processId].get());

        // Create custom folder in process tree
        createCustomTreeFolders();

        // Fill database with processes and plugins
        auto factory = m_processRegistrator.getProcessFactory();
        m_processCount = static_cast<int>(factory.getList().size());

        for(auto&& it : factory.getList())
        {
            size_t lastFolderId = buildPath(it->getInfo().getPath());
            addProcess(it, lastFolderId);
        }

        //Fill database with folders information
        addFoldersInfo();

        //Synchronise memory database with file database (main)
        syncProcessInfo();

        // Fill all models from database
        for(const auto& id : m_viewIds)
        {
            // Fill process model
            setModel(m_processList[id].get());
            fillModelFromDatabase();

            // Fill associated table model
            m_processTableList.insert(std::make_pair(id, new CProcessTableModel(0, m_db)));
            m_processTableList[id]->setTable("process");
            m_processTableList[id]->select();
        }

        // Reset current model as tree view
        setModel(m_processList[m_processId].get());
    }
    catch(std::exception& e)
    {
        qCCritical(logProcess).noquote() << QString::fromStdString(e.what());
    }
}

size_t CProcessManager::buildPath(const std::string &path)
{
    std::string pathTmp;
    size_t parentId = 0;
    VectorString folders;

    Utils::String::tokenize(path, folders, "/");
    for(auto&& it : folders)
    {
        pathTmp += it;
        parentId = addFolder(it, pathTmp, parentId);

        if(parentId == 0)
            throw CException(DatabaseExCode::INVALID_QUERY, "Query add process folder failed", __func__, __FILE__, __LINE__);

        pathTmp += "/";
    }
    return parentId;
}

size_t CProcessManager::addFolder(const std::string &folder, const std::string& path, size_t parentId)
{
    auto it = m_pathMap.find(path);
    if(it != m_pathMap.end())
    {
        it->second.second++;
        return it->second.first;
    }
    else
    {
        size_t id = m_id++;
        addItem(QString::fromStdString(folder), TreeItemType::FOLDER, id, parentId);
        m_pathMap.insert(std::make_pair(path, std::make_pair(id, 1)));
        m_iconMap.insert(std::make_pair(path, ""));
        return id;
    }
}

void CProcessManager::addCustomPathIcon(const std::string& path, const std::string& iconPath)
{
    //Maps the given path of the process tree with the given icon
    auto it = m_iconMap.find(path);
    if(it != m_iconMap.end())
        it->second = iconPath;
    else
        m_iconMap.insert(std::make_pair(path, iconPath));
}

size_t CProcessManager::addProcess(const ProcessFactoryPtr &process, size_t folderId)
{
    size_t id = m_id++;
    QString processName = QString::fromStdString(process->getInfo().getName());
    addItem(processName, TreeItemType::PROCESS, id, folderId);
    addProcessInfo(process, id, folderId);
    return id;
}

void CProcessManager::addProcessInfo(const ProcessFactoryPtr& process, size_t id, size_t folderId)
{
    makeCurrentDb();
    QSqlQuery q(m_db);

    //Internal Process or plugin
    bool bInternal = process->getInfo().isInternal();
    //Get process language C++ or Python
    int language = process->getInfo().getLanguage();
    //Get operating system
    int os = process->getInfo().getOS();

    //Icon path
    QString iconPath = QString::fromStdString(process->getInfo().getIconPath());
    if(iconPath.isEmpty() == false && bInternal == false)
    {
        QString pluginDir = QString::fromStdString(Utils::CPluginTools::getDirectory(process->getInfo().getName(), language));
        iconPath = pluginDir + "/" + iconPath;
    }

    auto name = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getName()));
    auto shortDescription = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getShortDescription()));
    auto description = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getDescription()));
    auto keywords = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getKeywords()));
    auto authors = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getAuthors()));
    auto article = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getArticle()));
    auto journal = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getJournal()));
    auto docLink = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getDocumentationLink()));
    auto version = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getVersion()));
    auto ikomiaVersion = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getIkomiaVersion()));
    auto license = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getLicense()));
    auto repo = QString::fromStdString(Utils::String::dbFormat(process->getInfo().getRepository()));

    if (!q.exec(QString("INSERT INTO process"
                        "(id, name, shortDescription, description, keywords, authors, article, journal, year, "
                        "docLink, version, ikomiaVersion, language, os, isInternal, iconPath, folderId, license, repository) "
                        "VALUES (%1, '%2', '%3', '%4', '%5', '%6', '%7', '%8', %9, '%10', '%11', '%12', %13, %14, %15, '%16', %17, '%18', '%19');")
                .arg(id).arg(name).arg(shortDescription).arg(description).arg(keywords).arg(authors).arg(article).arg(journal)
                .arg(process->getInfo().getYear()).arg(docLink).arg(version).arg(ikomiaVersion).arg(language).arg(os).arg(bInternal)
                .arg(iconPath).arg(folderId).arg(license).arg(repo)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if (!q.exec(QString("INSERT INTO processFTS "
                        "(id, name, shortDescription, description, keywords, authors, article, journal) "
                        "VALUES (%1, '%2', '%3', '%4', '%5', '%6', '%7', '%8');")
                .arg(id).arg(name).arg(shortDescription).arg(description).arg(keywords).arg(authors).arg(article).arg(journal)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CProcessManager::addFoldersInfo()
{
    QVariantList ids, names, counts, icons;

    ids << 0;
    names << "All process";
    counts << m_processCount;
    icons << ":/Images/folder-root.png";

    auto it_icon = m_iconMap.begin();
    for(auto it=m_pathMap.begin(); it!=m_pathMap.end(); ++it)
    {
        ids << (int)it->second.first;
        names << QString::fromStdString(it->first);
        counts << (int)it->second.second;
        icons << QString::fromStdString(it_icon->second);
        it_icon++;
    }

    QSqlQuery q(m_db);
    if(!q.prepare(QString("INSERT INTO processFolder (id, name, processCount, iconPath) values (?, ?, ?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    q.addBindValue(ids);
    q.addBindValue(names);
    q.addBindValue(counts);
    q.addBindValue(icons);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CProcessManager::syncProcessInfo()
{
    auto db = QSqlDatabase::database(Utils::Database::getMainConnectionName());
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QStringList tables = db.tables(QSql::Tables);
    if(tables.contains("process") == false)
    {
        createProcessTable(db);
        return;
    }

    //Get all records from main database stored in file
    QSqlQuery q1(db);
    if(!q1.exec("SELECT * FROM process;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList names, shortDescriptions, descriptions, keywords, users, authors, articles, journals, years,
            docLinks, createdDates, modifiedDates, versions, licenses, repositories, serverIds, userIds, os;

    while(q1.next())
    {
        names << q1.value("name");
        shortDescriptions << q1.value("shortDescription");
        descriptions << q1.value("description");
        keywords << q1.value("keywords");
        users << q1.value("user");
        authors << q1.value("authors");
        articles << q1.value("article");
        journals << q1.value("journal");
        years << q1.value("year");
        docLinks << q1.value("docLink");
        createdDates << q1.value("createdDate");
        modifiedDates << q1.value("modifiedDate");
        versions << q1.value("version");
        licenses << q1.value("license");
        repositories << q1.value("repository");
        serverIds << q1.value("serverId");
        userIds << q1.value("userId");
        os << q1.value("os");
    }

    //Update memory databases accordingly
    //Table process
    QSqlQuery q2(m_db);
    q2.prepare(QString("UPDATE process SET "
                       "shortDescription = IFNULL(?, shortDescription), "
                       "description = IFNULL(?, description), "
                       "keywords = IFNULL(?, keywords), "
                       "user = IFNULL(?, user), "
                       "authors = IFNULL(?, authors), "
                       "article = IFNULL(?, article), "
                       "journal = IFNULL(?, journal), "
                       "year = IFNULL(?, year), "
                       "docLink = IFNULL(?, docLink), "
                       "createdDate = IFNULL(?, createdDate), "
                       "modifiedDate = IFNULL(?, modifiedDate), "
                       "version = IFNULL(?, version), "
                       "license = IFNULL(?, license), "
                       "repository = IFNULL(?, repository), "
                       "serverId = IFNULL(?, serverId), "
                       "userId = IFNULL(?, userId), "
                       "os = IFNULL(?, os) "
                       "WHERE name = ?;"));

    q2.addBindValue(shortDescriptions);
    q2.addBindValue(descriptions);
    q2.addBindValue(keywords);
    q2.addBindValue(users);
    q2.addBindValue(authors);
    q2.addBindValue(articles);
    q2.addBindValue(journals);
    q2.addBindValue(years);
    q2.addBindValue(docLinks);
    q2.addBindValue(createdDates);
    q2.addBindValue(modifiedDates);
    q2.addBindValue(versions);
    q2.addBindValue(licenses);
    q2.addBindValue(repositories);
    q2.addBindValue(serverIds);
    q2.addBindValue(userIds);
    q2.addBindValue(os);
    q2.addBindValue(names);

    if(!q2.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Table processFTS
    q2.prepare(QString("UPDATE processFTS SET "
                       "shortDescription = IFNULL(?, shortDescription), "
                       "description = IFNULL(?, description), "
                       "keywords = IFNULL(?, keywords), "
                       "user = IFNULL(?, user), "
                       "authors = IFNULL(?, authors), "
                       "article = IFNULL(?, article), "
                       "journal = IFNULL(?, journal) "
                       "WHERE name = ?;"));

    q2.addBindValue(shortDescriptions);
    q2.addBindValue(descriptions);
    q2.addBindValue(keywords);
    q2.addBindValue(users);
    q2.addBindValue(authors);
    q2.addBindValue(articles);
    q2.addBindValue(journals);
    q2.addBindValue(names);

    if(!q2.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

#include "moc_CProcessManager.cpp"
