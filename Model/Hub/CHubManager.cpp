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

#include "CHubManager.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpPart>
#include <QtConcurrent>
#include "CHubQueryModel.h"
#include "CHubOnlineIconManager.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Core/CPluginTools.h"
#include "Model/Process/CProcessManager.h"
#include "Model/Plugin/CPluginManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/Common/CHttpRequest.h"
#include "JlCompress.h"
#include "Core/CIkomiaRegistry.h"

//---------------//
//- CHubManager -//
//---------------//
CHubManager::CHubManager():
    m_hubPluginModel(CPluginModel::Type::HUB),
    m_workspacePluginModel(CPluginModel::Type::WORKSPACE),
    m_localPluginModel(CPluginModel::Type::LOCAL)
{
    try
    {
        m_dbMgr.initDb();
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote() << QString::fromStdString(e.what());
    }
}

CHubManager::~CHubManager()
{
    deleteTransferFiles();
}

void CHubManager::setManagers(QNetworkAccessManager *pNetworkMgr, CProcessManager *pProcessMgr, CPluginManager *pPluginMgr,
                              CProgressBarManager *pProgressMgr, CWorkflowManager* pWorkflowMgr)
{
    m_pNetworkMgr = pNetworkMgr;
    m_pProcessMgr = pProcessMgr;
    m_pPluginMgr = pPluginMgr;
    m_pProgressMgr = pProgressMgr;
    m_pWorkflowMgr = pWorkflowMgr;
    checkPendingUpdates();
}

void CHubManager::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
    m_hubPluginModel.setCurrentUser(user);
    m_workspacePluginModel.setCurrentUser(user);
}

void CHubManager::onRequestHubModel()
{
    m_hubPluginModel.clear();
    m_dbMgr.clearServerRecords(m_hubPluginModel.getType());
    createHubPluginModel();
}

void CHubManager::onRequestWorkspaceModel()
{
    m_workspacePluginModel.clear();
    m_dbMgr.clearServerRecords(m_workspacePluginModel.getType());
    createWorkspacePluginModel();
}

void CHubManager::onRequestLocalModel()
{
    m_localPluginModel.clear();
    createQueryModel(&m_localPluginModel);
}

void CHubManager::onRequestNextPublishInfo(const QModelIndex &index)
{
    m_workspacePluginModel.setCurrentIndex(index);
    auto name = m_workspacePluginModel.getQStringField("name", index);
    QJsonObject pluginInfo = m_workspacePluginModel.getJsonPlugin(name);

    try
    {
        CHttpRequest request(pluginInfo["url"].toString() + "publish/", "application/json", m_currentUser);
        auto pReply = m_pNetworkMgr->get(request);
        connect(pReply, &QNetworkReply::finished, [=]{
            this->onReplyReceived(pReply, &m_workspacePluginModel, HubRequestType::GET_NEXT_PUBLISH_INFO);
        });
    }
    catch(CException& e)
    {
        qCWarning(logHub) << e.what();
        emit doSetNextPublishInfo(index, QJsonObject());
        return;
    }
}

void CHubManager::onPublishHub(const QModelIndex &index, const QJsonObject& info)
{
    assert(index.isValid());

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("An algorithm is already being published. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to publish plugins
    if(m_currentUser.isConnected() == false)
    {
        qCCritical(logHub).noquote() << tr("You have to login before publishing algorithms");
        return;
    }

    publishToHub(index, info);
}

void CHubManager::onPublishWorkspace(const QModelIndex &index, const QString& workspace)
{
    assert(index.isValid());

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("An algorithm is already being published. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to publish plugins
    if(m_currentUser.isConnected() == false)
    {
        qCCritical(logHub).noquote() << tr("You have to login before publishing algorithms");
        return;
    }

    publishToWorkspace(index, workspace);
}

void CHubManager::onInstallPlugin(CPluginModel::Type type, const QModelIndex &index)
{
    CPluginModel* pModel = nullptr;
    switch(type)
    {
        case CPluginModel::Type::HUB:
            pModel = &m_hubPluginModel;
            break;
        case CPluginModel::Type::WORKSPACE:
            pModel = &m_workspacePluginModel;
            break;
    }

    auto name = pModel->getQStringField("name", index);
    pModel->setCurrentIndex(index);
    pModel->setPackageFile(name + ".zip");
    QJsonObject pluginInfo = pModel->getJsonPlugin(name);
    queryServerInstallPlugin(pModel, pluginInfo["url"].toString());
}

void CHubManager::onHubSearchChanged(const QString &text)
{
    try
    {
        auto query = getQuery(CPluginModel::Type::HUB, text);
        m_hubPluginModel.setQuery(query, m_dbMgr.getPluginsDatabase(CPluginModel::Type::HUB));
    }
    catch(std::exception& e)
    {
        Utils::print(e.what(), QtMsgType::QtDebugMsg);
    }
}

void CHubManager::onWorkspaceSearchChanged(const QString &text)
{
    try
    {
        auto query = getQuery(CPluginModel::Type::WORKSPACE, text);
        m_workspacePluginModel.setQuery(query, m_dbMgr.getPluginsDatabase(CPluginModel::Type::WORKSPACE));
    }
    catch(std::exception& e)
    {
        Utils::print(e.what(), QtMsgType::QtDebugMsg);
    }
}

void CHubManager::onLocalSearchChanged(const QString &text)
{
    try
    {
        auto query = getQuery(CPluginModel::Type::LOCAL, text);
        m_localPluginModel.setQuery(query, m_dbMgr.getPluginsDatabase(CPluginModel::Type::LOCAL));
    }
    catch(std::exception& e)
    {
        Utils::print(e.what(), QtMsgType::QtDebugMsg);
    }
}

void CHubManager::onReplyReceived(QNetworkReply *pReply, CPluginModel* pModel, HubRequestType requestType)
{
    if (pReply == nullptr)
    {
        clearContext(pModel, true);
        qCCritical(logHub).noquote() << "Invalid reply from Ikomia HUB";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        clearContext(pModel, true);
        qCCritical(logHub).noquote() << pReply->errorString();
        QString content(pReply->readAll());
        Utils::print(content.toStdString(), QtDebugMsg);
        pReply->deleteLater();
        return;
    }

    switch(requestType)
    {
        case HubRequestType::GET_PLUGINS:
            fillServerPluginModel(pModel, pReply);
            break;
        case HubRequestType::GET_PLUGIN_DETAILS:
            addPluginsToModel(pModel, pReply);
            break;
        case HubRequestType::GET_NEXT_PUBLISH_INFO:
            sendNextPublishInfo(pModel, pReply);
            break;
        case HubRequestType::GET_PACKAGE_URL:
            downloadPluginPackage(pModel, pReply);
            break;
        case HubRequestType::PUBLISH_WORKSPACE:
            uploadPluginIcon(pReply);
            break;
        case HubRequestType::PUBLISH_HUB:
            finalyzePublishHub();
            break;
        case HubRequestType::UPLOAD_ICON:
            uploadPluginPackage();
            break;
        case HubRequestType::UPLOAD_PACKAGE:
            uploadDemoWorkflow();
            break;
        case HubRequestType::UPLOAD_WORKFLOW:
            finalizePublishWorkspace();
            break;
        case HubRequestType::DOWNLOAD_PACKAGE:
            savePluginFolder(pModel, pReply);
            break;
    }
    pReply->deleteLater();
}

void CHubManager::onUpdateProgress(qint64 bytesSent, qint64 bytesTotal, CProgressCircle* pProgress, const QString& msg)
{
    if (pProgress->maximum() == 0)
        pProgress->setMaximum(bytesTotal/1024);

    const float factor = 1024.0*1024.0;
    QString sent = QString::number(bytesSent / factor, 'f', 1);
    QString total = QString::number(bytesTotal / factor, 'f', 1);
    emit pProgress->doSetMessage(QString("%1: %2 Mb / %3 Mb").arg(msg).arg(sent).arg(total));
    emit pProgress->setValue(bytesSent / 1024);
}

void CHubManager::initTransferProgress(QNetworkReply *pReply, const QString& msg, size_t steps)
{
    auto pProgressSignal = new CProgressSignalHandler;
    pProgressSignal->setParent(pReply);
    CProgressCircle* pProgress = m_pProgressMgr->launchProgress(pProgressSignal, steps, msg, false);

    connect(pReply, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal)
    {
        onUpdateProgress(bytesSent, bytesTotal, pProgress, msg);
    });
    connect(pReply, &QNetworkReply::downloadProgress, [=](qint64 bytesSent, qint64 bytesTotal)
    {
        onUpdateProgress(bytesSent, bytesTotal, pProgress, msg);
    });
    connect(pReply, &QNetworkReply::finished, [=](){
       pProgressSignal->doFinish();
       pProgressSignal->deleteLater();
    });
}

QJsonObject CHubManager::getJsonObject(QNetworkReply *pReply, const QString &errorMsg) const
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logHub).noquote() << errorMsg << tr(": invalid JSON document");
        return QJsonObject();
    }

    if(doc.isObject() == false)
    {
        qCCritical(logHub).noquote() << errorMsg << tr(":invalid JSON document structure");
        return QJsonObject();
    }
    return doc.object();
}

QString CHubManager::getQuery(CPluginModel::Type serverType, const QString& text) const
{
    QString query;
    if(text.isEmpty())
        query = m_dbMgr.getAllPluginsQuery(serverType);
    else
        query = m_dbMgr.getSearchQuery(serverType, text);

    return query;
}

void CHubManager::queryServerPlugins(CPluginModel* pModel, const QString& strUrl)
{
    assert(m_pNetworkMgr);

    try
    {
        // Get algorithms count
        QString url = QString("%1?page_size=1").arg(strUrl);
        CHttpRequest request(url, "application/json");

        if (pModel->getType() == CPluginModel::Type::WORKSPACE)
            request.setUserAuth(m_currentUser);

        auto pReply = m_pNetworkMgr->get(request);
        connect(pReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pReply, pModel, HubRequestType::GET_PLUGINS);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(pModel, true);
        return;
    }
}

void CHubManager::queryServerPluginDetails(CPluginModel* pModel, int pluginsCount)
{
    assert(m_pNetworkMgr);

    try
    {
        QString url = QString("%1?page_size=%2&fields=all").arg(pModel->getCurrentRequestUrl()).arg(pluginsCount);
        CHttpRequest request(url, "application/json");

        if (pModel->getType() == CPluginModel::Type::WORKSPACE)
            request.setUserAuth(m_currentUser);

        auto pReply = m_pNetworkMgr->get(request);
        connect(pReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pReply, pModel, HubRequestType::GET_PLUGIN_DETAILS);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(pModel, true);
        return;
    }
}

void CHubManager::queryServerInstallPlugin(CPluginModel* pModel, const QString& strUrl)
{
    assert(m_pNetworkMgr);

    if (m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("An algorithm is already being installed. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    try
    {
        //Http request to get plugin packages
        CHttpRequest request(strUrl, "application/json");
        if (pModel->getType() == CPluginModel::Type::WORKSPACE)
            request.setUserAuth(m_currentUser);

        m_bBusy = true;
        auto pReply = m_pNetworkMgr->get(request);
        connect(pReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pReply, pModel, HubRequestType::GET_PACKAGE_URL);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(pModel, true);
        return;
    }
}

void CHubManager::createHubPluginModel()
{
    QString url = Utils::Network::getBaseUrl() + "/v1/hub/";
    m_hubPluginModel.setCurrentRequestUrl(url);
    queryServerPlugins(&m_hubPluginModel, url);
    m_pProgressMgr->launchInfiniteProgress(tr("Loading algorithms from Ikomia HUB..."), false);
}

void CHubManager::createWorkspacePluginModel()
{
    QString url = Utils::Network::getBaseUrl() + "/v1/algos/";
    m_workspacePluginModel.setCurrentRequestUrl(url);
    queryServerPlugins(&m_workspacePluginModel, url);
    m_pProgressMgr->launchInfiniteProgress(tr("Loading algorithms from private workspaces..."), false);
}

void CHubManager::createQueryModel(CPluginModel* pModel)
{
    auto type = pModel->getType();
    pModel->init(m_currentUser, m_dbMgr.getAllPluginsQuery(type), m_dbMgr.getPluginsDatabase(type));
    emit doSetPluginModel(pModel);
}

QByteArray CHubManager::createPluginPayload(CPluginModel* pModel)
{
    QJsonObject plugin;

    //Plugin name - mandatory
    QString name = pModel->getQStringField("name");
    if(name.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: algorithm name is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["name"] = name;

    //Short description - mandatory
    QString shortDescription = pModel->getQStringField("shortDescription");
    if(shortDescription.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: short description is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["short_description"] = shortDescription;

    //Description - mandatory
    QString description = pModel->getDescription();
    if(description.isEmpty())
        description = shortDescription;

    if(description.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: description is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["description"] = description;

    //Keywords - optional
    QString keywords = pModel->getQStringField("keywords");
    if(keywords.isEmpty() == false)
    {
        std::vector<std::string> keywordList;
        Utils::String::tokenize(keywords.toStdString(), keywordList, ",");

        QJsonArray jsonKeywords;
        for (size_t i=0; i<keywordList.size(); ++i)
            jsonKeywords.append(QString::fromStdString(keywordList[i]));

        plugin["keywords"] = jsonKeywords;
    }

    //----- Paper -----//
    QJsonObject paper;
    //Authors - mandatory
    QString authors = pModel->getQStringField("authors");
    if(authors.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: author is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    paper["authors"] = authors;

    //Article - optional
    QString article = pModel->getQStringField("article");
    if(article.isEmpty() == false)
        paper["title"] = article;

    //Journal - optional
    QString journal = pModel->getQStringField("journal");
    if(journal.isEmpty() == false)
        paper["journal"] = journal;

    //Year - optional
    paper["year"] = pModel->getIntegerField("year");

    //Link
    paper["link"] = pModel->getQStringField("articleUrl");

    plugin["paper"] = paper;

    //Online documentation - optional
    /*QString docLink = m_localPluginModel.getQStringField("docLink");
    if(docLink.isEmpty() == false)
        plugin["docLink"] = docLink;*/

    //License - optional
    QString license = m_localPluginModel.getQStringField("license");
    if(license.isEmpty() == false)
        plugin["license"] = QString::fromStdString(Utils::Plugin::getLicenseString(Utils::Plugin::getLicenseFromName(license.toStdString())));

    //Repository - optional
    QString repository = pModel->getQStringField("repository");
    if(repository.isEmpty() == false)
        plugin["repository"] = repository;

    //Original Repository - optional
    QString originalRepository = pModel->getQStringField("originalRepository");
    if(originalRepository.isEmpty() == false)
        plugin["original_implementation_repository"] = originalRepository;

    //Program language
    auto language = static_cast<ApiLanguage>(pModel->getIntegerField("language"));
    plugin["language"] = QString::fromStdString(Utils::Plugin::getLanguageString(language));

    //Algo type: DATASET, INFER, TRAIN, OTHER...
    auto algoType = static_cast<AlgoType>(pModel->getIntegerField("algoType"));
    plugin["algo_type"] = QString::fromStdString(Utils::Plugin::getAlgoTypeString(algoType));

    //Algo tasks: object detection, instance segmentation...
    QString algoTasks = pModel->getQStringField("algoTasks");
    QStringList strTasks = algoTasks.split(",", Qt::SkipEmptyParts);

    QJsonArray jsonTasks;
    for (int i=0; i<strTasks.size(); ++i)
        jsonTasks.append(strTasks[i]);

    plugin["algo_task"] = jsonTasks;

    QJsonDocument doc(plugin);
    return doc.toJson();
}

QJsonObject CHubManager::createPluginPackagePayload(CPluginModel *pModel)
{
    QJsonObject package;
    int language = pModel->getIntegerField("language");

    //Ikomia minimum version: if empty, set the current version
    QString minIkVersion = pModel->getQStringField("minIkomiaVersion");
    if(minIkVersion.isEmpty())
        minIkVersion = QString::fromStdString(Utils::IkomiaApp::getCurrentVersionNumber());

    package["ikomia_min_version"] = minIkVersion;

    //Ikomia maximum version
    QString maxIkVersion = pModel->getQStringField("maxIkomiaVersion");
    if (maxIkVersion.isEmpty())
    {
        //For C++ and since the API is not stable, binary compatibility is not ensured between minor versions
        CSemanticVersion maxVersion(minIkVersion.toStdString());
        if (language == ApiLanguage::CPP)
            maxVersion.nextMinor();
        else
            maxVersion.nextMajor();

        maxIkVersion = QString::fromStdString(maxVersion.toString());
    }
    package["ikomia_max_version"] = maxIkVersion;

    //Python minimum version
    QString minPythonVersion;
    if (language == ApiLanguage::CPP)
    {
        //For C++ Python version is determined at compile time -> so curent version
        minPythonVersion = QString::fromStdString(Utils::Python::getVersion());
    }
    else
    {
        minPythonVersion = pModel->getQStringField("minPythonVersion");
        if (minPythonVersion.isEmpty())
            minPythonVersion = QString::fromStdString(Utils::Python::getMinSupportedVersion());
    }
    package["python_min_version"] = minPythonVersion;

    //Python maximum version
    QString maxPythonVersion;
    if (language == ApiLanguage::CPP)
    {
        //For C++ we link with Python shared lib, so compatibility is only ensured for same major.minor versions
        CSemanticVersion maxVersion(minPythonVersion.toStdString());
        maxVersion.nextMinor();
        maxPythonVersion = QString::fromStdString(maxVersion.toString());
    }
    else
    {
        maxPythonVersion = pModel->getQStringField("maxPythonVersion");
        if (maxPythonVersion.isEmpty())
        {
            CSemanticVersion maxVersion(Utils::Python::getMaxSupportedVersion());
            maxVersion.nextMinor();
            maxPythonVersion = QString::fromStdString(maxVersion.toString());
        }
        else
        {
            CSemanticVersion maxVersion(maxPythonVersion.toStdString());
            maxVersion.nextMinor();
            maxPythonVersion = QString::fromStdString(maxVersion.toString());
        }
    }
    package["python_max_version"] = maxPythonVersion;

    // OS
    QJsonArray osList;
    OSType os = static_cast<OSType>(pModel->getIntegerField("os"));
    if (os == OSType::LINUX || os == OSType::WIN)
        osList.append(QString::fromStdString(Utils::OS::getName(os)));
    else if (os == OSType::ALL)
    {
        osList.append(QString::fromStdString(Utils::OS::getName(OSType::LINUX)));
        osList.append(QString::fromStdString(Utils::OS::getName(OSType::WIN)));
    }
    package["os"] = osList;

    if (language == ApiLanguage::CPP)
    {
        //Architecture
        QJsonArray arch;
        arch.append(QString::fromStdString(Utils::OS::getCpuArchName(Utils::OS::getCpuArch())));
        package["architecture"] = arch;

        //Features = CUDA version (for now)
        QJsonArray features;
        features.append(QString::fromStdString(Utils::OS::getCudaVersionName()));
        package["features"] = features;
    }
    return package;
}

QString CHubManager::createDemoWorkflow()
{
    CPyEnsureGIL gil;
    std::string name = m_localPluginModel.getStringField("name");
    std::string workflowFile = Utils::CPluginTools::getTransferPath() + "/" + Utils::String::httpFormat(name) + ".json";
    boost::python::str strAlgoName(name.c_str());
    boost::python::object algoModule = Utils::CPluginTools::loadPythonModule("ikomia.utils.algorithm", false);
    boost::python::object wf = algoModule.attr("create_demo_workflow")(strAlgoName);
    boost::python::str strWorkflowFile(workflowFile.c_str());
    wf.attr("save")(strWorkflowFile);
    return QString::fromStdString(workflowFile);
}

QString CHubManager::checkPythonPluginDirectory(const QString &directory)
{
    QDir pluginDir(directory);
    QString dirName = QString::fromStdString(Utils::File::getFileNameWithoutExtension(directory.toStdString()));
    QString goodDirName = dirName;
    QString newDirectory = directory;
    QRegularExpression re("([a-zA-Z0-9\-\._#@=]+)_process");

    foreach (QString fileName, pluginDir.entryList(QDir::Files))
    {
        QString fileNameNoExt = QString::fromStdString(Utils::File::getFileNameWithoutExtension(fileName.toStdString()));
        QRegularExpressionMatch match = re.match(fileNameNoExt);

        if(match.hasMatch())
        {
            goodDirName = match.captured(1);
            break;
        }
    }

    if(dirName != goodDirName)
    {
        newDirectory.replace(dirName, goodDirName);

        QDir oldDir(newDirectory);
        if(oldDir.exists() == true)
            oldDir.removeRecursively();

        pluginDir.rename(directory, newDirectory);
    }
    return newDirectory;
}

QString CHubManager::checkCppPluginDirectory(const QString &directory, const QString &name)
{
    QDir pluginDir(directory);
    QString newDirectory = directory;
    QString dirName = QString::fromStdString(Utils::File::getFileNameWithoutExtension(directory.toStdString()));
    auto goodDirName = Utils::File::conformName(name);

    if(dirName != goodDirName)
    {
        newDirectory.replace(dirName, goodDirName);

        QDir oldDir(newDirectory);
        if(oldDir.exists() == true)
            oldDir.removeRecursively();

        pluginDir.rename(directory, newDirectory);
    }
    return newDirectory;
}

void CHubManager::checkPendingUpdates()
{
    QSettings ikomiaSettings;
    const QString settingKey = "pending_plugin_update";
    QString pendingUpdates = ikomiaSettings.value(settingKey).toString();

    if(pendingUpdates.isEmpty())
        return;

    std::vector<std::string> pluginNames;
    Utils::String::tokenize(pendingUpdates.toStdString(), pluginNames, ";");

    for(size_t i=0; i<pluginNames.size(); ++i)
    {
        //Check if plugin archive exits
        QString zipPath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/" + pluginNames[i]);
        QFile zipFile(zipPath);

        if(zipFile.exists() == false)
            continue;

        std::string dirName = Utils::File::getFileNameWithoutExtension(pluginNames[i]);
        std::string destDir = Utils::Plugin::getCppPath() + "/" + dirName;
        boost::filesystem::path pluginFolder(destDir);

        //Remove plugin folder
        try
        {
            boost::filesystem::remove_all(pluginFolder);
        }
        catch (const boost::filesystem::filesystem_error& e)
        {
            qCCritical(logHub).noquote() << QString::fromStdString(e.code().message());
            continue;
        }

        //Extract new plugin content
        auto extractedFiles = JlCompress::extractDir(zipPath, QString::fromStdString(destDir));
        if(extractedFiles.size() == 0)
            qCCritical(logHub).noquote() << tr("Archive extraction failed: installation of %1 aborted").arg(QString::fromStdString(dirName));

        //Delete archive file
        zipFile.remove();
    }

    ikomiaSettings.setValue(settingKey, "");
}

void CHubManager::checkInstalledModules(const QString &pluginDir)
{
    auto modules = Utils::Python::getInstalledModules();

    //Remove global blacklisted packages
    auto to_remove = CIkomiaRegistry::getBlackListedPackages();
    for (auto&& name : to_remove)
    {
        auto it = std::find_if(modules.begin(), modules.end(),
                               [&](const std::pair<std::string, std::string>& mod){ return mod.first == name; });
        if (it != modules.end())
        {
            if (name == "tb-nightly")
            {
                Utils::Python::uninstallPackage(QString::fromStdString(name));
                auto itTb = std::find_if(modules.begin(), modules.end(),
                                         [&](const std::pair<std::string, std::string>& mod){ return mod.first == "tensorboard"; });
                if (itTb != modules.end())
                {
                    Utils::Python::uninstallPackage("tensorboard");
                    Utils::Python::installPackage("tensorboard", QString::fromStdString(itTb->second));
                }
            }
            else
                Utils::Python::uninstallPackage(QString::fromStdString(name));
        }
    }

    //Remove plugin specific blacklisted packages
    QString needlessFilePath = pluginDir + "/needless.txt";
    if (QFile::exists(needlessFilePath))
    {
        QFile needlessFile(needlessFilePath);
        if (needlessFile.open(QIODevice::ReadOnly))
        {
            QTextStream in(&needlessFile);
            while (!in.atEnd())
            {
                QString package = in.readLine();
                Utils::Python::uninstallPackage(package);
            }
        }
    }
}

void CHubManager::updateWorkspacePlugin(const QString& name)
{
    //Http request to update algorithm
    QByteArray jsonPayload = createPluginPayload(&m_localPluginModel);
    QJsonObject plugin = m_workspacePluginModel.getJsonPlugin(name);

    try
    {
        CHttpRequest request(plugin["url"].toString(), "application/json", m_currentUser);
        auto pReply = m_pNetworkMgr->sendCustomRequest(request, "PATCH", jsonPayload);
        connect(pReply, &QNetworkReply::finished, [=](){
           this->onReplyReceived(pReply, &m_workspacePluginModel, HubRequestType::PUBLISH_WORKSPACE);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(&m_localPluginModel, true);
        return;
    }
}

void CHubManager::updateLocalPlugin()
{
    try
    {
        QString pluginName = m_localPluginModel.getQStringField("name");
        m_dbMgr.updateLocalPluginModifiedDate(pluginName);
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote() << e.what();
    }
}

void CHubManager::fillServerPluginModel(CPluginModel* pModel, QNetworkReply* pReply)
{
    QJsonObject jsonPage = getJsonObject(pReply, tr("Error while retrieving algorithm list"));
    if (jsonPage.isEmpty())
    {
        clearContext(pModel, true);
        return;
    }

    int count = jsonPage["count"].toInt();
    queryServerPluginDetails(pModel, count);
}

void CHubManager::addPluginsToModel(CPluginModel *pModel, QNetworkReply *pReply)
{
    QJsonObject jsonPlugins = getJsonObject(pReply, tr("Error while retrieving algorihtm details"));
    if (jsonPlugins.isEmpty())
    {
        clearContext(pModel, true);
        return;
    }

    pModel->setJsonPlugins(jsonPlugins);
    pModel->filterCompatiblePlugins();

    auto pIconMgr = new CHubOnlineIconManager(pModel, m_pNetworkMgr, m_currentUser);
    connect(pIconMgr, &CHubOnlineIconManager::doIconsLoaded, [this, pIconMgr, pModel]
    {
        pIconMgr->deleteLater();
        validateServerPluginModel(pModel);
    });

    try
    {
        pIconMgr->loadIcons();
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote() << QString::fromStdString(e.what());
    }
}

void CHubManager::validateServerPluginModel(CPluginModel* pModel)
{
    m_pProgressMgr->endInfiniteProgress();
    try
    {
        //Add plugins to local db
        m_dbMgr.insertPlugins(pModel);
        //Mise à jour des infos des plugins locaux (synchronisation)
        m_dbMgr.updateMemoryLocalPluginsInfo();
        //Réinitialisation des données du modèle pour les plugins locaux
        m_localPluginModel.setQuery(m_dbMgr.getAllPluginsQuery(CPluginModel::Type::LOCAL), m_dbMgr.getPluginsDatabase(CPluginModel::Type::LOCAL));
        //Création du model pour les plugins du serveur
        createQueryModel(pModel);
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote() << QString::fromStdString(e.what());
    }
}

void CHubManager::validatePluginFolder(CPluginModel* pModel, const QStringList& files, const QString dstDir)
{
    if(files.isEmpty())
    {
        qCCritical(logHub).noquote() << tr("Archive extraction failed: installation aborted");
        clearContext(pModel, true);
        return;
    }

    try
    {
        //Get plugin info from server
        CUser pluginUser;
        pluginUser.m_id = pModel->getIntegerField("userId");
        pluginUser.m_firstName = pModel->getQStringField("user");
        CTaskInfo procInfo;
        procInfo.m_name = pModel->getStringField("name");
        procInfo.m_shortDescription = pModel->getStringField("shortDescription");
        procInfo.m_description = pModel->getStringField("description");
        procInfo.m_keywords = pModel->getStringField("keywords");
        procInfo.m_authors = pModel->getStringField("authors");
        procInfo.m_article = pModel->getStringField("article");
        procInfo.m_journal = pModel->getStringField("journal");
        procInfo.m_year = pModel->getIntegerField("year");
        procInfo.m_docLink = pModel->getStringField("docLink");
        procInfo.m_createdDate = pModel->getStringField("createdDate");
        procInfo.m_modifiedDate = pModel->getStringField("modifiedDate");
        procInfo.m_version = pModel->getStringField("version");
        procInfo.m_minIkomiaVersion = pModel->getStringField("ikomiaVersion");
        procInfo.m_license = pModel->getStringField("license");
        procInfo.m_repo = pModel->getStringField("repository");
        procInfo.m_language = pModel->getIntegerField("language") == 0 ? ApiLanguage::CPP : ApiLanguage::PYTHON;
        procInfo.m_os = static_cast<OSType>(pModel->getIntegerField("os"));

        //Package file name could have been changed during upload
        //We have to check and correct this name if necessary
        QString validDstDir;
        if(procInfo.m_language == ApiLanguage::PYTHON)
            validDstDir = checkPythonPluginDirectory(dstDir);
        else
            validDstDir = checkCppPluginDirectory(dstDir, QString::fromStdString(procInfo.m_name));

        //Asynchronous call -> install plugin dependencies
        installPythonPluginDependencies(pModel, validDstDir, procInfo, pluginUser);
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote().noquote() << QString::fromStdString(e.what());
        //Clean
        clearContext(pModel, true);
    }
}

void CHubManager::generateZipFile()
{
    assert(m_pProgressMgr);

    auto name = m_localPluginModel.getQStringField("name");
    auto language = m_localPluginModel.getIntegerField("language");
    QString zipFilePath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/" + Utils::String::httpFormat(name.toStdString()) + ".zip");

    QString pluginDir;
    if(language == ApiLanguage::CPP)
        pluginDir = QString::fromStdString(Utils::CPluginTools::getCppValidPluginFolder(name.toStdString()));
    else if(language == ApiLanguage::PYTHON)
        pluginDir = QString::fromStdString(Utils::CPluginTools::getPythonPluginFolder(name.toStdString()));

    if(pluginDir.isEmpty())
    {
        qCritical().noquote() << tr("Unable to find folder of plugin %1").arg(name);
        return;
    }

    m_pProgressMgr->launchInfiniteProgress(tr("Algorithm package compression..."), false);

    QFutureWatcher<bool>* pWatcher = new QFutureWatcher<bool>;
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, zipFilePath]
    {
        bool bCompress = pWatcher->result();
        QString zipFile = (bCompress == true ? zipFilePath : QString());
        m_pProgressMgr->endInfiniteProgress();
        publishOrUpdateToWorkspace(zipFile);
    });

    //Compress plugin into separate thread
    auto future = QtConcurrent::run([zipFilePath, pluginDir]
    {
        return JlCompress::compressDir(zipFilePath, pluginDir, true);
    });
    pWatcher->setFuture(future);
}

void CHubManager::extractZipFile(CPluginModel* pModel, const QString& src, const QString& dstDir)
{
    m_pProgressMgr->launchInfiniteProgress(tr("Algorithm package extraction..."), false);

    QFutureWatcher<QStringList>* pWatcher = new QFutureWatcher<QStringList>(this);
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, pModel, src, dstDir]
    {
        //Delete archive file
        QFile file(src);
        file.remove();

        m_pProgressMgr->endInfiniteProgress();
        QStringList files = pWatcher->result();
        validatePluginFolder(pModel, files, dstDir);
    });

    //Extract plugin into separate thread
    auto future = QtConcurrent::run([src, dstDir]
    {
        return JlCompress::extractDir(src, dstDir);
    });
    pWatcher->setFuture(future);
}

void CHubManager::publishToHub(const QModelIndex& index, const QJsonObject& info)
{
    m_workspacePluginModel.setCurrentIndex(index);
    auto name = m_workspacePluginModel.getQStringField("name");
    QJsonObject pluginInfo = m_workspacePluginModel.getJsonPlugin(name);

    try
    {
        m_bBusy = true;
        CHttpRequest request(pluginInfo["url"].toString() + "publish/", "application/json", m_currentUser);
        QJsonDocument payload(info);

        auto pReply = m_pNetworkMgr->put(request, payload.toJson());
        connect(pReply, &QNetworkReply::finished, [=]{
            this->onReplyReceived(pReply, &m_hubPluginModel, HubRequestType::PUBLISH_HUB);
        });
    }
    catch(CException& e)
    {
        qCWarning(logHub) << e.what();
        return;
    }
}

void CHubManager::publishToWorkspace(const QModelIndex &index, const QString& workspace)
{
    m_bBusy = true;
    m_localPluginModel.setCurrentIndex(index);
    m_workspacePluginModel.setCurrentWorkspace(workspace);
    //Asynchronous call -> plugin compression is made into separate thread
    generateZipFile();
}

void CHubManager::publishOrUpdateToWorkspace(const QString& zipFile)
{
    try
    {
        if(zipFile.isEmpty())
        {
            qCCritical(logHub).noquote() << tr("Compression of algorithm package failed, transfer to Ikomia HUB aborted");
            clearContext(&m_localPluginModel, true);
            return;
        }

        m_workspacePluginModel.setPackageFile(zipFile);
        auto name = m_localPluginModel.getQStringField("name");

        if (m_workspacePluginModel.isPluginExists(name))
            updateWorkspacePlugin(name);
        else
            publishPluginToWorkspace();
    }
    catch(std::exception& e)
    {
        qCCritical(logHub).noquote() << QString::fromStdString(e.what());
        clearContext(&m_localPluginModel, true);
    }
}

void CHubManager::publishPluginToWorkspace()
{
    assert(m_pNetworkMgr);

    //Http request to create new plugin
    QByteArray jsonPayload = createPluginPayload(&m_localPluginModel);
    CUserNamespace ns = m_currentUser.getNamespace(m_workspacePluginModel.getCurrentWorkspace());

    try
    {
        CHttpRequest request(ns.m_url + "algos/", "application/json", m_currentUser);
        auto pReply = m_pNetworkMgr->post(request, jsonPayload);
        connect(pReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pReply, &m_workspacePluginModel, HubRequestType::PUBLISH_WORKSPACE);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(&m_localPluginModel, true);
        return;
    }
}

void CHubManager::uploadPluginPackage()
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    //Http request to send plugin file
    try
    {
        CHttpRequest request(m_workspacePluginModel.getCurrentRequestUrl() + "packages/", m_currentUser);

        // Build multi-part request
        QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        // Package payload part
        QJsonObject payload = createPluginPackagePayload(&m_localPluginModel);
        foreach(const QString& key, payload.keys())
        {
            QHttpPart fieldPart;
            QString formData = QString("form-data; name=\"%1\"").arg(key);
            fieldPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));

            auto value = payload.value(key);
            if (value.isArray())
            {
                QJsonDocument jsonDoc(value.toArray());
                fieldPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
                fieldPart.setBody(jsonDoc.toJson());
            }
            else
            {
                fieldPart.setBody(value.toString().toUtf8());
            }
            pMultiPart->append(fieldPart);
        }

        // Plugin package zip part
        QString packageFile = m_workspacePluginModel.getPackageFile();
        auto pTranferFile = new QFile(packageFile);
        m_transferFiles.push_back(pTranferFile);
        pTranferFile->open(QIODevice::ReadOnly);
        QHttpPart filePart = request.createFilePart(pTranferFile, "file", "application/zip");
        pMultiPart->append(filePart);

        auto pNewReply = m_pNetworkMgr->post(request, pMultiPart);
        pMultiPart->setParent(pNewReply);

        //Init progress
        initTransferProgress(pNewReply, tr("Uploading algorithm package"), pTranferFile->size() / 1024);

        connect(pNewReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pNewReply, &m_workspacePluginModel, HubRequestType::UPLOAD_PACKAGE);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(&m_localPluginModel, true);
        return;
    }
}

void CHubManager::uploadPluginIcon(QNetworkReply* pReply)
{    
    QJsonObject jsonResponse = getJsonObject(pReply, tr("Error in algorithm creation response from Ikomia HUB"));
    if (jsonResponse.isEmpty())
    {
        clearContext(&m_localPluginModel, true);
        return;
    }

    // Get algo URL
    QString strUrl = jsonResponse["url"].toString();
    uploadPluginIcon(strUrl);
}

void CHubManager::uploadPluginIcon(const QString &strUrl)
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    m_workspacePluginModel.setCurrentRequestUrl(strUrl);

    try
    {
        //Http request to send plugin file
        CHttpRequest request(strUrl, m_currentUser);

        //Get full path to icon file
        QString iconPath = m_localPluginModel.getQStringField("iconPath");
        if(iconPath.isEmpty())
        {
            iconPath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/" + "icon.png");
            QPixmap pixmap = QPixmap(":/Images/default-process.png");
            pixmap.save(iconPath, "PNG");
        }

        QFile* pIconFile = new QFile(iconPath);
        pIconFile->open(QIODevice::ReadOnly);

        QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart = request.createFilePart(pIconFile, "icon", "image/png");
        pMultiPart->append(filePart);
        pIconFile->setParent(pMultiPart);

        auto pNewReply = m_pNetworkMgr->sendCustomRequest(request, "PATCH", pMultiPart);
        pMultiPart->setParent(pNewReply);

        //Init progress
        initTransferProgress(pNewReply, tr("Uploading icon"), pIconFile->size() / 1024);

        connect(pNewReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pNewReply, &m_workspacePluginModel, HubRequestType::UPLOAD_ICON);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(&m_localPluginModel, true);
        return;
    }
}

void CHubManager::uploadDemoWorkflow()
{
    try
    {
        QString workflowFile = createDemoWorkflow();
        m_transferFiles.push_back(new QFile(workflowFile));
        m_pProgressMgr->launchInfiniteProgress(tr("Workflow package compression..."), false);
        QString zipPath = m_pWorkflowMgr->getScaleManager()->createPackage(workflowFile);
        m_pProgressMgr->endInfiniteProgress();

        CHttpRequest request(m_workspacePluginModel.getCurrentRequestUrl() + "demo-workflows/", m_currentUser);
        // Build multi-part request
        QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        // Plugin package zip part
        auto pTransferFile = new QFile(zipPath);
        m_transferFiles.push_back(pTransferFile);
        pTransferFile->open(QIODevice::ReadOnly);
        QHttpPart filePart = request.createFilePart(pTransferFile, "archive", "application/zip");
        pMultiPart->append(filePart);
        auto pNewReply = m_pNetworkMgr->post(request, pMultiPart);
        pMultiPart->setParent(pNewReply);

        //Init progress
        initTransferProgress(pNewReply, tr("Uploading workflow package"), pTransferFile->size() / 1024);

        connect(pNewReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pNewReply, &m_workspacePluginModel, HubRequestType::UPLOAD_WORKFLOW);
        });
    }
    catch (boost::python::error_already_set&)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
        clearContext(&m_localPluginModel, true);
        return;
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(&m_localPluginModel, true);
        return;
    }
}

void CHubManager::downloadPluginPackage(CPluginModel* pModel, QNetworkReply* pReply)
{
    assert(m_pNetworkMgr);

    QJsonObject jsonPlugin = getJsonObject(pReply, tr("Error while retrieving algorithm details"));
    if (jsonPlugin.isEmpty())
    {
        clearContext(pModel, true);
        return;
    }

    QJsonArray jsonPackages = jsonPlugin["packages"].toArray();
    QString packageUrl = findBestPackageUrl(jsonPackages);

    try
    {
        //Http request to get plugin packages
        CHttpRequest request(packageUrl + "download/", "application/json");
        if (pModel->getType() == CPluginModel::Type::WORKSPACE)
            request.setUserAuth(m_currentUser);

        auto pNewReply = m_pNetworkMgr->get(request);

        //Init progress
        initTransferProgress(pNewReply, tr("Downloading algorithm package"), 0);

        connect(pNewReply, &QNetworkReply::finished, [=](){
            this->onReplyReceived(pNewReply, pModel, HubRequestType::DOWNLOAD_PACKAGE);
        });
    }
    catch(CException& e)
    {
        qCDebug(logHub) << e.what();
        clearContext(pModel, true);
        return;
    }
}

void CHubManager::savePluginFolder(CPluginModel* pModel, QNetworkReply* pReply)
{
    assert(m_pProcessMgr);

    //Save archive to disk
    QString packageFile = pModel->getPackageFile();
    QByteArray data = pReply->readAll();
    QString downloadPath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/") + packageFile;
    QFile file(downloadPath);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    pReply->deleteLater();

    //Prepare plugin installation
    QString destDir;
    std::string dirName = Utils::File::getFileNameWithoutExtension(packageFile.toStdString());
    int language = pModel->getIntegerField("language");

    if(language == ApiLanguage::CPP)
        destDir = QString::fromStdString(Utils::Plugin::getCppPath() + "/" + dirName);
    else
        destDir = QString::fromStdString(Utils::Plugin::getPythonPath() + "/" + dirName);

    boost::filesystem::path pluginFolder(destDir.toStdString());
    if(boost::filesystem::exists(pluginFolder))
    {
        //Plugin already exists, it's an update
        if(language == ApiLanguage::CPP)
        {
            //Save plugin archive name to settings to finalize installation on next start
            QSettings ikomiaSettings;
            const QString settingKey = "pending_plugin_update";
            QString pendingUpdates = ikomiaSettings.value(settingKey).toString();

            //Ensure unicity
            std::vector<std::string> pluginNames;
            Utils::String::tokenize(pendingUpdates.toStdString(), pluginNames, ";");
            auto res = std::find(pluginNames.begin(), pluginNames.end(), packageFile.toStdString());

            if(res == pluginNames.end())
            {
                pendingUpdates += packageFile + ";";
                ikomiaSettings.setValue(settingKey, pendingUpdates);
            }

            clearContext(pModel, false);

            //Need to restart to make the copy
            auto buttons = QMessageBox::question(nullptr, tr("Restart required"),
                                                 tr("Ikomia needs restarting to finalize algorithm installation. Do you want to restart now?"),
                                                 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            if(buttons == QMessageBox::Yes)
            {
                qCInfo(logHub).noquote() << tr("Restarting Ikomia...");
                emit doRestartIkomia();
            }
            return;
        }
        else
        {
            //Remove current plugin folder
            try
            {
                boost::filesystem::remove_all(pluginFolder);
            }
            catch (const boost::filesystem::filesystem_error& e)
            {
                qCCritical(logHub).noquote() << QString::fromStdString(e.code().message());
                clearContext(pModel, true);
                return;
            }
        }
    }

    //Asynchronous call -> Extract archive into separate thread
    extractZipFile(pModel, downloadPath, destDir);
}

void CHubManager::installPythonPluginDependencies(CPluginModel* pModel, const QString &directory, const CTaskInfo& info, const CUser& user)
{
    m_pProgressMgr->launchInfiniteProgress(tr("Installation of algorithm dependencies..."), false);

    QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>(this);
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, pModel, directory, info, user]
    {
        m_pProgressMgr->endInfiniteProgress();
        checkInstalledModules(directory);
        finalizePluginInstall(pModel, info, user);
    });

    //Install dependencies into separate thread
    auto future = QtConcurrent::run([directory, info]
    {
        if(info.m_language == ApiLanguage::CPP)
            return;

        //Requirements files
        Utils::Plugin::installRequirements(info.m_name);
    });
    pWatcher->setFuture(future);
}

void CHubManager::deleteTransferFiles()
{
    for (size_t i=0; i<m_transferFiles.size(); ++i)
    {
        m_transferFiles[i]->remove();
        delete m_transferFiles[i];
    }
    m_transferFiles.clear();
}

void CHubManager::clearContext(CPluginModel* pModel, bool bError)
{
    pModel->clearContext();
    m_pProgressMgr->endInfiniteProgress();
    m_bBusy = false;

    if (bError)
        emit doNotifyModelError(pModel);
}

void CHubManager::finalyzePublishHub()
{
    auto name = m_workspacePluginModel.getQStringField("name");
    qCInfo(logHub).noquote() << tr("Algorithm %1 was successfully published to Ikomia HUB").arg(name);
    onRequestHubModel();
    clearContext(&m_workspacePluginModel, false);
}

void CHubManager::finalizePublishWorkspace()
{
    updateLocalPlugin();
    deleteTransferFiles();
    auto name =  m_localPluginModel.getQStringField("name");
    qCInfo(logHub).noquote() << tr("Algorithm %1 was successfully published to your workspace").arg(name);
    clearContext(&m_localPluginModel, false);
    onRequestLocalModel();
    onRequestWorkspaceModel();
}

void CHubManager::finalizePluginInstall(CPluginModel* pModel, const CTaskInfo& info, const CUser& user)
{
    //Insert or update plugin to file database
    try
    {
        m_dbMgr.insertPlugin(info, user);
    }
    catch(CException& e)
    {
        qCDebug(logHub) << QString::fromStdString(e.what());
    }

    //Reload process library
    auto name = QString::fromStdString(info.m_name);
    m_pProgressMgr->launchInfiniteProgress(tr("Reloading algorithm %1...").arg(name), false);

    bool bLoaded = m_pProcessMgr->reloadPlugin(name, info.m_language);
    if(bLoaded)
    {
        onRequestLocalModel();
        qCInfo(logHub).noquote() << tr("Algorithm %1 was successfully installed").arg(name);
    }
    else
        qCWarning(logHub).noquote() << tr("Algorithm %1 was successfully installed but failed to load. Try to restart Ikomia Studio.").arg(name);

    //Clean
    clearContext(pModel, false);
}

void CHubManager::sendNextPublishInfo(CPluginModel* pModel, QNetworkReply *pReply)
{
    QJsonObject jsonInfo = getJsonObject(pReply, tr("Error while retrieving algorithm information for publication"));
    if (jsonInfo.isEmpty())
    {
        emit doSetNextPublishInfo(pModel->getCurrentIndex(), QJsonObject());
        return;
    }
    emit doSetNextPublishInfo(pModel->getCurrentIndex(), jsonInfo);
}

QString CHubManager::findBestPackageUrl(const QJsonArray &packages)
{
    //At this point, plugins are already filtered to ensure compatibility.
    //So for now, we just want to pick the latest version
    QString url;
    CSemanticVersion lastVersion("0.0.0");

    for (int i=0; i<packages.size(); ++i)
    {
        QJsonObject package = packages[i].toObject();
        if (package.contains("version"))
        {
            // HUB plugin
            CSemanticVersion packageVersion(package["version"].toString().toStdString());
            if (packageVersion > lastVersion)
            {
                url = package["url"].toString();
                lastVersion = packageVersion;
            }
        }
        else if (package.contains("tag"))
        {
            // Workspace plugin
            url = package["url"].toString();
        }
    }
    return url;
}

#include "moc_CHubManager.cpp"
