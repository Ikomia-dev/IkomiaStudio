#include "CDimensionItemDbMgr.h"
#include "UtilsTools.hpp"
#include "Model/Project/CProjectUtils.hpp"
#include "CDimensionItem.hpp"

CDimensionItemDbMgr::CDimensionItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CDimensionItemDbMgr::CDimensionItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CDimensionItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    loadTypes();

    auto name = q.record().value("name").toString().toStdString();
    auto id = q.record().value("id").toInt();
    auto it = m_mapTypes.find(id);

    auto pSet = CProjectUtils::getDataset<CMat>(previousIndex);
    if(!pSet)
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "No matching dataset from model index", __func__, __FILE__, __LINE__);

    DataDimension dim = static_cast<DataDimension>(it.value());
    if(!pSet->hasDimension(dim))
    {
        //Find the dimension level
        size_t level = 0;
        QModelIndex tmpItemIndex = previousIndex;
        auto pItem = static_cast<ProjectTreeItem*>(tmpItemIndex.internalPointer());

        while(pItem != nullptr && pItem->getTypeId() != TreeItemType::DATASET)
        {
            tmpItemIndex = tmpItemIndex.parent();
            pItem = static_cast<ProjectTreeItem*>(tmpItemIndex.internalPointer());
            level++;
        }
        pSet->insertDimension(dim, level);
    }
    else
        pSet->upDimension(dim, 1);

    return std::make_shared<CDimensionItem>(dim, name);
}

void CDimensionItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(dbId);

    createTables();
    auto pItem = itemPtr->getNode<std::shared_ptr<CDimensionItem>>();
    auto dim = static_cast<int>(pItem->getDimension());
    auto id = static_cast<int>(itemPtr->getId());
    m_mapTypes.insert(id, dim);
}

void CDimensionItemDbMgr::batchSave()
{
    auto db = Utils::Database::connect(m_dbPath, m_connection);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.prepare(QString("INSERT INTO dimension (itemId, datadimension) values (?, ?)")))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QVariantList ids, values;
    ids = Utils::toVariantList(m_mapTypes.keys());
    values = Utils::toVariantList(m_mapTypes.values());
    q.addBindValue(ids);
    q.addBindValue(values);

    if(!q.execBatch())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CDimensionItemDbMgr::createTables()
{
    if(m_bTablesCreated == false)
    {
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        QStringList tables = db.tables(QSql::Tables);

        //Table "dimension" to store dimension type
        if(tables.contains("dimension"))
        {
            if(!q.exec(QString("DROP TABLE dimension")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        if(!q.exec("CREATE TABLE dimension (id INTEGER PRIMARY KEY, itemId INTEGER, datadimension INTEGER);"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        m_bTablesCreated = true;
    }
}

void CDimensionItemDbMgr::loadTypes()
{
    if(m_bTypesLoaded == false)
    {
        //Get all dimension types
        auto db = Utils::Database::connect(m_dbPath, m_connection);
        if(db.isValid() == false)
            throw CException(DatabaseExCode::INVALID_QUERY, "Invalid database connection", __func__, __FILE__, __LINE__);

        QSqlQuery q(db);
        if(!q.exec("SELECT itemId, datadimension FROM dimension"))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

        while(q.next())
            m_mapTypes.insert(q.value(0).toInt(), q.value(1).toInt());

        m_bTypesLoaded = true;
    }
}
