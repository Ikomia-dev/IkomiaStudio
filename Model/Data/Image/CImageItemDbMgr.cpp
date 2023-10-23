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

#include "CImageItemDbMgr.h"
#include <QSqlRecord>
#include "UtilsTools.hpp"
#include "CImageItem.hpp"
#include "Model/Project/CProjectModel.h"
#include "Model/Project/CProjectUtils.hpp"

CImageItemDbMgr::CImageItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CImageItemDbMgr::CImageItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CImageItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    loadPaths();
    loadWorkflowImageMap();

    auto id = q.record().value("id").toInt();
    auto itPath = m_mapPaths.find(id);
    auto path = itPath.value();
    auto pItem = static_cast<ProjectTreeItem*>(previousIndex.internalPointer());
    assert(pItem != nullptr);

    //On cherche l'item parent d'une image (DATASET ou DIMENSION)
    while(pItem->getTypeId() != TreeItemType::DATASET &&
          pItem->getTypeId() != TreeItemType::DIMENSION)
    {
        previousIndex = previousIndex.parent();
        pItem = static_cast<ProjectTreeItem*>(previousIndex.internalPointer());
        assert(pItem != nullptr);
    }

    std::shared_ptr<CImageItem> pImage = nullptr;
    if(pItem->getTypeId() == TreeItemType::DATASET)
        pImage = addImagesToDataset(previousIndex, path);
    else if(pItem->getTypeId() == TreeItemType::DIMENSION)
        pImage = addImagesToDimension(previousIndex, path);

    //Add protocol<->image associations
    auto itWorkflow = m_mapWorkflowIds.find(id);
    if(itWorkflow != m_mapWorkflowIds.end())
    {
        for(int i=0; i<itWorkflow.value().size(); ++i)
            pImage->addWorkflowDbId(itWorkflow.value()[i]);
    }
    return pImage;
}

void CImageItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(dbId);

    createTables();

    //Insert current image path into the global map
    auto pItem = itemPtr->getNode<std::shared_ptr<CImageItem>>();
    auto path = QString::fromStdString(pItem->getFullPath());
    auto id = itemPtr->getId();
    m_mapPaths.insert(id, path);

    //Insert workflow ids associated with current image into the global map
    auto ids = pItem->getWorkflowDbIds();
    auto workflowIds = QVector<int>(ids.begin(), ids.end());

    if(workflowIds.size() > 0)
        m_mapWorkflowIds.insert(id, workflowIds);
}

void CImageItemDbMgr::batchSave()
{
    auto db = Utils::Database::connect(m_dbPath, m_connection);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Batch save image paths
    QSqlQuery q1(db);
    if(!q1.prepare(QString("INSERT INTO image (itemId, path) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList ids, values;
    ids = Utils::toVariantList(m_mapPaths.keys());
    values = Utils::toVariantList(m_mapPaths.values());
    q1.addBindValue(ids);
    q1.addBindValue(values);

    if(!q1.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q1.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    //Batch save protocol-image associations
    QSqlQuery q2(db);
    if(!q2.prepare(QString("INSERT INTO protocolImageMap (imageId, protocolId) values (?, ?)")))
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

void CImageItemDbMgr::createTables()
{
    if(m_bTablesCreated == false)
    {
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        //Table "image" to store path
        if(tables.contains("image"))
        {
            if(!q.exec(QString("DROP TABLE image")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE image (id INTEGER PRIMARY KEY, itemId INTEGER, path TEXT NOT NULL);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        //Table "protocolImageMap" to store associations between protocol and image
        if(tables.contains("protocolImageMap"))
        {
            if(!q.exec(QString("DROP TABLE protocolImageMap")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE protocolImageMap (id INTEGER PRIMARY KEY, imageId INTEGER, protocolId INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        m_bTablesCreated = true;
    }
}

void CImageItemDbMgr::loadPaths()
{
    if(m_bPathLoaded == false)
    {
        //Get all image paths
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        if(!q.exec("SELECT itemId, path FROM image"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
            m_mapPaths.insert(q.value(0).toInt(), q.value(1).toString());

        if(isPathUpdateRequired() == true)
            updatePath(db);

        m_bPathLoaded = true;
    }
}

void CImageItemDbMgr::loadWorkflowImageMap()
{
    if(m_bWorkflowImageMapLoaded == false)
    {
        //Get all image-protocol associations
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        if(tables.contains("protocolImageMap") == false)
        {
            if(!q.exec("CREATE TABLE protocolImageMap (id INTEGER PRIMARY KEY, imageId INTEGER, protocolId INTEGER);"))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("SELECT imageId, protocolId FROM protocolImageMap"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
        {
            auto it = m_mapWorkflowIds.find(q.value(0).toInt());
            if(it == m_mapWorkflowIds.end())
                m_mapWorkflowIds.insert(q.value(0).toInt(), QVector<int>({q.value(1).toInt()}));
            else
                it.value() << q.value(1).toInt();
        }
        m_bWorkflowImageMapLoaded = true;
    }
}

std::shared_ptr<CImageItem> CImageItemDbMgr::addImagesToDataset(QModelIndex &datasetItemIndex, const QString &file)
{
    std::shared_ptr<CImageItem> pImage = nullptr;
    auto pDatasetItem = CProjectUtils::getDatasetItem<CMat>(datasetItemIndex);
    if(pDatasetItem)
    {
        auto pDataset = pDatasetItem->getDataset();
        if(pDataset)
        {
            size_t nbImages = pDataset->getDataInfo().size(DataDimension::IMAGE);
            pDataset->setDimension(DataDimension::IMAGE, nbImages + 1);
            boost::filesystem::path path(file.toStdString());
            pDataset->appendFile(file.toStdString());
            pImage = std::make_shared<CImageItem>(path.stem().string(), path.string());
        }
    }
    return pImage;
}

std::shared_ptr<CImageItem> CImageItemDbMgr::addImagesToDimension(QModelIndex &itemIndex, const QString &file)
{
    auto pSet = CProjectUtils::getDataset<CMat>(itemIndex);
    if(!pSet)
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(itemIndex.internalPointer());

    assert(pItem);

    size_t nbImagesIntoDim = pItem->getChildCount();
    size_t nbImages = pSet->getDataInfo().size(DataDimension::IMAGE);

    if(nbImagesIntoDim + 1 > nbImages)
        pSet->setDimension(DataDimension::IMAGE, nbImagesIntoDim + 1);

    boost::filesystem::path path(file.toStdString());
    pSet->appendFile(file.toStdString());

    return std::make_shared<CImageItem>(path.stem().string(), path.string());
}

void CImageItemDbMgr::updatePath(QSqlDatabase& db)
{
    boost::filesystem::path p(m_projectPath.toStdString());
    auto newFolder = QString::fromStdString(p.parent_path().string());

    for(auto it=m_mapPaths.begin(); it!=m_mapPaths.end(); ++it)
    {
        boost::filesystem::path imgPath(it.value().toStdString());
        auto fileName = QString::fromStdString(imgPath.filename().string());
        QString newImgPath = newFolder + "/Data/" + fileName;
        m_mapPaths[it.key()] = newImgPath;
    }

    //Update in database
    QSqlQuery q(db);
    q.prepare("UPDATE image SET path = ? WHERE itemId = ?;");

    QVariantList ids, pathList;
    for(auto it=m_mapPaths.begin(); it!=m_mapPaths.end(); ++it)
    {
        ids << it.key();
        pathList << it.value();
    }
    q.addBindValue(pathList);
    q.addBindValue(ids);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}
