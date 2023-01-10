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

#include "CGraphicsDbManager.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include "Graphics/CGraphicsLayer.h"
#include "Graphics/CGraphicsJSON.h"
#include "CGraphicsLayerItem.hpp"

CGraphicsDbManager::CGraphicsDbManager() : CProjectItemBaseDbMgr()
{
    m_connection = "ResultDB";
}

CGraphicsDbManager::CGraphicsDbManager(const QString &path, const QString &connectionName) : CProjectItemBaseDbMgr(path, connectionName)
{
}

std::shared_ptr<CItem> CGraphicsDbManager::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);

    auto name = q.record().value("name").toString().toStdString();
    auto id = q.record().value("id").toInt();
    int childCount = getLayerChildsCount(id);
    return std::make_shared<CGraphicsLayerItem>(name, id, childCount);
}

void CGraphicsDbManager::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    createTables();

    auto pLayerItem = itemPtr->getNode<std::shared_ptr<CGraphicsLayerItem>>();
    auto pLayer = pLayerItem->getLayer();

    if(pLayer)
    {
        //Graphics scene has been loaded, we have to replace all graphics items (delete then add)
        replaceLayer(pLayer, pLayerItem->getDbId(), dbId);
    }
    else
    {
        //Graphics scene has not been loaded, we have to update graphics items (layerId)
        updateProjectId(pLayerItem->getDbId(), dbId);
    }
    pLayerItem->notifyProjectSaved(dbId);
}

void CGraphicsDbManager::batchSave()
{
}

int CGraphicsDbManager::getLayerChildsCount(int layerId)
{
    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Get number of child items
    QSqlQuery query(db);
    if(!query.exec(QString("SELECT COUNT(*) FROM graphics where layerId=%1").arg(layerId)))
        throw CException(DatabaseExCode::INVALID_QUERY, query.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!query.next())
       throw CException(DatabaseExCode::INVALID_QUERY, query.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    return query.value(0).toInt();
}

void CGraphicsDbManager::replaceLayer(CGraphicsLayer *pLayer, int oldLayerId, int newLayerId)
{
    assert(pLayer);

    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("DELETE FROM graphics WHERE layerId=%1;").arg(oldLayerId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    CGraphicsJSON jsonMgr;
    QVariantList types, dataList, layerIds;
    q.prepare("INSERT INTO graphics (type, data, layerId) VALUES (?, ?, ?);");

    auto childs = pLayer->childItems();
    for(auto it : childs)
    {
        auto pItem = dynamic_cast<CGraphicsItem*>(it);
        if(pItem)
        {
            types << it->type();
            dataList << pItem->getJsonData(jsonMgr);
            layerIds << newLayerId;
        }
    }

    if(types.size() > 0)
    {
        q.addBindValue(types);
        q.addBindValue(dataList);
        q.addBindValue(layerIds);

        if(!q.execBatch())
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CGraphicsDbManager::updateProjectId(int oldId, int newId)
{
    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("UPDATE graphics SET layerId=%1 WHERE layerId=%2").arg(newId).arg(oldId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

GraphicsDbInfo CGraphicsDbManager::loadGraphics(size_t typeId, const QString& path)
{
    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Get project row for image from its path
    QString type;
    switch(typeId)
    {
        case TreeItemType::IMAGE:
            type = "image";
            break;
        case TreeItemType::VIDEO:
            type = "video";
            break;
        case TreeItemType::LIVE_STREAM:
            type = "Stream";
            break;
    }
    QSqlQuery q(db);
    if(!q.exec(QString("SELECT p.left, p.right FROM project p INNER JOIN %1 i ON p.id = i.itemId WHERE i.path = '%2';").arg(type).arg(path)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!q.next())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    int left = q.value(0).toInt();
    int right = q.value(1).toInt();

    //Get all graphics items for this image
    if(!q.exec(QString("SELECT g.type, g.data, p.id FROM graphics g INNER JOIN project p ON g.layerId = p.id\
                        WHERE (SELECT p.id FROM project p WHERE p.left > %1 AND p.right < %2);").arg(left).arg(right)))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    GraphicsDbInfo info;
    while(q.next())
    {
        int itemType = q.value(0).toInt();
        QByteArray itemData = q.value(1).toByteArray();
        int layerId = q.value(2).toInt();
        info.insert(std::make_pair(layerId, std::make_pair(itemType, itemData)));
    }
    return info;
}

void CGraphicsDbManager::remove(std::vector<int> layerIds)
{
    auto db = connectDB();
    if(db.isValid() == false)
       throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Delete graphics items of layers which id is in vector layerIds
    QString whereCondition;
    for(size_t i=0; i<layerIds.size(); ++i)
    {
        if(i == layerIds.size() - 1)
            whereCondition += QString("layerId=%1;").arg(layerIds[i]);
        else
            whereCondition += QString("layerId=%1 OR ").arg(layerIds[i]);
    }

    QSqlQuery q(db);
    if(!q.exec(QString("DELETE FROM graphics WHERE ") + whereCondition))
      throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

QSqlDatabase CGraphicsDbManager::connectDB()
{
    return Utils::Database::connect(m_dbPath, m_connection);
}

void CGraphicsDbManager::createTables()
{
    auto db = connectDB();
    if(db.isValid() == false)
       throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    QStringList tables = db.tables(QSql::Tables);

    //Table "graphics" to store all graphics objects
    if(tables.contains("graphics") == false)
    {
       if(!q.exec("CREATE TABLE graphics (id INTEGER PRIMARY KEY, type INTEGER, data BLOB, layerId INTEGER);"))
           throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}
