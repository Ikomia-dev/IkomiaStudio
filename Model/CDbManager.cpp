#include "CDbManager.h"
#include <QSqlDriver>
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"

CDbManager::CDbManager()
{
}

void CDbManager::init()
{
    initTables();

    if(!m_bFirstLaunch)
        makeMigrations();
    else
        storeCurrentVersion();
}

void CDbManager::initTables()
{
    // Load main database Ikomia.pcl
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        qCritical() << db.lastError().text();

    QStringList tables = db.tables(QSql::Tables);
    m_bFirstLaunch = tables.size() == 0;

    // Create settings table if it does not exist
    if(tables.contains("version") == false)
    {
        QSqlQuery q(db);
        if(!q.exec(QString("CREATE TABLE version (id INTEGER PRIMARY KEY, component TEXT UNIQUE NOT NULL, number TEXT NOT NULL);")))
            qCritical() << q.lastError().text();
    }
}

QString CDbManager::getVersionNumber(const QString &component)
{
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        qCritical() << db.lastError().text();

    QSqlQuery q(db);
    if(!q.exec(QString("SELECT number FROM version WHERE component='%1'").arg(component)))
        qCritical() << q.lastError().text();

    QString versionNumber;
    if(q.next())
        versionNumber = q.value(0).toString();

    return versionNumber;
}

void CDbManager::makeMigrations()
{
    QString currentDbVersion = getVersionNumber("global");
    QString currentAppVersion = Utils::IkomiaApp::getCurrentVersionNumber();

    if(currentDbVersion == currentAppVersion)
    {
        // Everything up to date
        return;
    }
    else
    {
        auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
        if(db.isValid() == false)
            qCritical() << db.lastError().text();

        // Make necessary migrations
        for(auto it=m_migrationMap.begin(); it!=m_migrationMap.end(); ++it)
        {
            if(it.key() <= currentDbVersion)
                continue;

            if(it.key() > currentAppVersion)
                break;

            try
            {
                QString sqlFile = ":/Migrations/" + it.value();
                executeSqlFile(sqlFile, db);
            }
            catch(const std::exception& e)
            {
                qCritical() << QString("Migration for version %1 has failed:").arg(it.key()) + QString::fromStdString(e.what());
                return;
            }
        }
        storeCurrentVersion();
    }
}

void CDbManager::storeCurrentVersion()
{
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
        qCritical() << db.lastError().text();

    QSqlQuery q(db);
    QString currentAppVersion = Utils::IkomiaApp::getCurrentVersionNumber();

    auto strQuery = QString("INSERT INTO version (component, number) VALUES ('%1', '%2') "
                       "ON CONFLICT(component) DO UPDATE SET number = excluded.number;")
            .arg("global").arg(currentAppVersion);

    if(!q.exec(strQuery))
        qCritical() << q.lastError().text();
}

void CDbManager::executeSqlFile(const QString &path, QSqlDatabase &db)
{
    //Read query file content
    QFile qf(path);
    if(!qf.open(QIODevice::ReadOnly))
        throw CException(CoreExCode::INVALID_FILE, "Invalid migration file:" + path.toStdString(), __func__, __FILE__, __LINE__);

    QString queryStr(qf.readAll());
    qf.close();

    //Check if SQL Driver supports Transactions
    if(db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        //Replace comments and tabs and new lines with space
        queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
        //Remove waste spaces
        queryStr = queryStr.trimmed();

        //Extracting queries -> basic parsing, write SQL with caution...
        QStringList qList = queryStr.split(';', QString::SkipEmptyParts);

        //Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
        //NOTE: I used new regular expression for Qt5 as recommended by Qt documentation.
        QRegularExpression re_transaction("\\bbegin.transaction.*", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression re_commit("\\bcommit.*", QRegularExpression::CaseInsensitiveOption);

        //Check if query file is already wrapped with a transaction
        bool isStartedWithTransaction = re_transaction.match(qList.at(0)).hasMatch();
        if(!isStartedWithTransaction)
            db.transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.

        //Execute each individual queries
        foreach(const QString &s, qList)
        {
            if(re_transaction.match(s).hasMatch())  //<== detecting special query
                db.transaction();
            else if(re_commit.match(s).hasMatch())  //<== detecting special query
                db.commit();
            else
            {
                QSqlQuery query(db);
                if(!query.exec(s))
                {
                    db.rollback();                  //<== rollback the transaction if there is any problem
                    throw CException(DatabaseExCode::INVALID_QUERY, query.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
                }
            }
        }

        if(!isStartedWithTransaction)
            db.commit();                            //<== ... completing of wrapping with transaction
    }
    else
    {
        //Sql Driver doesn't supports transaction
        //...so we need to remove special queries (`begin transaction` and `commit`)
        queryStr = queryStr.replace(QRegularExpression("(\\bbegin.transaction.*;|\\bcommit.*;|\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
        queryStr = queryStr.trimmed();

        //Execute each individual queries
        QStringList qList = queryStr.split(';', QString::SkipEmptyParts);
        foreach(const QString &s, qList)
        {
            QSqlQuery query(db);
            if(!query.exec(s))
                throw CException(DatabaseExCode::INVALID_QUERY, query.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }
    }
}

