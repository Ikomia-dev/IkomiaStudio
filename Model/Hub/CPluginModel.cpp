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

CHubQueryModel *CPluginModel::getModel() const
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
    m_pModel = new CHubQueryModel;
    m_pModel->setCurrentUser(user);
    m_pModel->setQuery(query, db);
}

void CPluginModel::addJsonPlugin(const QJsonObject &jsonPlugin)
{
    m_jsonPlugins.append(jsonPlugin);
    updatePluginPackagesInfo(m_jsonPlugins.count() - 1);
}

void CPluginModel::updatePluginPackagesInfo(int index)
{
    if (index >= m_jsonPlugins.size())
        return;

    QJsonObject plugin = m_jsonPlugins[index].toObject();
    QJsonArray packages = plugin["packages"].toArray();

    for (int i=0; i<packages.size(); ++i)
    {
        QJsonObject package = packages[i].toObject();
        QJsonObject platform = package["platform"].toObject();

        if (platform["ikomia"].isNull() == false)
        {
            // Set Ikomia min and max versions
            QString ikomiaVersions = platform["ikomia"].toString();
            QRegularExpression re(">=(\\d+.\\d+.\\d+),?<?(\\d*.?\\d*.?\\d*)");
            QRegularExpressionMatch match = re.match(ikomiaVersions);

            if(match.hasMatch())
            {
                package["ikomia_min_version"] = match.captured(1);
                package["ikomia_max_version"] = match.captured(2);
            }
        }

        // Set Python min and max versions
        if (platform["python"].isNull() == false)
        {
            // Get min and max versions
            QString pythonVersions = platform["python"].toString();
            QRegularExpression re(">=(\\d+.\\d+.\\d*),?<?(\\d*.?\\d*.?\\d*)");
            QRegularExpressionMatch match = re.match(pythonVersions);

            if(match.hasMatch())
            {
                package["python_min_version"] = match.captured(1);
                package["python_max_version"] = match.captured(2);
            }
        }
        packages[i] = package;
    }
    plugin["packages"] = packages;
    m_jsonPlugins[index] = plugin;
}

void CPluginModel::filterCompatiblePlugins()
{
    std::vector<int> toRemove;
    for (int i=0; i<m_jsonPlugins.size(); ++i)
    {
        bool bCompatible = false;
        QJsonObject plugin = m_jsonPlugins[i].toObject();
        ApiLanguage language = getLanguageFromString(plugin["language"].toString());
        QJsonArray packages = plugin["packages"].toArray();

        for (int j=0; j<packages.size(); ++j)
        {
            QJsonObject package = packages[j].toObject();
            if (checkPackageCompatibility(package, language))
            {
                bCompatible = true;
                break;
            }
        }

        if (bCompatible == false)
            toRemove.push_back(i);
    }

    // Remove incompatible plugins
    if (toRemove.size() > 0)
    {
        for (size_t i=0; i<toRemove.size(); ++i)
            m_jsonPlugins.removeAt(toRemove[i]);
    }
}

bool CPluginModel::checkPackageCompatibility(const QJsonObject &package, ApiLanguage language) const
{
    if (checkOSCompatibility(package) == false)
        return false;

    if (checkIkomiaCompatibility(package, language) == false)
        return false;

    if (checkPythonCompatibility(package) == false)
        return false;

    if (checkArchitecture(package, language) == false)
        return false;

    return true;
}

bool CPluginModel::checkOSCompatibility(const QJsonObject& package) const
{
    // Check OS compatibility
    QJsonObject platform = package["platform"].toObject();
    QJsonArray os = platform["os"].toArray();

    std::set<OSType> osList;
    for (int j=0; j<os.size(); ++j)
    {
        if (os[j] == "LINUX")
            osList.insert(OSType::LINUX);
        else if (os[j] == "WINDOWS")
            osList.insert(OSType::WIN);
    }

    OSType currentOS = Utils::OS::getCurrent();
    auto it = osList.find(currentOS);
    return (it != osList.end());
}

bool CPluginModel::checkIkomiaCompatibility(const QJsonObject &package, ApiLanguage language) const
{
    std::string minIkomiaVersion = package["ikomia_min_version"].toString().toStdString();
    std::string maxIkomiaVersion = package["ikomia_max_version"].toString().toStdString();
    PluginState state = Utils::Plugin::getApiCompatibilityState(minIkomiaVersion, maxIkomiaVersion, language);
    return (state == PluginState::VALID);
}

bool CPluginModel::checkPythonCompatibility(const QJsonObject &package) const
{
    CSemanticVersion version(Utils::Python::getVersion());
    std::string minPythonVersion = package["python_min_version"].toString().toStdString();

    if (minPythonVersion.empty() == false)
    {
        CSemanticVersion minVersion(minPythonVersion);
        if (version < minVersion)
            return false;
    }

    std::string maxPythonVersion = package["python_max_version"].toString().toStdString();
    if (maxPythonVersion.empty() == false)
    {
        CSemanticVersion maxVersion(maxPythonVersion);
        if (version >= maxVersion)
            return false;
    }
    return true;
}

bool CPluginModel::checkArchitecture(const QJsonObject &package, ApiLanguage language) const
{
    //Check architecture and features compatibility
    if (language == ApiLanguage::CPP)
    {
        QJsonObject platform = package["platform"].toObject();

        //Check CPU architecture
        bool bArchOk = false;
        QString currentArch = QString::fromStdString(Utils::OS::getCpuArchName(Utils::OS::getCpuArch()));
        QJsonArray platformArch = platform["architecture"].toArray();

        for (int i=0; i<platformArch.size(); ++i)
        {
            if (currentArch == platformArch[i].toString())
            {
                bArchOk = true;
                break;
            }
        }

        if (bArchOk == false)
            return false;

        //Check features: CUDA version
        QJsonArray currentFeatures;
        currentFeatures.append(QString::fromStdString(Utils::OS::getCudaVersionName()));
        QJsonArray platformFeatures = platform["features"].toArray();
        return currentFeatures == platformFeatures;
    }
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
