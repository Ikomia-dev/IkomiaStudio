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

#include "CWorkflowDBManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include "Main/AppTools.hpp"
#include "Model/Process/CProcessManager.h"

CWorkflowDBManager::CWorkflowDBManager()
{
    m_path = Utils::Database::getMainPath();
}

CWorkflowDBManager::~CWorkflowDBManager()
{
    QSqlDatabase::removeDatabase(Utils::Database::getMainConnectionName());
}

void CWorkflowDBManager::setManagers(CSettingsManager *pSettingsMgr)
{
    m_pSettingsMgr = pSettingsMgr;
}

std::map<QString, int> CWorkflowDBManager::getWorkflows()
{
    std::map<QString, int> protocols;

    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec("SELECT id, name FROM protocol;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    while(q.next())
        protocols.insert(std::make_pair(q.value(1).toString(), q.value(0).toInt()));

    return protocols;
}

QString CWorkflowDBManager::getWorkflowDescription(const int protocolId)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    if(!db.isValid())
       throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT description FROM protocol WHERE id=%1").arg(protocolId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QString description;
    if(q.next())
        description = q.value("description").toString();

    return description;
}

QStringList CWorkflowDBManager::searchWorkflows(const QString &text)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);

    if(text.isEmpty())
    {
        if (!q.exec(QString("SELECT name FROM protocolFTS;")))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
    else
    {
        QString searchQuery = Utils::Database::getFTSKeywords(text);
        if(!q.exec(QString("SELECT name FROM protocolFTS WHERE protocolFTS MATCH '%1';").arg(searchQuery)))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    QStringList protocolNames;
    while(q.next())
        protocolNames.push_back(q.value(0).toString());

    return protocolNames;
}

int CWorkflowDBManager::save(const WorkflowPtr &pWorkflow)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    return save(db, pWorkflow);
}

int CWorkflowDBManager::save(const WorkflowPtr &pWorkflow, QString path)
{
    auto db = initDB(path, "WorkflowConnectionTmp");
    int protocolId = save(db, pWorkflow);
    db.close();
    return protocolId;
}

int CWorkflowDBManager::save(QSqlDatabase& db, const WorkflowPtr &pWorkflow)
{
    assert(pWorkflow);

    if(!db.isValid())
       throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    std::unordered_map<WorkflowVertex, int> mapVertexToDbId;

    bool bTransaction = db.driver()->hasFeature(QSqlDriver::Transactions);
    if(bTransaction)
       db.transaction();

    //Ajout du protocol
    QString name = QString::fromStdString(Utils::String::dbFormat(pWorkflow->getName()));
    QString keywords = QString::fromStdString(Utils::String::dbFormat(pWorkflow->getKeywords()));
    QString description = QString::fromStdString(Utils::String::dbFormat(pWorkflow->getDescription()));

    if(!q.exec(QString("INSERT INTO protocol (name, keywords, description) VALUES ('%1', '%2', '%3');").arg(name, keywords, description)))
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    int protocolId = q.lastInsertId().toInt();
    if(!q.exec(QString("INSERT INTO protocolFTS (id, name, keywords, description) VALUES (%1, '%2', '%3', '%4');")
               .arg(protocolId)
               .arg(name, keywords, description)))
    {
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    //Ajout des taches
    auto vertexIt = pWorkflow->getVertices();
    if(vertexIt.first != vertexIt.second)
    {
       QSqlQuery q2(db);
       q.prepare("INSERT INTO protocolTask (name, protocolId) VALUES (?, ?);");
       q2.prepare("INSERT INTO protocolParameter (name, value, taskId) VALUES (? , ?, ?);");

       for(auto it=vertexIt.first; it!=vertexIt.second; ++it)
       {
           if(pWorkflow->isRoot(*it) == false)
           {
               auto pTask = pWorkflow->getTask(*it);
               //Nom de la tache
               q.addBindValue(QString::fromStdString(pTask->getName()));
               //Foreign key : protocole
               q.addBindValue(protocolId);

               if(!q.exec())
                   throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

               int taskId = q.lastInsertId().toInt();
               mapVertexToDbId.insert(std::make_pair(*it, taskId));

               //Sauvegarde des paramètres associés
               auto paramMap = pTask->getParam()->getParamMap();
               for(auto it=paramMap.begin(); it!=paramMap.end(); ++it)
               {
                   q2.addBindValue(QString::fromStdString(it->first));
                   q2.addBindValue(QString::fromStdString(it->second));
                   q2.addBindValue(taskId);

                   if(!q2.exec())
                       throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
               }
           }
       }
    }

    //Ajout des arcs
    auto edgeIt = pWorkflow->getEdges();
    if(edgeIt.first != edgeIt.second)
    {
       q.prepare("INSERT INTO protocolEdge (srcIndex, targetIndex, protocolId, srcTaskId, targetTaskId) VALUES (?, ?, ?, ?, ?);");
       for(auto it=edgeIt.first; it!=edgeIt.second; ++it)
       {
           auto pEdge = pWorkflow->getEdge(*it);
           //Indices des ports I/O
           q.addBindValue((int)pEdge->getSourceIndex());
           q.addBindValue((int)pEdge->getTargetIndex());
           //Foreign key : protocole
           q.addBindValue(protocolId);

           //Foreign key : tâche source
           auto srcId = pWorkflow->getEdgeSource(*it);
           auto srcFk = mapVertexToDbId.find(srcId);

           if(srcFk != mapVertexToDbId.end())
               q.addBindValue(srcFk->second);
           else
               q.addBindValue(-1);

           //Foreign key : tâche cible
           auto targetId = pWorkflow->getEdgeTarget(*it);
           auto targetFk = mapVertexToDbId.find(targetId);

           if(targetFk != mapVertexToDbId.end())
               q.addBindValue(targetFk->second);
           else
               q.addBindValue(-1);

           if(!q.exec())
               throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
       }
    }

    if(bTransaction)
        db.commit();

    return protocolId;
}

std::unique_ptr<CWorkflow> CWorkflowDBManager::load(int protocolId, CProcessManager *pProcessMgr, const GraphicsContextPtr &graphicsContextPtr)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    return load(db, protocolId, pProcessMgr, graphicsContextPtr);
}

std::unique_ptr<CWorkflow> CWorkflowDBManager::load(QString path, CProcessManager *pProcessMgr, const GraphicsContextPtr &graphicsContextPtr)
{
    std::unique_ptr<CWorkflow> pWorkflow = nullptr;
    auto db = initDB(path, "WorkflowConnectionTmp");

    QSqlQuery q(db);
    if(!q.exec("SELECT id FROM protocol;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(q.first())
        pWorkflow = load(db, q.value(0).toInt(), pProcessMgr, graphicsContextPtr);

    db.close();
    return pWorkflow;
}

std::unique_ptr<CWorkflow> CWorkflowDBManager::load(QSqlDatabase& db, int protocolId, CProcessManager *pProcessMgr, const GraphicsContextPtr& graphicsContextPtr)
{
    assert(pProcessMgr);
    std::unordered_map<int,WorkflowVertex>  mapDbIdToVertexId;
    std::unique_ptr<CWorkflow> pWorkflow = nullptr;

    if(!db.isValid())
       throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);

    //Chargement des tâches
    if(!q.exec(QString("SELECT p.name as protocolName, p.keywords, p.description, t.id as taskId, t.name as taskName\
                        FROM protocol p INNER JOIN protocolTask t ON p.id=t.protocolId WHERE p.id=%1;").arg(protocolId)))
    {
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(!q.first())
       return nullptr;

    auto name = q.value("protocolName").toString().toStdString();
    pWorkflow = std::make_unique<CWorkflow>(name, &pProcessMgr->m_processRegistrator, &pProcessMgr->m_ioRegistrator, graphicsContextPtr);

    if(m_pSettingsMgr)
        pWorkflow->setOutputFolder(m_pSettingsMgr->getWorkflowSaveFolder() + name + "/");

    if(!q.isNull("keywords"))
        pWorkflow->setKeywords(q.value("keywords").toString().toStdString());

    if(!q.isNull("description"))
        pWorkflow->setKeywords(q.value("description").toString().toStdString());

    CPyEnsureGIL gil;
    do
    {
        int taskId = q.value("taskId").toInt();
        std::string taskName = q.value("taskName").toString().toStdString();
        UMapString paramMap = getTaskParameters(db, taskId);
        auto pTask = pProcessMgr->createObject(taskName, nullptr);

        if(pTask)
        {
            pTask->setParamValues(paramMap);
            pTask->parametersModified();
            auto vertexId = pWorkflow->addTask(pTask);
            mapDbIdToVertexId.insert(std::make_pair(taskId, vertexId));
        }
        else
            throw CException(CoreExCode::NULL_POINTER, "The task of type: " + taskName + " can't be created", __func__, __FILE__, __LINE__);
    }
    while(q.next());

    //Chargement des arcs
    if(!q.exec(QString("SELECT e.srcIndex, e.targetIndex, e.srcTaskId, e.targetTaskId\
                        FROM protocol p INNER JOIN protocolEdge e ON p.id=e.protocolId WHERE p.id=%1;").arg(protocolId)))
    {
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    while(q.next())
    {
        int dbSrcId = q.value("srcTaskId").toInt();
        WorkflowVertex srcTaskId = boost::graph_traits<WorkflowGraph>::null_vertex();

        auto itSrc = mapDbIdToVertexId.find(dbSrcId);
        if(itSrc != mapDbIdToVertexId.end())
            srcTaskId = itSrc->second;

        int dbTargetId = q.value("targetTaskId").toInt();
        WorkflowVertex targetTaskId = boost::graph_traits<WorkflowGraph>::null_vertex();

        auto itTarget = mapDbIdToVertexId.find(dbTargetId);
        if(itTarget != mapDbIdToVertexId.end())
            targetTaskId = itTarget->second;

        try
        {
            pWorkflow->connect(srcTaskId, q.value("srcIndex").toInt(), targetTaskId, q.value("targetIndex").toInt());
        }
        catch(std::exception& e)
        {
            qWarning().noquote() << QString::fromStdString(e.what());
        }
    }
    return pWorkflow;
}

void CWorkflowDBManager::remove(int protocolId)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("DELETE FROM protocol WHERE id=%1").arg(protocolId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!q.exec(QString("DELETE FROM protocolFTS WHERE id=%1").arg(protocolId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

QSqlDatabase CWorkflowDBManager::initDB(const QString& path, const QString& connectionName)
{
    auto db = Utils::Database::connect(path, connectionName);
    createTables(db);
    return db;
}

void CWorkflowDBManager::createTables(QSqlDatabase& db)
{
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    QStringList tables = db.tables(QSql::Tables);

    //Table Workflow
    if(tables.contains("protocol") == false)
    {
        if(!q.exec("CREATE TABLE protocol (id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, keywords TEXT, description TEXT);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(tables.contains("protocolFTS") == false)
    {
        if(!q.exec("CREATE VIRTUAL TABLE protocolFTS USING FTS5(id, name, keywords, description);"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    //Table Task
    if(tables.contains("protocolTask") == false)
    {
        if(!q.exec("CREATE TABLE protocolTask (id INTEGER PRIMARY KEY, name TEXT NOT NULL, protocolId INTEGER,\
                    FOREIGN KEY(protocolId) REFERENCES protocol(id) ON DELETE CASCADE);"))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }

    //Table Edge
    if(tables.contains("protocolEdge") == false)
    {
        if(!q.exec("CREATE TABLE protocolEdge (id INTEGER PRIMARY KEY, srcIndex INTEGER, targetIndex INTEGER,\
                    protocolId INTEGER, srcTaskId INTEGER, targetTaskId INTEGER,\
                    FOREIGN KEY(protocolId) REFERENCES protocol(id) ON DELETE CASCADE);"))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }

    //Table Parameter
    if(tables.contains("protocolParameter") == false)
    {
        if(!q.exec("CREATE TABLE protocolParameter (id INTEGER PRIMARY KEY, name TEXT NOT NULL, value TEXT, taskId INTEGER,\
                    FOREIGN KEY(taskId) REFERENCES protocolTask(id) ON DELETE CASCADE);"))
        {
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }
}

UMapString CWorkflowDBManager::getTaskParameters(QSqlDatabase& db, int taskId)
{
    QSqlQuery q(db);

    if(!q.exec(QString("SELECT * FROM protocolParameter WHERE taskId=%1").arg(taskId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    UMapString paramMap;
    while(q.next())
    {
        paramMap.insert(std::make_pair(q.value("name").toString().toStdString(),
                                       q.value("value").toString().toStdString()));
    }
    return paramMap;
}
