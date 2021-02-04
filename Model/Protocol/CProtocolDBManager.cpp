#include "CProtocolDBManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include "Main/AppTools.hpp"
#include "Model/Process/CProcessManager.h"

CProtocolDBManager::CProtocolDBManager()
{
    m_path = Utils::Database::getMainPath();
}

CProtocolDBManager::~CProtocolDBManager()
{
    QSqlDatabase::removeDatabase(Utils::Database::getMainConnectionName());
}

void CProtocolDBManager::setManagers(CSettingsManager *pSettingsMgr)
{
    m_pSettingsMgr = pSettingsMgr;
}

std::map<QString, int> CProtocolDBManager::getProtocols()
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

QString CProtocolDBManager::getProtocolDescription(const int protocolId)
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

QStringList CProtocolDBManager::searchProtocols(const QString &text)
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

int CProtocolDBManager::save(const ProtocolPtr &pProtocol)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    return save(db, pProtocol);
}

int CProtocolDBManager::save(const ProtocolPtr &pProtocol, QString path)
{
    auto db = initDB(path, "ProtocolConnectionTmp");
    int protocolId = save(db, pProtocol);
    db.close();
    return protocolId;
}

int CProtocolDBManager::save(QSqlDatabase& db, const ProtocolPtr &pProtocol)
{
    assert(pProtocol);

    if(!db.isValid())
       throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    std::unordered_map<ProtocolVertex, int> mapVertexToDbId;

    bool bTransaction = db.driver()->hasFeature(QSqlDriver::Transactions);
    if(bTransaction)
       db.transaction();

    //Ajout du protocol
    QString name = QString::fromStdString(Utils::String::dbFormat(pProtocol->getName()));
    QString keywords = QString::fromStdString(Utils::String::dbFormat(pProtocol->getKeywords()));
    QString description = QString::fromStdString(Utils::String::dbFormat(pProtocol->getDescription()));

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
    auto vertexIt = pProtocol->getVertices();
    if(vertexIt.first != vertexIt.second)
    {
       QSqlQuery q2(db);
       q.prepare("INSERT INTO protocolTask (name, protocolId) VALUES (?, ?);");
       q2.prepare("INSERT INTO protocolParameter (name, value, taskId) VALUES (? , ?, ?);");

       for(auto it=vertexIt.first; it!=vertexIt.second; ++it)
       {
           if(pProtocol->isRoot(*it) == false)
           {
               auto pTask = pProtocol->getTask(*it);
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
    auto edgeIt = pProtocol->getEdges();
    if(edgeIt.first != edgeIt.second)
    {
       q.prepare("INSERT INTO protocolEdge (srcIndex, targetIndex, protocolId, srcTaskId, targetTaskId) VALUES (?, ?, ?, ?, ?);");
       for(auto it=edgeIt.first; it!=edgeIt.second; ++it)
       {
           auto pEdge = pProtocol->getEdge(*it);
           //Indices des ports I/O
           q.addBindValue((int)pEdge->getSourceIndex());
           q.addBindValue((int)pEdge->getTargetIndex());
           //Foreign key : protocole
           q.addBindValue(protocolId);

           //Foreign key : tâche source
           auto srcId = pProtocol->getEdgeSource(*it);
           auto srcFk = mapVertexToDbId.find(srcId);

           if(srcFk != mapVertexToDbId.end())
               q.addBindValue(srcFk->second);
           else
               q.addBindValue(-1);

           //Foreign key : tâche cible
           auto targetId = pProtocol->getEdgeTarget(*it);
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

std::unique_ptr<CProtocol> CProtocolDBManager::load(int protocolId, CProcessManager *pProcessMgr, const GraphicsContextPtr &graphicsContextPtr)
{
    auto db = initDB(m_path, Utils::Database::getMainConnectionName());
    return load(db, protocolId, pProcessMgr, graphicsContextPtr);
}

std::unique_ptr<CProtocol> CProtocolDBManager::load(QString path, CProcessManager *pProcessMgr, const GraphicsContextPtr &graphicsContextPtr)
{
    std::unique_ptr<CProtocol> pProtocol = nullptr;
    auto db = initDB(path, "ProtocolConnectionTmp");

    QSqlQuery q(db);
    if(!q.exec("SELECT id FROM protocol;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(q.first())
        pProtocol = load(db, q.value(0).toInt(), pProcessMgr, graphicsContextPtr);

    db.close();
    return pProtocol;
}

std::unique_ptr<CProtocol> CProtocolDBManager::load(QSqlDatabase& db, int protocolId, CProcessManager *pProcessMgr, const GraphicsContextPtr& graphicsContextPtr)
{
    assert(pProcessMgr);
    std::unordered_map<int,ProtocolVertex>  mapDbIdToVertexId;
    std::unique_ptr<CProtocol> pProtocol = nullptr;

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
    pProtocol = std::make_unique<CProtocol>(name);

    if(m_pSettingsMgr)
        pProtocol->setOutputFolder(m_pSettingsMgr->getProtocolSaveFolder() + name + "/");

    if(!q.isNull("keywords"))
        pProtocol->setKeywords(q.value("keywords").toString().toStdString());

    if(!q.isNull("description"))
        pProtocol->setKeywords(q.value("description").toString().toStdString());

    do
    {
        int taskId = q.value("taskId").toInt();
        std::string taskName = q.value("taskName").toString().toStdString();
        UMapString paramMap = getTaskParameters(db, taskId);
        auto pTask = pProcessMgr->createObject(taskName, nullptr);

        if(pTask)
        {
            pTask->setParamMap(paramMap);
            pTask->parametersModified();
            pTask->setGraphicsContext(graphicsContextPtr);
            auto vertexId = pProtocol->addTask(pTask);
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
        ProtocolVertex srcTaskId = boost::graph_traits<ProtocolGraph>::null_vertex();

        auto itSrc = mapDbIdToVertexId.find(dbSrcId);
        if(itSrc != mapDbIdToVertexId.end())
            srcTaskId = itSrc->second;

        int dbTargetId = q.value("targetTaskId").toInt();
        ProtocolVertex targetTaskId = boost::graph_traits<ProtocolGraph>::null_vertex();

        auto itTarget = mapDbIdToVertexId.find(dbTargetId);
        if(itTarget != mapDbIdToVertexId.end())
            targetTaskId = itTarget->second;

        try
        {
            pProtocol->connect(srcTaskId, q.value("srcIndex").toInt(), targetTaskId, q.value("targetIndex").toInt());
        }
        catch(std::exception& e)
        {
            qWarning().noquote() << QString::fromStdString(e.what());
        }
    }
    return pProtocol;
}

void CProtocolDBManager::remove(int protocolId)
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

QSqlDatabase CProtocolDBManager::initDB(const QString& path, const QString& connectionName)
{
    auto db = Utils::Database::connect(path, connectionName);
    createTables(db);
    return db;
}

void CProtocolDBManager::createTables(QSqlDatabase& db)
{
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    QStringList tables = db.tables(QSql::Tables);

    //Table Protocol
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

UMapString CProtocolDBManager::getTaskParameters(QSqlDatabase& db, int taskId)
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
