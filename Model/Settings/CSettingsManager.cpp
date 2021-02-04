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
    initProtocolOption();
}

std::string CSettingsManager::getProtocolSaveFolder() const
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
    QJsonObject json = getSettings("showTuto");
    if(!json.empty())
        m_bShowTuto = json["bShowTuto"].toBool();

    // Tell preference dlg if tuto is enabled or not
    emit doEnableTutorialHelper(m_bShowTuto);
}

void CSettingsManager::initProtocolOption()
{
    m_protocolSaveFolder = Utils::IkomiaApp::getAppFolder() + "/Workflows/";
    QJsonObject json = getSettings("protocol");
    if(!json.empty())
        m_protocolSaveFolder = json["saveFolder"].toString().toStdString();

    emit doSetProtocolSaveFolder(QString::fromStdString(m_protocolSaveFolder));
}

void CSettingsManager::setSettings(const QString &category, const QJsonObject& jsonData)
{
    QJsonDocument jsonDoc(jsonData);
    auto blob = jsonDoc.toBinaryData();

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
        QJsonDocument doc = QJsonDocument::fromBinaryData(data);
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

void CSettingsManager::onSetProtocolSaveFolder(const QString &path)
{
    m_protocolSaveFolder = path.toStdString();
    QJsonObject json;
    json["saveFolder"] = path;
    setSettings("protocol", json);
}
