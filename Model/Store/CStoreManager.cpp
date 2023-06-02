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

#include "CStoreManager.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpPart>
#include <QtConcurrent>
#include "CStoreQueryModel.h"
#include "CStoreOnlineIconManager.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Core/CPluginTools.h"
#include "Model/Process/CProcessManager.h"
#include "Model/Plugin/CPluginManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "JlCompress.h"
#include "Core/CIkomiaRegistry.h"

//-----------------//
//- CStoreManager -//
//-----------------//
CStoreManager::CStoreManager():
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
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
    }
}

void CStoreManager::setManagers(QNetworkAccessManager *pNetworkMgr, CProcessManager *pProcessMgr, CPluginManager *pPluginMgr, CProgressBarManager *pProgressMgr)
{
    m_pNetworkMgr = pNetworkMgr;
    m_pProcessMgr = pProcessMgr;
    m_pPluginMgr = pPluginMgr;
    m_pProgressMgr = pProgressMgr;
    checkPendingUpdates();
}

void CStoreManager::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
    m_hubPluginModel.setCurrentUser(user);
    m_workspacePluginModel.setCurrentUser(user);
}

void CStoreManager::onRequestHubModel()
{
    m_hubPluginModel.clear();
    m_dbMgr.clearServerRecords(m_hubPluginModel.getType());
    createHubPluginModel();
}

void CStoreManager::onRequestWorkspaceModel()
{
    m_workspacePluginModel.clear();
    m_dbMgr.clearServerRecords(m_workspacePluginModel.getType());
    createWorkspacePluginModel();
}

void CStoreManager::onRequestLocalModel()
{
    m_localPluginModel.clear();
    createQueryModel(&m_localPluginModel);
}

void CStoreManager::onRequestNextPublishInfo(const QModelIndex &index)
{
    m_workspacePluginModel.setCurrentIndex(index);
    auto name = m_workspacePluginModel.getQStringField("name", index);
    QJsonObject pluginInfo = m_workspacePluginModel.getJsonPlugin(name);
    QUrlQuery urlQuery(pluginInfo["url"].toString() + "publish/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCWarning(logStore) << url.errorString();
        emit doSetNextPublishInfo(index, QJsonObject());
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=]{
       this->onReplyReceived(pReply, &m_workspacePluginModel, StoreRequestType::GET_NEXT_PUBLISH_INFO);
    });
}

void CStoreManager::onPublishHub(const QModelIndex &index, const QJsonObject& info)
{
    assert(index.isValid());

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("A plugin is already downloaded or uploaded. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to publish plugins
    if(m_currentUser.isConnected() == false)
    {
        qCCritical(logStore).noquote() << tr("You have to login before publishing plugins");
        return;
    }

    publishToHub(index, info);
}

void CStoreManager::onPublishWorkspace(const QModelIndex &index, const QString& workspace)
{
    assert(index.isValid());

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("A plugin is already downloaded or uploaded. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to publish plugins
    if(m_currentUser.isConnected() == false)
    {
        qCCritical(logStore).noquote() << tr("You have to login before publishing plugins");
        return;
    }

    publishToWorkspace(index, workspace);
}

void CStoreManager::onInstallPlugin(CPluginModel::Type type, const QModelIndex &index)
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

void CStoreManager::onUpdatePluginInfo(bool bFullEdit, const CTaskInfo &info)
{
    //Slot called if a user edit documentation and save modifications -> only available on local plugins
    assert(m_pProcessMgr);
    m_pProcessMgr->onUpdateProcessInfo(bFullEdit, info);
    createQueryModel(&m_localPluginModel);
}

void CStoreManager::onServerSearchChanged(const QString &text)
{
    /*QString query;
    if(text.isEmpty())
        query = m_dbMgr.getAllPluginsQuery(CPluginModel::Type::HUB);
    else
        query = m_dbMgr.getServerSearchQuery(text);

    m_hubPluginModel.setQuery(query, m_dbMgr.getPluginsDatabase(CPluginModel::Type::HUB));*/
}

void CStoreManager::onLocalSearchChanged(const QString &text)
{
    /*QString query;
    if(text.isEmpty())
        query = m_dbMgr.getAllLocalPluginsQuery();
    else
        query = m_dbMgr.getLocalSearchQuery(text);

    m_localPluginModel.setQuery(query, m_dbMgr.getServerPluginsDatabase());*/
}

void CStoreManager::onReplyReceived(QNetworkReply *pReply, CPluginModel* pModel, StoreRequestType requestType)
{
    if (pReply == nullptr)
    {
        clearContext();
        qCCritical(logStore).noquote() << "Invalid reply from Ikomia Scale";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        clearContext();
        qCCritical(logStore).noquote() << pReply->errorString();
        QString content(pReply->readAll());
        Utils::print(content.toStdString(), QtDebugMsg);
        pReply->deleteLater();
        return;
    }

    switch(requestType)
    {
        case StoreRequestType::GET_PLUGINS:
            fillServerPluginModel(pModel, pReply);
            break;
        case StoreRequestType::GET_PLUGIN_DETAILS:
            addPluginToModel(pModel, pReply);
            break;
        case StoreRequestType::GET_NEXT_PUBLISH_INFO:
            sendNextPublishInfo(pModel, pReply);
            break;
        case StoreRequestType::GET_PACKAGE_URL:
            downloadPluginPackage(pModel, pReply);
            break;
        case StoreRequestType::PUBLISH_WORKSPACE:
            uploadPluginIcon(pReply);
            break;
        case StoreRequestType::PUBLISH_HUB:
            finalyzePublishHub();
            break;
        case StoreRequestType::UPLOAD_ICON:
            uploadPluginPackage();
            break;
        case StoreRequestType::UPLOAD_PACKAGE:
            finalizePublishWorkspace();
            break;
        case StoreRequestType::DOWNLOAD_PACKAGE:
            savePluginFolder(pModel, pReply);
            break;
    }

    m_pProgressMgr->endInfiniteProgress();
    pReply->deleteLater();
}

void CStoreManager::onPluginCompressionDone(const QString& zipFile)
{
    m_pProgressMgr->endInfiniteProgress();

    try
    {
        if(zipFile.isEmpty())
        {
            qCCritical(logStore).noquote() << tr("Plugin compression failed, transfer to server aborted");
            clearContext();
            return;
        }

        m_workspacePluginModel.setPackageFile(zipFile);
        auto name = m_localPluginModel.getQStringField("name");

        if (m_workspacePluginModel.isPluginExists(name))
            updateServerPlugin();
        else
            publishPluginToWorkspace();
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
        clearContext();
    }
}

void CStoreManager::onUpdatePluginDone()
{
    /*auto info = checkReply(StoreRequestType::UPDATE_PLUGIN);
    auto pReply = info.first;

    if(pReply == nullptr)
    {
        clearContext();
        return;
    }

    uploadPluginIcon();
    pReply->deleteLater();*/
}

void CStoreManager::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    const float factor = 1024.0*1024.0;
    QString sent = QString::number(bytesSent / factor, 'f', 1);
    QString total = QString::number(bytesTotal / factor, 'f', 1);
    emit m_progressSignal.doSetMessage(QString("Uploading plugin: %1 Mb / %2 Mb").arg(sent).arg(total));
    emit m_progressSignal.doSetValue(bytesSent / 1024);
}

void CStoreManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    assert(m_pProgressMgr);

    if(m_bDownloadStarted == false)
    {
        m_pProgressMgr->launchProgress(&m_progressSignal, bytesTotal/1024, tr("Downloading algorithm..."), false);
        m_bDownloadStarted = true;
    }

    const float factor = 1024.0*1024.0;
    QString received = QString::number(bytesReceived / factor, 'f', 1);
    QString total = QString::number(bytesTotal / factor, 'f', 1);
    emit m_progressSignal.doSetMessage(QString("Downloading algorithm: %1 Mb / %2 Mb").arg(received).arg(total));
    emit m_progressSignal.doSetValue(bytesReceived / 1024);
}

void CStoreManager::queryServerPlugins(CPluginModel* pModel, const QString& strUrl)
{
    assert(m_pNetworkMgr);
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    if (pModel->getType() == CPluginModel::Type::WORKSPACE)
    {
        QVariant cookieHeaders;
        cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
        request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    }

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, pModel, StoreRequestType::GET_PLUGINS);
    });
}

void CStoreManager::queryServerPluginDetails(CPluginModel* pModel, QString strUrl)
{
    assert(m_pNetworkMgr);
    QUrlQuery urlQuery(strUrl);

    QUrl url(urlQuery.query());
    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    if (pModel->getType() == CPluginModel::Type::WORKSPACE)
    {
        QVariant cookieHeaders;
        cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
        request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    }

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, pModel, StoreRequestType::GET_PLUGIN_DETAILS);
    });
}

void CStoreManager::queryServerInstallPlugin(CPluginModel* pModel, const QString& strUrl)
{
    assert(m_pNetworkMgr);
    if (m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("An algorithm is already downloaded or uploaded. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //Http request to get plugin packages
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    m_bBusy = true;
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    if (pModel->getType() == CPluginModel::Type::WORKSPACE)
    {
        QVariant cookieHeaders;
        cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
        request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    }

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, pModel, StoreRequestType::GET_PACKAGE_URL);
    });
}

void CStoreManager::createHubPluginModel()
{
    QString url = Utils::Network::getBaseUrl() + "/v1/hub/";
    m_hubPluginModel.setCurrentRequestUrl(url);
    queryServerPlugins(&m_hubPluginModel, url);
    m_pProgressMgr->launchInfiniteProgress(tr("Loading algorithms from Ikomia HUB..."), false);
}

void CStoreManager::createWorkspacePluginModel()
{
    QString url = Utils::Network::getBaseUrl() + "/v1/algos/";
    m_workspacePluginModel.setCurrentRequestUrl(url);
    queryServerPlugins(&m_workspacePluginModel, url);
    m_pProgressMgr->launchInfiniteProgress(tr("Loading algorithms from private workspaces..."), false);
}

void CStoreManager::createQueryModel(CPluginModel* pModel)
{
    auto type = pModel->getType();
    pModel->init(m_currentUser, m_dbMgr.getAllPluginsQuery(type), m_dbMgr.getPluginsDatabase(type));
    emit doSetPluginModel(pModel);
}

QByteArray CStoreManager::createPluginPayload(CPluginModel* pModel)
{
    QJsonObject plugin;

    //Plugin name - mandatory
    QString name = pModel->getQStringField("name");
    if(name.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: plugin name is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["name"] = name;

    //Short description - mandatory
    QString shortDescription = pModel->getQStringField("shortDescription");
    if(shortDescription.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: short description is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["short_description"] = shortDescription;

    //Description - mandatory
    QString description = pModel->getQStringField("description");
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
    // TODO paper["link"] = ;

    plugin["paper"] = paper;

    //Online documentation - optional
    /*QString docLink = m_localPluginModel.getQStringField("docLink");
    if(docLink.isEmpty() == false)
        plugin["docLink"] = docLink;*/

    // TODO License - optional
    /*QString license = m_localPluginModel.getQStringField("license");
    if(license.isEmpty() == false)
        plugin["license"] = license;*/

    //Repository - optional
    QString repository = pModel->getQStringField("repository");
    if(repository.isEmpty() == false)
        plugin["repository"] = repository;

    //Original Repository - optional
    QString originalRepository = pModel->getQStringField("originalRepository");
    if(originalRepository.isEmpty() == false)
        plugin["original_implementation_repository"] = originalRepository;

    //Date of creation
    /*QString createdDate = m_localPluginModel.getQStringField("createdDate");
    if(createdDate.isEmpty() == false)
        plugin["createdDate"] = createdDate;*/

    //Date of modification
    //plugin["modifiedDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    //Version - mandatory
    /*QString version = m_localPluginModel.getQStringField("version");
    if(version.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: version is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["version"] = version;*/

    //Program language
    auto language = static_cast<ApiLanguage>(pModel->getIntegerField("language"));
    plugin["language"] = QString::fromStdString(Utils::Plugin::getLanguageName(language));

    QJsonDocument doc(plugin);
    return doc.toJson();
}

QJsonObject CStoreManager::createPluginPackagePayload(CPluginModel *pModel)
{
    QJsonObject package;

    //Ikomia version
    QString ikomiaVersion = pModel->getQStringField("ikomiaVersion");
    if(ikomiaVersion.isEmpty())
        ikomiaVersion = Utils::IkomiaApp::getCurrentVersionNumber();

    package["ikomia_min_version"] = ikomiaVersion;

    //Python minimum version
    QString minPythonVersion = pModel->getQStringField("minPythonVersion");
    package["python_min_version"] = minPythonVersion;

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
    return package;
}

QString CStoreManager::checkPythonPluginDirectory(const QString &directory)
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

QString CStoreManager::checkCppPluginDirectory(const QString &directory, const QString &name)
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

void CStoreManager::checkPendingUpdates()
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
            qCCritical(logStore).noquote() << QString::fromStdString(e.code().message());
            continue;
        }

        //Extract new plugin content
        auto extractedFiles = JlCompress::extractDir(zipPath, QString::fromStdString(destDir));
        if(extractedFiles.size() == 0)
            qCCritical(logStore).noquote() << tr("Archive extraction failed: installation of %1 aborted").arg(QString::fromStdString(dirName));

        //Delete archive file
        zipFile.remove();
    }

    ikomiaSettings.setValue(settingKey, "");
}

void CStoreManager::checkInstalledModules(const QString &pluginDir)
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

void CStoreManager::queryServerUpdatePlugin(const QString& strUrl, StoreRequestType requestType)
{
    //Http request to update plugin
    /*QByteArray jsonString = createPluginJson();
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
        throw CException(HttpExCode::INVALID_URL, url.errorString().toStdString(), __func__, __FILE__, __LINE__);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->put(request, jsonString);
    m_requests.push(requestType, serverType, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onUpdatePluginDone);*/
}

void CStoreManager::updateServerPlugin()
{
    /*QString url = Utils::Network::getBaseUrl() + QString("/algos/plugin/%1/").arg(m_hubPluginModel.getCurrentPluginId());
    queryServerUpdatePlugin(url, StoreRequestType::UPDATE_PLUGIN, HUB);*/
}

void CStoreManager::updateLocalPlugin()
{
    try
    {
        int pluginId = m_localPluginModel.getIntegerField("id");
        m_dbMgr.updateLocalPluginModifiedDate(pluginId);
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << e.what();
    }
}

void CStoreManager::fillServerPluginModel(CPluginModel* pModel, QNetworkReply* pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document while retrieving algorithm list");
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure while retrieving algorithm list");
        clearContext();
        return;
    }

    QJsonObject jsonPage = doc.object();
    int count = jsonPage["count"].toInt();
    pModel->setTotalPluginCount(count);

    if (jsonPage["next"].isNull() == false)
    {
        QString url = pModel->getCurrentRequestUrl() + QString("?page_size=%1").arg(count);
        queryServerPlugins(pModel, url);
    }
    else if (jsonPage["count"] == 0)
    {
        validateServerPluginModel(pModel);
        return;
    }
    else
    {
        QJsonArray plugins = jsonPage["results"].toArray();
        for (int i=0; i<plugins.size(); i++)
        {
            QJsonObject plugin = plugins[i].toObject();
            queryServerPluginDetails(pModel, plugin["url"].toString());
        }
    }
}

void CStoreManager::addPluginToModel(CPluginModel *pModel, QNetworkReply *pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document while retrieving algorihtm details");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure while retrieving algorithm details");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }
    pModel->addJsonPlugin(doc.object());

    if( pModel->isComplete())
    {
        pModel->filterCompatiblePlugins();

        auto pIconMgr = new CStoreOnlineIconManager(pModel, m_pNetworkMgr, m_currentUser);
        connect(pIconMgr, &CStoreOnlineIconManager::doIconsLoaded, [this, pIconMgr, pModel]
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
            qCCritical(logStore).noquote() << QString::fromStdString(e.what());
        }
    }
}

void CStoreManager::validateServerPluginModel(CPluginModel* pModel)
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
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
    }
}

void CStoreManager::validatePluginFolder(CPluginModel* pModel, const QStringList& files, const QString dstDir)
{
    if(files.isEmpty())
    {
        qCCritical(logStore).noquote() << tr("Archive extraction failed: installation aborted");
        clearContext();
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
        procInfo.m_ikomiaVersion = pModel->getStringField("ikomiaVersion");
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
        installPythonPluginDependencies(validDstDir, procInfo, pluginUser);
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote().noquote() << QString::fromStdString(e.what());
        //Clean
        clearContext();
    }
}

void CStoreManager::generateZipFile()
{
    assert(m_pProgressMgr);

    auto name = m_localPluginModel.getQStringField("name");
    auto language = m_localPluginModel.getIntegerField("language");
    QString zipFilePath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/" + Utils::String::httpFormat(name.toStdString()) + ".zip");

    QString pluginDir;
    if(language == ApiLanguage::CPP)
        pluginDir = Utils::CPluginTools::getCppPluginFolder(name);
    else if(language == ApiLanguage::PYTHON)
        pluginDir = Utils::CPluginTools::getPythonPluginFolder(name);

    if(pluginDir.isEmpty())
    {
        qCritical().noquote() << tr("Unable to find folder of plugin %1").arg(name);
        return;
    }

    m_pProgressMgr->launchInfiniteProgress(tr("Plugin compression..."), false);

    QFutureWatcher<bool>* pWatcher = new QFutureWatcher<bool>;
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, zipFilePath]
    {
        bool bCompress = pWatcher->result();
        QString zipFile = (bCompress == true ? zipFilePath : QString());
        onPluginCompressionDone(zipFile);
    });

    //Compress plugin into separate thread
    auto future = QtConcurrent::run([zipFilePath, pluginDir]
    {
        return JlCompress::compressDir(zipFilePath, pluginDir, true);
    });
    pWatcher->setFuture(future);
}

void CStoreManager::extractZipFile(CPluginModel* pModel, const QString& src, const QString& dstDir)
{
    m_pProgressMgr->launchInfiniteProgress(tr("Plugin extraction..."), false);

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

void CStoreManager::publishToHub(const QModelIndex& index, const QJsonObject& info)
{
    auto name = m_workspacePluginModel.getQStringField("name", index);
    QJsonObject pluginInfo = m_workspacePluginModel.getJsonPlugin(name);
    QUrlQuery urlQuery(pluginInfo["url"].toString() + "publish/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCWarning(logStore) << url.errorString();
        return;
    }

    m_bBusy = true;
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_currentUser.getSessionCookie("csrftoken"));

    QJsonDocument payload(info);
    auto pReply = m_pNetworkMgr->put(request, payload.toJson());

    connect(pReply, &QNetworkReply::finished, [=]{
       this->onReplyReceived(pReply, &m_hubPluginModel, StoreRequestType::PUBLISH_HUB);
    });
}

void CStoreManager::publishToWorkspace(const QModelIndex &index, const QString& workspace)
{
    m_bBusy = true;
    m_localPluginModel.setCurrentIndex(index);
    m_workspacePluginModel.setCurrentWorkspace(workspace);
    //Asynchronous call -> plugin compression is made into separate thread
    generateZipFile();
}

void CStoreManager::publishPluginToWorkspace()
{
    assert(m_pNetworkMgr);

    //Http request to create new plugin
    QByteArray jsonPayload = createPluginPayload(&m_localPluginModel);
    CUserNamespace ns = m_currentUser.getNamespace(m_workspacePluginModel.getCurrentWorkspace());
    QUrlQuery urlQuery(ns.m_url + "algos/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_currentUser.getSessionCookie("csrftoken"));

    auto pReply = m_pNetworkMgr->post(request, jsonPayload);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, &m_workspacePluginModel, StoreRequestType::PUBLISH_WORKSPACE);
    });
}

void CStoreManager::uploadPluginPackage()
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    //Http request to send plugin file
    QUrlQuery urlQuery(m_workspacePluginModel.getCurrentRequestUrl() + "packages/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);

    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_currentUser.getSessionCookie("csrftoken"));

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
    m_pTranferFile = new QFile(packageFile);
    m_pTranferFile->open(QIODevice::ReadOnly);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/zip"));
    QString formData = QString("form-data; name=\"file\"; filename=\"%1\"").arg(packageFile);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, m_pTranferFile->size());
    filePart.setBodyDevice(m_pTranferFile);
    pMultiPart->append(filePart);

    //Init progress: size in ko
    m_pProgressMgr->launchProgress(&m_progressSignal, m_pTranferFile->size() / 1024, tr("Uploading plugin..."), false);

    auto pNewReply = m_pNetworkMgr->post(request, pMultiPart);
    pMultiPart->setParent(pNewReply);

    connect(pNewReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pNewReply, &m_workspacePluginModel, StoreRequestType::UPLOAD_PACKAGE);
    });
    connect(pNewReply, &QNetworkReply::uploadProgress, this, &CStoreManager::onUploadProgress);
}

void CStoreManager::uploadPluginIcon(QNetworkReply* pReply)
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document in plugin creation response");
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure in plugin creation response");
        clearContext();
        return;
    }

    // Get algo URL
    QJsonObject jsonResponse = doc.object();
    QString strUrl = jsonResponse["url"].toString();
    m_workspacePluginModel.setCurrentRequestUrl(strUrl);

    //Http request to send plugin file
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

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

    QNetworkRequest request;
    request.setUrl(url);

    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_currentUser.getSessionCookie("csrftoken"));

    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    QString formData = QString("form-data; name=\"icon\"; filename=\"%1\"").arg(iconPath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, pIconFile->size());
    filePart.setBodyDevice(pIconFile);
    pMultiPart->append(filePart);
    pIconFile->setParent(pMultiPart);

    //Init progress: size in ko
    m_pProgressMgr->launchProgress(&m_progressSignal, pIconFile->size() / 1024, tr("Uploading icon..."), false);

    auto pNewReply = m_pNetworkMgr->sendCustomRequest(request, "PATCH", pMultiPart);
    pMultiPart->setParent(pNewReply);

    connect(pNewReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pNewReply, &m_workspacePluginModel, StoreRequestType::UPLOAD_ICON);
    });
    connect(pNewReply, &QNetworkReply::uploadProgress, this, &CStoreManager::onUploadProgress);
}

void CStoreManager::downloadPluginPackage(CPluginModel* pModel, QNetworkReply* pReply)
{
    assert(m_pNetworkMgr);

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document while retrieving algorihtm details");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure while retrieving algorithm details");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }

    QJsonObject jsonPlugin = doc.object();
    QJsonArray jsonPackages = jsonPlugin["packages"].toArray();
    QString packageUrl = findBestPackageUrl(jsonPackages);

    //Http request to get plugin packages
    QUrlQuery urlQuery(packageUrl + "download/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    if (pModel->getType() == CPluginModel::Type::WORKSPACE)
    {
        QVariant cookieHeaders;
        cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
        request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    }

    auto pNewReply = m_pNetworkMgr->get(request);
    connect(pNewReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pNewReply, pModel, StoreRequestType::DOWNLOAD_PACKAGE);
    });
    connect(pNewReply, &QNetworkReply::downloadProgress, this, &CStoreManager::onDownloadProgress);
}

void CStoreManager::savePluginFolder(CPluginModel* pModel, QNetworkReply* pReply)
{
    assert(m_pProcessMgr);

    m_bDownloadStarted = false;
    emit m_progressSignal.doFinish();

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

            clearContext();

            //Need to restart to make the copy
            auto buttons = QMessageBox::question(nullptr, tr("Restart required"),
                                                 tr("Ikomia needs restarting to finalize plugin installation. Do you want to restart now?"),
                                                 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            if(buttons == QMessageBox::Yes)
            {
                qCInfo(logStore).noquote() << tr("Restarting Ikomia...");
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
                qCCritical(logStore).noquote() << QString::fromStdString(e.code().message());
                clearContext();
                return;
            }
        }
    }

    //Asynchronous call -> Extract archive into separate thread
    extractZipFile(pModel, downloadPath, destDir);
}

void CStoreManager::installPythonPluginDependencies(const QString &directory, const CTaskInfo& info, const CUser& user)
{
    m_pProgressMgr->launchInfiniteProgress(tr("Plugin dependencies installation..."), false);

    QFutureWatcher<void>* pWatcher = new QFutureWatcher<void>(this);
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, directory, info, user]
    {
        m_pProgressMgr->endInfiniteProgress();
        checkInstalledModules(directory);
        finalizePluginInstall(info, user);
    });

    //Install dependencies into separate thread
    auto future = QtConcurrent::run([directory, info]
    {
        if(info.m_language == ApiLanguage::CPP)
            return;

        //Requirements files
        std::set<QString> requirements;
        QDir dir(directory);
        QRegularExpression re("[rR]equirements[0-9]*.txt");

        foreach (QString fileName, dir.entryList(QDir::Files|QDir::NoSymLinks))
        {
            if(fileName.contains(re))
                requirements.insert(fileName);
        }

        for(auto&& name : requirements)
        {
            QString requirementFile = directory + "/" + name;
            qCInfo(logStore()).noquote() << "Plugin dependencies installation from " + requirementFile;
            Utils::Python::installRequirements(requirementFile);
        }
    });
    pWatcher->setFuture(future);
}

void CStoreManager::deleteTranferFile()
{
    m_pTranferFile->remove();
    delete m_pTranferFile;
    m_pTranferFile = nullptr;
}

void CStoreManager::clearContext()
{
    m_hubPluginModel.clearContext();
    m_localPluginModel.clearContext();
    m_workspacePluginModel.clearContext();
    m_pProgressMgr->endInfiniteProgress();
    m_bBusy = false;
}

void CStoreManager::finalyzePublishHub()
{
    emit m_progressSignal.doFinish();
    onRequestHubModel();
    clearContext();
    qCInfo(logStore).noquote() << tr("Algorithm was successfully published to Ikomia HUB");
}

void CStoreManager::finalizePublishWorkspace()
{
    emit m_progressSignal.doFinish();
    updateLocalPlugin();
    onRequestLocalModel();
    onRequestWorkspaceModel();
    deleteTranferFile();
    clearContext();
    qCInfo(logStore).noquote() << tr("Algorithm was successfully published to your workspace");
}

void CStoreManager::finalizePluginInstall(const CTaskInfo& info, const CUser& user)
{
    //Insert or update plugin to file database
    m_dbMgr.insertPlugin(info, user);
    //Reload process library
    m_pProgressMgr->launchInfiniteProgress(tr("Reloading algorithm..."), false);

    bool bLoaded = m_pProcessMgr->reloadPlugin(QString::fromStdString(info.m_name), info.m_language);
    if(bLoaded)
    {
        createQueryModel(&m_localPluginModel);
        qCInfo(logStore).noquote() << tr("Algorithm was successfully installed");
    }
    else
        qCWarning(logStore).noquote() << tr("Algorithm was successfully installed but failed to load. Try to restart Ikomia Studio.");

    //Clean
    clearContext();
    m_pProgressMgr->endInfiniteProgress();
}

void CStoreManager::sendNextPublishInfo(CPluginModel* pModel, QNetworkReply *pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document while retrieving algorithm information for publication");
        emit doSetNextPublishInfo(pModel->getCurrentIndex(), QJsonObject());
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure while retrieving algorithm information for publication");
        emit doSetNextPublishInfo(pModel->getCurrentIndex(), QJsonObject());
        return;
    }
    emit doSetNextPublishInfo(pModel->getCurrentIndex(), doc.object());
}

QString CStoreManager::findBestPackageUrl(const QJsonArray &packages)
{
    //At this point, plugins are already filtered to ensure compatibility.
    //So for now, we just want to pick the latest version -> TODO: how to get latest version in workspace?
    QString url;
    QString lastTag;
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
            auto tag = package["tag"].toString();
            if (tag > lastTag)
            {
                url = package["url"].toString();
                lastTag = tag;
            }
        }
    }
    return url;
}

#include "moc_CStoreManager.cpp"
