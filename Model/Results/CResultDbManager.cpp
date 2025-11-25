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

#include "CResultDbManager.h"
#include "CResultItem.hpp"

//----------------------------------//
//----- Class CResultDBManager -----//
//----------------------------------//
CResultDbManager::CResultDbManager() : CProjectItemBaseDbMgr()
{
    m_connection = "ResultMemoryDB";
    initMemoryDB();
}

CResultDbManager::CResultDbManager(const QString &path, const QString& connectionName) : CProjectItemBaseDbMgr(path, connectionName)
{
}

CResultDbManager::~CResultDbManager()
{
    QSqlDatabase::removeDatabase(m_connection);
}

std::shared_ptr<CItem> CResultDbManager::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);

    loadTypes();

    auto name = q.record().value("name").toString().toStdString();
    auto id = q.record().value("id").toInt();
    auto it = m_mapTypes.find(id);
    auto type = static_cast<CResultItem::NodeType>(it.value());
    return std::make_shared<CResultItem>(name, id, type);
}

void CResultDbManager::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    createTables();

    auto pResultItem = itemPtr->getNode<std::shared_ptr<CResultItem>>();
    if(pResultItem)
    {
        auto currentDbId = pResultItem->getDbId();
        if(currentDbId == -1)
        {
            //Result item never saved
           auto measures = pResultItem->getMeasures();
           setMeasures(measures, dbId);
        }
        else if(currentDbId != dbId)
        {
            //Update projectItemId (foreign key) for tables "result" and "results"
            updateProjectId(currentDbId, dbId);
        }
        auto type = static_cast<int>(pResultItem->getNodeType());
        m_mapTypes.insert(itemPtr->getId(), type);
        pResultItem->notifyItemSaved(dbId);
    }
}

void CResultDbManager::batchSave()
{
    auto db = Utils::Database::connect(m_dbPath, m_connection);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.prepare(QString("INSERT INTO result (itemId, resultType) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList ids, values;
    ids = Utils::toVariantList(m_mapTypes.keys());
    values = Utils::toVariantList(m_mapTypes.values());
    q.addBindValue(ids);
    q.addBindValue(values);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CResultDbManager::setMeasures(const ObjectsMeasures &measures, int resultDbId)
{    
    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    QVariantList measureIds, measureNames, values, valuesList, blobIds, labels, projectResIds;
    q.prepare("INSERT INTO measures (measureId, measureName, value, valueList, blobId, label, projectResultId) VALUES (?, ?, ?, ?, ?, ?, ?);");

    for(size_t i=0; i<measures.size(); ++i)
    {
        for(size_t j=0; j<measures[i].size(); ++j)
        {
            measureIds << static_cast<int>(measures[i][j].m_measure.m_id);

            if(measures[i][j].m_measure.m_name.empty() == true)
                measureNames << QString::fromStdString(CMeasure::getName(measures[i][j].m_measure.m_id));
            else
                measureNames << QString::fromStdString(measures[i][j].m_measure.m_name);

            if(measures[i][j].m_values.size() == 1)
            {
                values << measures[i][j].m_values[0];
                valuesList << QVariant(QMetaType(QMetaType::QString));
            }
            else
            {
                values << QVariant(QMetaType(QMetaType::Double));
                valuesList << encodeValues(measures[i][j].m_values);
            }
            blobIds << static_cast<int>(measures[i][j].m_graphicsId);
            labels << QString::fromStdString(measures[i][j].m_label);
            projectResIds << static_cast<int>(resultDbId);
        }
    }

    q.addBindValue(measureIds);
    q.addBindValue(measureNames);
    q.addBindValue(values);
    q.addBindValue(valuesList);
    q.addBindValue(blobIds);
    q.addBindValue(labels);
    q.addBindValue(projectResIds);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

ObjectsMeasures CResultDbManager::getMeasures(int resultId)
{
    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT measureId, measureName, value, valueList, blobId, label FROM measures WHERE projectResultId=%1 ORDER BY blobId;").arg(resultId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    ObjectsMeasures measures;
    if(q.first())
    {
        auto nbMeasures = getMeasureCount(db, resultId);
        auto nbObjects = Utils::Database::getQuerySize(q) / nbMeasures;

        for(int i=0; i<nbObjects; ++i)
        {
            ObjectMeasures objMeasures;
            for(int j=0; j<nbMeasures; ++j)
            {
                CObjectMeasure objMeasure;
                objMeasure.m_measure.m_id = static_cast<CMeasure::Id>(q.value("measureId").toInt());
                objMeasure.m_measure.m_name = q.value("measureName").toString().toStdString();
                objMeasure.m_graphicsId = static_cast<size_t>(q.value("blobId").toInt());
                objMeasure.m_label = q.value("label").toString().toStdString();

                auto singleValue = q.value("value");
                if(singleValue.isNull() == false)
                    objMeasure.m_values.push_back(singleValue.toDouble());
                else
                    objMeasure.m_values = decodeValues(q.value("valueList").toString());

                objMeasures.push_back(objMeasure);
                q.next();
            }
            measures.push_back(objMeasures);
        }
    }
    return measures;
}

CMeasuresTableModel *CResultDbManager::createMeasureModel(int resultId)
{
    auto db = connectDB();
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    auto names = getMeasureNames(db, resultId);
    auto strQuery = buildMeasureQuery(resultId, names);
    auto pModel = new CMeasuresTableModel(nullptr);
    pModel->setQuery(strQuery, db);
    pModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Id"));
    pModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Object"));
    pModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Category"));

    int index = 3;
    for(int i=0; i<names.size(); ++i)
        pModel->setHeaderData(index++, Qt::Horizontal, names[i]);

    return pModel;
}

void CResultDbManager::updateProjectId(int oldId, int newId)
{
    auto db = connectDB();
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("UPDATE result SET itemId=%1 WHERE itemId=%2").arg(newId).arg(oldId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(!q.exec(QString("UPDATE measures SET projectResultId=%1 WHERE projectResultId=%2").arg(newId).arg(oldId)))
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CResultDbManager::remove(std::vector<int> resultIds)
{
    auto db = connectDB();
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    //Delete result items which id is in vector resultIds in table "results"
    QString whereCondition;
    for(size_t i=0; i<resultIds.size(); ++i)
    {
        if(i == resultIds.size() - 1)
            whereCondition += QString("projectResultId=%1;").arg(resultIds[i]);
        else
            whereCondition += QString("projectResultId=%1 OR ").arg(resultIds[i]);
    }

    QSqlQuery q(db);
    if(!q.exec(QString("DELETE FROM measures WHERE ") + whereCondition))
       throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CResultDbManager::initMemoryDB()
{
    //Table creation for temporary results -> use of default constructor
    //If we are in the project DB, the results are loaded on demand and must be kept in database

    auto db = connectDB();
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    QStringList tables = db.tables(QSql::Tables);

    //Table results -> contient toutes les mesures calculées par une tâche d'un protocole
    if(tables.contains("measures") == false)
    {
        if(!q.exec("CREATE TABLE measures (id INTEGER PRIMARY KEY, measureId INTEGER, measureName TEXT, value REAL, valueList TEXT, blobId INTEGER, label TEXT, projectResultId INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
    else
    {
        if(!q.exec("DROP TABLE measures;"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        if(!q.exec("CREATE TABLE measures (id INTEGER PRIMARY KEY, measureId INTEGER, measureName TEXT, value REAL, valueList TEXT, blobId INTEGER, label TEXT, projectResultId INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

QSqlDatabase CResultDbManager::connectDB()
{
    return Utils::Database::connect(m_dbPath, m_connection);
}

void CResultDbManager::createTables()
{
    if(m_bTablesCreated == false)
    {
        auto db = connectDB();
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        if(tables.contains("result"))
        {
           if(!q.exec(QString("DROP TABLE result")))
               throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE result (id INTEGER PRIMARY KEY, itemId INTEGER, resultType INTEGER);"))
           throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        //Table results -> contient toutes les mesures calculées par une tâche d'un protocole
        if(tables.contains("measures") == false)
        {
           if(!q.exec("CREATE TABLE measures (id INTEGER PRIMARY KEY, measureId INTEGER, measureName TEXT, value REAL, valueList TEXT, blobId INTEGER, label TEXT, projectResultId INTEGER);"))
               throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        m_bTablesCreated = true;
    }
}

int CResultDbManager::getMeasureCount(const QSqlDatabase &db, int resultId)
{
    if(!db.isValid())
        return 0;

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT COUNT(measureName) FROM measures WHERE projectResultId=%1;").arg(resultId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(q.first())
        return q.value(0).toInt();
    else
        return 0;
}

QVector<QString> CResultDbManager::getMeasureNames(const QSqlDatabase &db, int resultId)
{
    if(!db.isValid())
        return QVector<QString>();

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT DISTINCT measureName FROM measures WHERE projectResultId=%1;").arg(resultId)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVector<QString> names;
    while(q.next())
        names.push_back(q.value(0).toString());

    return names;
}

QString CResultDbManager::buildMeasureQuery(int resultId, const QVector<QString> &measureNames)
{
    QString strQuery = "SELECT id, blobId, label";

    for(int i=0; i<measureNames.size(); ++i)
    {
        strQuery += ",";
        QString measureNameNoSpace = measureNames[i];
        measureNameNoSpace.remove(' ');
        strQuery += QString("MAX(CASE WHEN (measureName='%1' AND value is not null) THEN value\
                                      WHEN (measureName='%1' AND valueList is not null) THEN valueList\
                                 END) AS %2").arg(measureNames[i]).arg(measureNameNoSpace);
    }
    strQuery += QString(" FROM measures WHERE projectResultId=%1 GROUP BY blobId;").arg(resultId);
    return strQuery;
}

QString CResultDbManager::encodeValues(const std::vector<double> &values)
{
    QString strValues;

    for(size_t i=0; i<values.size(); ++i)
    {
        if(strValues.isEmpty() == false)
            strValues += ";";

        strValues += QString::number(values[i]);
    }
    return strValues;
}

std::vector<double> CResultDbManager::decodeValues(const QString& strValues)
{
    std::vector<double> values;
    std::vector<std::string> tokens;

    Utils::String::tokenize(strValues.toStdString(), tokens, ";");
    for(size_t i=0; i<tokens.size(); ++i)
        values.push_back(std::stod(tokens[i]));

    return values;
}

void CResultDbManager::loadTypes()
{
    if(m_bTypesLoaded == false)
    {
        //Get all result item types
        auto db = connectDB();
        if(!db.isValid())
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        if(!q.exec("SELECT itemId, resultType FROM result"))
           throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
           m_mapTypes.insert(q.value(0).toInt(), q.value(1).toInt());

        m_bTypesLoaded = true;
    }
}


