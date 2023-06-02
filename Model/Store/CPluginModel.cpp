#include "CPluginModel.h"
#include "UtilsTools.hpp"

CPluginModel::CPluginModel(Type type)
{
    m_type = type;
}

CPluginModel::~CPluginModel()
{
    clear();
}

void CPluginModel::setType(Type type)
{
    m_type = type;
}

void CPluginModel::setCurrentUser(const CUser &user)
{
    m_user = user;
    if (m_pModel)
        m_pModel->setCurrentUser(user);
}

void CPluginModel::setCurrentIndex(const QModelIndex &index)
{
    m_currentIndex = index;
}

void CPluginModel::setCurrentWorkspace(const QString &workspace)
{
    m_currentWorkspace = workspace;
}

void CPluginModel::setCurrentRequestUrl(const QString &url)
{
    m_currentRequestUrl = url;
}

void CPluginModel::setQuery(const QString &query, const QSqlDatabase& db)
{
    if (!m_pModel)
        throw CException(CoreExCode::NULL_POINTER, "Plugin model does not exist", __func__, __FILE__, __LINE__);

    m_pModel->setQuery(query, db);
}

void CPluginModel::setTotalPluginCount(int count)
{
    m_totalPluginCount = count;
}

void CPluginModel::setPluginField(int index, const QString& key, const QString& value)
{
    if (index < 0 || index >= m_jsonPlugins.size())
        throw CException(CoreExCode::INDEX_OVERFLOW, "Invalid plugin index (overflow)", __func__, __FILE__, __LINE__);

    QJsonObject plugin = m_jsonPlugins[index].toObject();
    plugin[key] = value;
    m_jsonPlugins[index] = plugin;
}

void CPluginModel::setPackageFile(const QString &file)
{
    m_packageFile = file;
}

CPluginModel::Type CPluginModel::getType() const
{
    return m_type;
}

CStoreQueryModel *CPluginModel::getModel() const
{
    return m_pModel;
}

CUser CPluginModel::getCurrentUser() const
{
    return m_user;
}

QModelIndex CPluginModel::getCurrentIndex() const
{
    return m_currentIndex;
}

QString CPluginModel::getCurrentWorkspace() const
{
    return m_currentWorkspace;
}

QString CPluginModel::getCurrentRequestUrl() const
{
    return m_currentRequestUrl;
}

int CPluginModel::getIntegerField(const QString &fieldName, const QModelIndex &index) const
{
    if (!m_pModel)
        throw CException(CoreExCode::NULL_POINTER, "Plugin model does not exist", __func__, __FILE__, __LINE__);

    if (index.isValid())
        return m_pModel->record(index.row()).value(fieldName).toInt();
    else if(m_currentIndex.isValid())
        return m_pModel->record(m_currentIndex.row()).value(fieldName).toInt();
    else
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "Plugin model index is not valid", __func__, __FILE__, __LINE__);
}

std::string CPluginModel::getStringField(const QString &fieldName, const QModelIndex &index) const
{
    return getQStringField(fieldName, index).toStdString();
}

QString CPluginModel::getQStringField(const QString &fieldName, const QModelIndex &index) const
{
    if (!m_pModel)
        throw CException(CoreExCode::NULL_POINTER, "Plugin model does not exist", __func__, __FILE__, __LINE__);

    if (index.isValid())
        return m_pModel->record(index.row()).value(fieldName).toString();
    else if(m_currentIndex.isValid())
        return m_pModel->record(m_currentIndex.row()).value(fieldName).toString();
    else
        throw CException(CoreExCode::INVALID_MODEL_INDEX, "Plugin model index is not valid", __func__, __FILE__, __LINE__);
}

QJsonArray CPluginModel::getJsonPlugins() const
{
    return m_jsonPlugins;
}

QJsonObject CPluginModel::getJsonPlugin(const QString &name) const
{
    for (int i=0; i<m_jsonPlugins.size(); ++i)
    {
        QJsonObject plugin = m_jsonPlugins[i].toObject();
        if (plugin["name"] == name)
            return plugin;
    }
    std::string msg = "Algorithm " + name.toStdString() + " can't be found";
    throw CException(CoreExCode::NOT_FOUND, msg, __func__, __FILE__, __LINE__);
}

int CPluginModel::getTotalPluginCount() const
{
    return m_totalPluginCount;
}

ApiLanguage CPluginModel::getLanguageFromString(const QString strLanguage) const
{
    if (strLanguage == "CPP")
        return ApiLanguage::CPP;
    else
        return ApiLanguage::PYTHON;
}

QString CPluginModel::getPackageFile() const
{
    return m_packageFile;
}

bool CPluginModel::isComplete() const
{
    return m_totalPluginCount == m_jsonPlugins.size();
}

bool CPluginModel::isPluginExists(const QString &name) const
{
    for (int i=0; i<m_jsonPlugins.size(); ++i)
    {
        QJsonObject plugin = m_jsonPlugins[i].toObject();
        if (plugin["name"] == name)
            return true;
    }
    return false;
}

void CPluginModel::init(const CUser &user, const QString &query, const QSqlDatabase &db)
{
    m_pModel = new CStoreQueryModel;
    m_pModel->setCurrentUser(user);
    m_pModel->setQuery(query, db);
}

void CPluginModel::addJsonPlugin(const QJsonObject &jsonPlugin)
{
    m_jsonPlugins.append(jsonPlugin);
}

void CPluginModel::filterCompatiblePlugins()
{
    std::vector<int> toRemove;
    for (int i=0; i<m_jsonPlugins.size(); ++i)
    {
        QJsonObject plugin = m_jsonPlugins[i].toObject();

        if (checkOSCompatibility(plugin) == false)
        {
            toRemove.push_back(i);
            continue;
        }

        if (checkIkomiaCompatibility(plugin) == false)
        {
            toRemove.push_back(i);
            continue;
        }

        if (checkArchitecture(plugin) == false)
        {
            toRemove.push_back(i);
            continue;
        }
    }

    // Remove incompatible plugins
    if (toRemove.size() > 0)
    {
        for (size_t i=0; i<toRemove.size(); ++i)
            m_jsonPlugins.removeAt(toRemove[i]);
    }
}

bool CPluginModel::checkOSCompatibility(const QJsonObject& plugin) const
{
    // Check OS compatibility
    bool bOSCompatible = false;
    OSType currentOS = Utils::OS::getCurrent();
    QJsonArray jsonPackages = plugin["packages"].toArray();

    for (int i=0; i<jsonPackages.size(); ++i)
    {
        QJsonObject jsonPackage = jsonPackages[i].toObject();
        QJsonObject jsonPlatform = jsonPackage["platform"].toObject();
        QJsonArray jsonOSList = jsonPlatform["os"].toArray();

        std::set<OSType> osList;
        for (int j=0; j<jsonOSList.size(); ++j)
        {
            if (jsonOSList[j] == "LINUX")
                osList.insert(OSType::LINUX);
            else if (jsonOSList[j] == "WINDOWS")
                osList.insert(OSType::WIN);
        }

        auto it = osList.find(currentOS);
        if (it != osList.end())
        {
            bOSCompatible = true;
            break;
        }
    }
    return bOSCompatible;
}

bool CPluginModel::checkIkomiaCompatibility(const QJsonObject &plugin) const
{
    PluginState state;
    bool bVersionCompatible = false;
    ApiLanguage language = getLanguageFromString(plugin["language"].toString());
    QJsonArray jsonPackages = plugin["packages"].toArray();

    for (int i=0; i<jsonPackages.size(); ++i)
    {
        QJsonObject jsonPackage = jsonPackages[i].toObject();
        QJsonObject jsonPlatform = jsonPackage["platform"].toObject();
        QString ikomiaVersions = jsonPlatform["ikomia"].toString();

        // Get min version
        QString ikomiaVersion;
        QRegularExpression re(">=(\\d.\\d.\\d),<(\\d.\\d.\\d)");
        QRegularExpressionMatch match = re.match(ikomiaVersions);

        if(match.hasMatch())
        {
            ikomiaVersion = match.captured(1);

            if(language == ApiLanguage::CPP)
                state = Utils::Plugin::getCppState(ikomiaVersion);
            else
                state = Utils::Plugin::getPythonState(ikomiaVersion);

            if (state == PluginState::VALID || state == PluginState::UPDATED)
            {
                bVersionCompatible = true;
                break;
            }
        }
    }
    return bVersionCompatible;
}

bool CPluginModel::checkArchitecture(const QJsonObject &plugin) const
{
    //Check architecture compatibility
    //TODO
//    QString strLanguage = plugin["language"].toString();
//    if (strLanguage == "CPP")
//    {
//        std::string keywords = plugin["keywords"].toString().toStdString();
//        return Utils::Plugin::checkArchitectureKeywords(keywords);
//    }
    return true;
}

void CPluginModel::clear()
{
    // Clear all
    clearContext();
    m_jsonPlugins = QJsonArray();
    m_totalPluginCount = 0;

    if (m_pModel != nullptr)
    {
        delete m_pModel;
        m_pModel = nullptr;
    }
}

void CPluginModel::clearContext()
{
    // Clear contextual data
    m_currentIndex = QModelIndex();
    m_currentWorkspace.clear();
    m_currentRequestUrl.clear();
    m_packageFile.clear();
}
