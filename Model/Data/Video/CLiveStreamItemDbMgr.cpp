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

#include "CLiveStreamItemDbMgr.h"
#include <QSqlRecord>
#include "UtilsTools.hpp"
#include "CLiveStreamItem.hpp"
#include "Model/Project/CProjectModel.h"
#include "Model/Project/CProjectUtils.hpp"

CLiveStreamItemDbMgr::CLiveStreamItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CLiveStreamItemDbMgr::CLiveStreamItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CLiveStreamItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    loadPaths();
    loadWorkflowVideoMap();

    auto id = q.record().value("id").toInt();
    auto name = q.record().value("name").toString().toStdString();
    auto itPath = m_mapPaths.find(id);
    auto filepath = itPath.value();
    auto pItem = static_cast<ProjectTreeItem*>(previousIndex.internalPointer());
    assert(pItem != nullptr);

    //On cherche l'item parent d'une Video (DIMENSION TIME)
    while(pItem->getTypeId() != TreeItemType::DIMENSION)
    {
        previousIndex = previousIndex.parent();
        pItem = static_cast<ProjectTreeItem*>(previousIndex.internalPointer());
        assert(pItem != nullptr);
    }

    boost::filesystem::path path(filepath.toStdString());

    auto pSet = CProjectUtils::getDataset<CMat>(previousIndex);
    if(!pSet)
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

    pSet->appendFile(filepath.toStdString());
    auto pLiveStream = std::make_shared<CLiveStreamItem>(name, path.string());

    //Add protocol<->Video associations
    auto itWorkflow = m_mapWorkflowIds.find(id);
    if(itWorkflow != m_mapWorkflowIds.end())
    {
        for(int i=0; i<itWorkflow.value().size(); ++i)
            pLiveStream->addWorkflowDbId(itWorkflow.value()[i]);
    }
    return pLiveStream;
}

void CLiveStreamItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(dbId);

    createTables();

    //Insert current Video path into the global map
    auto pItem = itemPtr->getNode<std::shared_ptr<CLiveStreamItem>>();
    auto path = QString::fromStdString(pItem->getFullPath());
    auto id = itemPtr->getId();
    m_mapPaths.insert(id, path);

    //Insert protocol ids associated with current Video into the global map
    auto protocolIds = QVector<int>::fromStdVector(pItem->getWorkflowDbIds());
    if(protocolIds.size() > 0)
        m_mapWorkflowIds.insert(id, protocolIds);
}

void CLiveStreamItemDbMgr::batchSave()
{
    auto db = Utils::Database::connect(m_dbPath, m_connection);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Batch save Video paths
    QSqlQuery q1(db);
    if(!q1.prepare(QString("INSERT INTO Stream (itemId, path) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList ids, values;
    ids = Utils::toVariantList(m_mapPaths.keys());
    values = Utils::toVariantList(m_mapPaths.values());
    q1.addBindValue(ids);
    q1.addBindValue(values);

    if(!q1.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Batch save protocol-Video associations
    QSqlQuery q2(db);
    if(!q2.prepare(QString("INSERT INTO protocolVideoMap (VideoId, protocolId) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    ids.clear();
    values.clear();

    for(auto it=m_mapWorkflowIds.begin(); it!=m_mapWorkflowIds.end(); ++it)
    {
        for(int i=0; i<it.value().size(); ++i)
        {
            ids << it.key();
            values << it.value()[i];
        }
    }
    q2.addBindValue(ids);
    q2.addBindValue(values);

    if(!q2.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q2.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CLiveStreamItemDbMgr::createTables()
{
    if(m_bTablesCreated == false)
    {
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        //Table "Video" to store path
        if(tables.contains("Stream"))
        {
            if(!q.exec(QString("DROP TABLE Stream")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE Stream (id INTEGER PRIMARY KEY, itemId INTEGER, path TEXT NOT NULL);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        //Table "protocolVideoMap" to store associations between protocol and Video
        if(tables.contains("protocolVideoMap"))
        {
            if(!q.exec(QString("DROP TABLE protocolVideoMap")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE protocolVideoMap (id INTEGER PRIMARY KEY, VideoId INTEGER, protocolId INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        m_bTablesCreated = true;
    }
}

void CLiveStreamItemDbMgr::loadPaths()
{
    if(m_bPathLoaded == false)
    {
        //Get all Video paths
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        if(!q.exec("SELECT itemId, path FROM Stream"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
            m_mapPaths.insert(q.value(0).toInt(), q.value(1).toString());

        m_bPathLoaded = true;
    }
}

void CLiveStreamItemDbMgr::loadWorkflowVideoMap()
{
    if(m_bWorkflowVideoMapLoaded == false)
    {
        //Get all Video-protocol associations
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        if(tables.contains("protocolVideoMap") == false)
        {
            if(!q.exec("CREATE TABLE protocolVideoMap (id INTEGER PRIMARY KEY, VideoId INTEGER, protocolId INTEGER);"))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("SELECT VideoId, protocolId FROM protocolVideoMap"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
        {
            auto it = m_mapWorkflowIds.find(q.value(0).toInt());
            if(it == m_mapWorkflowIds.end())
                m_mapWorkflowIds.insert(q.value(0).toInt(), QVector<int>({q.value(1).toInt()}));
            else
                it.value() << q.value(1).toInt();
        }
        m_bWorkflowVideoMapLoaded = true;
    }
}
