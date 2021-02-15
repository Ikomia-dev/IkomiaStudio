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

#include "CDatasetItemDbMgr.h"
#include "Data/CMat.hpp"
#include "CDatasetItem.hpp"

CDatasetItemDbMgr::CDatasetItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CDatasetItemDbMgr::CDatasetItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CDatasetItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);

    loadDataTypes();

    auto name = q.record().value("name").toString().toStdString();
    auto id = q.record().value("id").toInt();
    auto it = m_mapTypes.find(id);

    IODataType type = static_cast<IODataType>(it.value());
    switch(type)
    {
        case IODataType::IMAGE:
            return std::make_shared<CDatasetItem<CMat>>(name, type);
        case IODataType::VIDEO:
            return std::make_shared<CDatasetItem<CMat>>(name, type);
        default:
            return nullptr;
    }
    return nullptr;
}

void CDatasetItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(dbId);

    createTables();
    auto pItem = itemPtr->getNode<std::shared_ptr<CDatasetItem<CMat>>>();
    auto dataset = pItem->getDataset();
    auto id = itemPtr->getId();
    auto type = static_cast<int>(dataset->getType());
    m_mapTypes.insert(id, type);
}

void CDatasetItemDbMgr::batchSave()
{
    auto db = Utils::Database::connect(m_dbPath, m_connection);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.prepare(QString("INSERT INTO dataset (itemId, datatype) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList ids, values;
    ids = Utils::toVariantList(m_mapTypes.keys());
    values = Utils::toVariantList(m_mapTypes.values());
    q.addBindValue(ids);
    q.addBindValue(values);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CDatasetItemDbMgr::createTables()
{
    if(m_bTablesCreated == false)
    {
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        //Table "dataset" to store data type
        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        if(tables.contains("dataset"))
        {
            if(!q.exec(QString("DROP TABLE dataset")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE dataset (id INTEGER PRIMARY KEY, itemId INTEGER, datatype INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        m_bTablesCreated = true;
    }
}

void CDatasetItemDbMgr::loadDataTypes()
{
    if(m_bDataTypeLoaded == false)
    {
        //Get all dataset data types
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        if(!q.exec("SELECT itemId, datatype FROM dataset"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
            m_mapTypes.insert(q.value(0).toInt(), q.value(1).toInt());

        m_bDataTypeLoaded = true;
    }
}
