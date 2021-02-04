#include "CWizardDbManager.h"
#include <QSqlError>
#include "CWizardScenario.h"
#include "CException.h"
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"

CWizardDbManager::CWizardDbManager()
{

}

void CWizardDbManager::initDb()
{
    createTutoDb();
}

QSqlDatabase CWizardDbManager::getTutorialDatabase() const
{
    auto db = Utils::Database::connect(m_name, m_connectionName);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    return db;
}

QString CWizardDbManager::getAllTutorialsQuery() const
{
    return QString("SELECT * FROM tutorials");
}

void CWizardDbManager::addTutorial(const CWizardScenarioInfo& pScenarioInfo)
{
    auto db = Utils::Database::connect(m_name, m_connectionName);
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec(QString("INSERT INTO tutorials "
                       "(name, description, keywords, authors, iconPath) "
                       "VALUES ('%1', '%2', '%3', '%4', '%5') ")
               .arg(QString::fromStdString(pScenarioInfo.getName()))
               .arg(QString::fromStdString(pScenarioInfo.getDescription()))
               .arg(QString::fromStdString(pScenarioInfo.getKeywords()))
               .arg(QString::fromStdString(pScenarioInfo.getAuthors()))
               .arg(QString::fromStdString(pScenarioInfo.getIconPath()))
               ))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

void CWizardDbManager::createTutoDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(m_type, m_connectionName);
    if(!db.isValid())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    db.setDatabaseName(m_name);

    if(!db.open())
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec("CREATE TABLE tutorials ("
               "id INTEGER PRIMARY KEY, name TEXT NOT NULL, description TEXT, keywords TEXT, authors TEXT, iconPath TEXT);"))
    {
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }

    if(!q.exec("CREATE VIRTUAL TABLE tutoFTS USING fts5(id, name, description, keywords);"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}
