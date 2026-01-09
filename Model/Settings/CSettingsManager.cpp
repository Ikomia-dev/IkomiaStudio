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

#include "CSettingsManager.h"
#include "Model/Wizard/CWizardManager.h"

QFileDialog::Options CSettingsManager::m_dlgOptions = QFileDialog::Options();

CSettingsManager::CSettingsManager()
{
#ifdef Q_OS_MACOS
    m_bUseNativeDlg = true;
#endif
}

void CSettingsManager::init()
{
    // Init main database and create table settings if necessary
    initMainDb();
}

void CSettingsManager::notifyViewShow()
{
    initTutorialHelperOption();
    initNativeDialogOption();
    initWorkflowOption();
}

std::string CSettingsManager::getWorkflowSaveFolder() const
{
    return m_protocolSaveFolder;
}

void CSettingsManager::initNativeDialogOption()
{
    QJsonObject json = getSettings("useNative");
    if(!json.empty())
    {
        m_bUseNativeDlg = json["bUseNative"].toBool();
        setUseNativeDlg(m_bUseNativeDlg);
    }

    // Update preference dialog
    emit doEnableNativeDialog(m_bUseNativeDlg);
}

void CSettingsManager::initTutorialHelperOption()
{
    try
    {
        QJsonObject json = getSettings("showTuto");
        if(!json.empty())
            m_bShowTuto = json["bShowTuto"].toBool();
    }
    catch (const CException& e)
    {
        // Possible root cause: porting from Qt5 to Qt6 leads to unpossible binary serialization of JSON
        // There is no way to solve this -> fallback to default value
        m_bShowTuto = false;
    }

    // Tell preference dlg if tuto is enabled or not
    emit doEnableTutorialHelper(m_bShowTuto);
}

void CSettingsManager::initWorkflowOption()
{
    m_protocolSaveFolder = Utils::IkomiaApp::getIkomiaFolder() + "/Workflows/";
    QJsonObject json = getSettings("protocol");
    if(!json.empty())
    {
        m_protocolSaveFolder = json["saveFolder"].toString().toStdString();
        if(m_protocolSaveFolder.back() != '/')
            m_protocolSaveFolder += "/";
    }
    emit doSetWorkflowSaveFolder(QString::fromStdString(m_protocolSaveFolder));
}

void CSettingsManager::setSettings(const QString &category, const QJsonObject& jsonData)
{
    QJsonDocument jsonDoc(jsonData);
    auto blob = jsonDoc.toJson(QJsonDocument::JsonFormat::Compact);

    QSqlQuery q(m_mainDb);
    q.prepare("REPLACE INTO settings (name, value) VALUES (:name, :blob);");
    q.bindValue(":name", category);
    q.bindValue(":blob", blob);

    if(!q.exec())
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
}

void CSettingsManager::initMainDb()
{
    // Load main database Ikomia.pcl
    m_mainDb = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(m_mainDb.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, m_mainDb.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QStringList tables = m_mainDb.tables(QSql::Tables);

    // Create settings table if it does not exist
    if(tables.contains("settings") == false)
    {
        QSqlQuery q(m_mainDb);
        if(!q.exec(QString("CREATE TABLE settings (id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, value BLOB);")))
            throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
    }
}

bool CSettingsManager::isNativeDlgEnabled() const
{
    return m_bUseNativeDlg;
}

bool CSettingsManager::isTutorialEnabled() const
{
    return m_bShowTuto;
}

void CSettingsManager::enableTutorial(bool bEnable)
{
    setTutoEnabled(bEnable);
    emit doEnableTutorialHelper(bEnable);
}

QFileDialog::Options CSettingsManager::dialogOptions()
{
    return m_dlgOptions;
}

void CSettingsManager::setUseNativeDlg(bool bEnable)
{
    m_bUseNativeDlg = bEnable;
    if(!m_bUseNativeDlg)
        setDialogOptions(QFileDialog::DontUseNativeDialog);
    else
        setDialogOptions(QFileDialog::Options());

    QJsonObject json;
    json["bUseNative"] = bEnable;
    setSettings("useNative", json);
}

void CSettingsManager::setDialogOptions(QFileDialog::Options options)
{
    m_dlgOptions = options;
}

QJsonObject CSettingsManager::getSettings(const QString &category) const
{
    QJsonObject json;
    QSqlQuery q(m_mainDb);

    if(!q.exec(QString("SELECT value FROM settings WHERE name='%1'").arg(category)))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    if(q.next())
    {
        auto data = q.value(0).toByteArray();
        if(data.isNull() || data.isEmpty())
            throw CException(CoreExCode::NULL_POINTER, QObject::tr("Empty settings data").toStdString(), __func__, __FILE__, __LINE__);

        //Retrieve JSON document and object
        QJsonDocument doc = QJsonDocument::fromJson(data);
        json = doc.object();

        if(json.isEmpty())
            throw CException(CoreExCode::INVALID_JSON_FORMAT, "Invalid JSON data", __func__, __FILE__, __LINE__);
    }
    return json;
}

void CSettingsManager::setTutoEnabled(bool bEnable)
{
    m_bShowTuto = bEnable;
    QJsonObject json;
    json["bShowTuto"] = bEnable;
    setSettings("showTuto", json);
}

void CSettingsManager::onUseNativeDlg(bool bEnable)
{
    setUseNativeDlg(bEnable);
}

void CSettingsManager::onEnableTutorialHelper(bool bEnable)
{
    setTutoEnabled(bEnable);
}

void CSettingsManager::onSetWorkflowSaveFolder(const QString &path)
{
    m_protocolSaveFolder = path.toStdString();
    if(m_protocolSaveFolder.back() != '/')
        m_protocolSaveFolder += "/";

    QJsonObject json;
    json["saveFolder"] = path;
    setSettings("protocol", json);
}
