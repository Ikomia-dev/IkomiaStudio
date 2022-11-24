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
#include "Model/Process/CProcessManager.h"
#include "Model/Plugin/CPluginManager.h"
#include "Model/ProgressBar/CProgressBarManager.h"
#include "JlCompress.h"
#include "Core/CIkomiaRegistry.h"

CStoreManager::CStoreManager()
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

    if(m_pServerPluginModel != nullptr)
        m_pServerPluginModel->setCurrentUser(user);

    if(m_pLocalPluginModel != nullptr)
        m_pLocalPluginModel->setCurrentUser(user);
}

void CStoreManager::onRequestServerModel()
{
    createServerPluginModel();
}

void CStoreManager::onRequestLocalModel()
{
    createLocalPluginModel();
}

void CStoreManager::onPublishPlugin(const QModelIndex &index)
{
    assert(index.isValid());

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("A plugin is already downloaded or uploaded. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to publish plugins
    if(m_currentUser.m_id == -1)
    {
        qCCritical(logStore).noquote() << tr("You have to login before publishing plugins");
        return;
    }

    //We can only publish plugins owned by current user (ie userId == -1 OR userId == m_currentUser.m_id)
    int authorId = m_pLocalPluginModel->record(index.row()).value("userId").toInt();
    if(authorId != -1 && authorId != m_currentUser.m_id)
    {
        qCCritical(logStore).noquote() << tr("This plugin is not yours, you can't publish it");
        return;
    }

    m_bBusy = true;
    m_currentLocalIndex = index;
    //Asynchronous call -> plugin compression is made into separate thread
    generateZipFile();
}

void CStoreManager::onInstallPlugin(const QModelIndex &index)
{
    assert(m_pNetworkMgr);

    if(m_bBusy == true)
    {
        QMessageBox::information(nullptr, tr("Information"), tr("A plugin is already downloaded or uploaded. Please wait until it is finished."), QMessageBox::Ok);
        return;
    }

    //User has to be logged in to install plugins
    if(m_currentUser.m_id == -1)
    {
        qCCritical(logStore).noquote() << tr("You have to login before installing plugins");
        return;
    }

    //Http request to get plugin package (zip) url
    m_currentPluginServerId = m_pServerPluginModel->record(index.row()).value("id").toInt();
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/package/").arg(m_currentPluginServerId));
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        m_currentPluginServerId = -1;
        return;
    }

    m_bBusy = true;
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());
    m_currentServerIndex = index;

    auto pReply = m_pNetworkMgr->get(request);
    m_mapTypeRequest.insert(GET_PACKAGE_URL, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onGetPackageUrlDone);
}

void CStoreManager::onUpdatePluginInfo(bool bFullEdit, const CTaskInfo &info)
{
    //Slot called if a user edit documentation and save modifications -> only available on local plugins
    assert(m_pProcessMgr);
    m_pProcessMgr->onUpdateProcessInfo(bFullEdit, info);
    createLocalPluginModel();
}

void CStoreManager::onServerSearchChanged(const QString &text)
{
    QString query;
    if(text.isEmpty())
        query = m_dbMgr.getAllServerPluginsQuery();
    else
        query = m_dbMgr.getServerSearchQuery(text);

    if (m_pServerPluginModel)
        m_pServerPluginModel->setQuery(query, m_dbMgr.getServerPluginsDatabase());
}

void CStoreManager::onLocalSearchChanged(const QString &text)
{
    QString query;
    if(text.isEmpty())
        query = m_dbMgr.getAllLocalPluginsQuery();
    else
        query = m_dbMgr.getLocalSearchQuery(text);

    if (m_pLocalPluginModel)
        m_pLocalPluginModel->setQuery(query, m_dbMgr.getLocalPluginsDatabase());
}

void CStoreManager::onGetPlugins()
{
    auto pReply = checkReply(GET_PLUGINS);
    if(pReply == nullptr)
    {
        clearContext();
        m_pProgressMgr->endInfiniteProgress();
        return;
    }
    fillServerPluginModel(pReply);
    pReply->deleteLater();
}

void CStoreManager::onPublishPluginDone()
{
    auto pReply = checkReply(PUBLISH_PLUGIN);
    if(pReply == nullptr)
    {
        clearContext();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document");
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure");
        clearContext();
        return;
    }
    QJsonObject jsonPlugin = doc.object();
    m_currentPluginServerId = jsonPlugin["id"].toInt();
    uploadPluginIcon();
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

        m_currentPluginPackageFile = zipFile;
        m_currentPluginServerId = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("serverId").toInt();

        if(m_currentPluginServerId == -1)
            publishPluginToServer();
        else
            updateServerPlugin();
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
        clearContext();
    }
}

void CStoreManager::onUpdatePluginDone()
{
    auto pReply = checkReply(UPDATE_PLUGIN);
    if(pReply == nullptr)
    {
        clearContext();
        return;
    }

    uploadPluginIcon();
    pReply->deleteLater();
}

void CStoreManager::onUploadPackageDone()
{
    auto pReply = checkReply(UPLOAD_PACKAGE);
    if(pReply == nullptr)
    {
        qCCritical(logStore).noquote().noquote() << tr("Plugin package was not published successfully. Check your connection and try again.");
        deletePlugin(); // Only works if connection is still active otherwise the plugin is not deleted on the server
        deleteTranferFile();
        return;
    }

    qCInfo(logStore).noquote() << tr("Plugin was successfully published on the server");
    emit m_progressSignal.doFinish();
    updateLocalPlugin();
    createLocalPluginModel();
    createServerPluginModel();
    pReply->deleteLater();
    deleteTranferFile();
    //Clear member data
    clearContext();
}

void CStoreManager::onUploadIconDone()
{
    //If error occurs when uploading icon, we do not stop the publication. It's not critical.
    auto pReply = checkReply(UPLOAD_ICON);
    if(pReply == nullptr)
        qCWarning(logStore).noquote() << tr("Plugin icon was not published successfully.");

    pReply->deleteLater();
    emit m_progressSignal.doFinish();
    uploadPluginPackage();
}

void CStoreManager::onGetPackageUrlDone()
{
    auto pReply = checkReply(GET_PACKAGE_URL);
    if(pReply == nullptr)
    {
        clearContext();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document");
        clearContext();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure");
        clearContext();
        return;
    }
    QJsonObject jsonPackage = doc.object();
    QString url = jsonPackage["packageFile"].toString();

    if(url.isEmpty())
    {
        qCCritical(logStore).noquote() << tr("Invalid package file name");
        clearContext();
        return;
    }

    downloadPluginPackage(url);
    pReply->deleteLater();
}

void CStoreManager::onDownloadPackageDone()
{
    assert(m_pProcessMgr);
    assert(m_currentServerIndex.isValid());
    m_bDownloadStarted = false;

    auto pReply = checkReply(DOWNLOAD_PACKAGE);
    if(pReply == nullptr)
    {
        clearContext();
        return;
    }

    emit m_progressSignal.doFinish();

    //Save archive to disk
    QByteArray data = pReply->readAll();
    QString downloadPath = QString::fromStdString(Utils::CPluginTools::getTransferPath() + "/") + m_currentPluginPackageFile;
    QFile file(downloadPath);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
    pReply->deleteLater();

    //Prepare plugin installation
    QString destDir;
    std::string dirName = Utils::File::getFileNameWithoutExtension(m_currentPluginPackageFile.toStdString());
    int language = m_pServerPluginModel->record(m_currentServerIndex.row()).value("language").toInt();

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
            auto res = std::find(pluginNames.begin(), pluginNames.end(), m_currentPluginPackageFile.toStdString());

            if(res == pluginNames.end())
            {
                pendingUpdates += m_currentPluginPackageFile + ";";
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
    extractZipFile(downloadPath, destDir);
}

void CStoreManager::onPluginExtractionDone(const QStringList& files, const QString dstDir)
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
        int row = m_currentServerIndex.row();
        pluginUser.m_id = m_pServerPluginModel->record(row).value("userId").toInt();
        pluginUser.m_firstName = m_pServerPluginModel->record(row).value("user").toString();
        CTaskInfo procInfo;
        procInfo.m_name = m_pServerPluginModel->record(row).value("name").toString().toStdString();
        procInfo.m_shortDescription = m_pServerPluginModel->record(row).value("shortDescription").toString().toStdString();
        procInfo.m_description = m_pServerPluginModel->record(row).value("description").toString().toStdString();
        procInfo.m_keywords = m_pServerPluginModel->record(row).value("keywords").toString().toStdString();
        procInfo.m_authors = m_pServerPluginModel->record(row).value("authors").toString().toStdString();
        procInfo.m_article = m_pServerPluginModel->record(row).value("article").toString().toStdString();
        procInfo.m_journal = m_pServerPluginModel->record(row).value("journal").toString().toStdString();
        procInfo.m_year = m_pServerPluginModel->record(row).value("year").toInt();
        procInfo.m_docLink = m_pServerPluginModel->record(row).value("docLink").toString().toStdString();
        procInfo.m_createdDate = m_pServerPluginModel->record(m_currentServerIndex.row()).value("createdDate").toString().toStdString();
        procInfo.m_modifiedDate = m_pServerPluginModel->record(m_currentServerIndex.row()).value("modifiedDate").toString().toStdString();
        procInfo.m_version = m_pServerPluginModel->record(row).value("version").toString().toStdString();
        procInfo.m_ikomiaVersion = m_pServerPluginModel->record(row).value("ikomiaVersion").toString().toStdString();
        procInfo.m_license = m_pServerPluginModel->record(row).value("license").toString().toStdString();
        procInfo.m_repo = m_pServerPluginModel->record(row).value("repository").toString().toStdString();
        procInfo.m_language = m_pServerPluginModel->record(row).value("language").toInt() == 0 ? ApiLanguage::CPP : ApiLanguage::PYTHON;
        procInfo.m_os = m_pServerPluginModel->record(row).value("os").toInt();

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
        m_pProgressMgr->launchProgress(&m_progressSignal, bytesTotal/1024, tr("Downloading plugin..."), false);
        m_bDownloadStarted = true;
    }

    const float factor = 1024.0*1024.0;
    QString received = QString::number(bytesReceived / factor, 'f', 1);
    QString total = QString::number(bytesTotal / factor, 'f', 1);
    emit m_progressSignal.doSetMessage(QString("Downloading plugin: %1 Mb / %2 Mb").arg(received).arg(total));
    emit m_progressSignal.doSetValue(bytesReceived / 1024);
}

void CStoreManager::onDeletePlugin()
{
    auto pReply = checkReply(DELETE_PLUGIN);
    try
    {
        QString pluginName = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("name").toString();

        if(pReply)
        {
            // Delete succeeds and Current plugin does not exist on server anymore
            // Remove from server database
            m_dbMgr.removeRemotePlugin(pluginName);
            // Remove from local database
            m_dbMgr.removeLocalPlugin(pluginName);

            pReply->deleteLater();

            // Update server QSqlQueryModel
            m_pServerPluginModel->setQuery(m_dbMgr.getAllServerPluginsQuery(), m_dbMgr.getServerPluginsDatabase());
        }
        else
        {
            // Plugin still exists on server side, must display it in storeDlg
            updateLocalPlugin();
            createServerPluginModel();
        }

        clearContext();
        // Update local QSqlQueryModel
        m_pLocalPluginModel->setQuery(m_dbMgr.getAllLocalPluginsQuery(), m_dbMgr.getLocalPluginsDatabase());
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << e.what();
    }
}

void CStoreManager::createServerPluginModel()
{
    assert(m_pNetworkMgr);

    if(m_pServerPluginModel != nullptr)
    {
        delete m_pServerPluginModel;
        m_pServerPluginModel = nullptr;
    }

    if(m_currentUser.m_token.isEmpty())
    {
        emit doSetServerPluginModel(nullptr);
        return;
    }

    m_dbMgr.clearServerRecords();
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/api/plugin/");

    QUrl url(urlQuery.query());
    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    m_pProgressMgr->launchInfiniteProgress(tr("Loading plugins..."), false);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->get(request);
    m_mapTypeRequest.insert(GET_PLUGINS, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onGetPlugins);
}

void CStoreManager::createServerQueryModel()
{
    m_pServerPluginModel = new CStoreQueryModel;
    m_pServerPluginModel->setCurrentUser(m_currentUser);
    m_pServerPluginModel->setQuery(m_dbMgr.getAllServerPluginsQuery(), m_dbMgr.getServerPluginsDatabase());
    emit doSetServerPluginModel(m_pServerPluginModel);
}

void CStoreManager::createLocalPluginModel()
{
    if(m_pLocalPluginModel != nullptr)
    {
        delete m_pLocalPluginModel;
        m_pLocalPluginModel = nullptr;
    }

    if(!m_currentUser.isConnected())
    {
        emit doSetLocalPluginModel(nullptr);
        return;
    }

    m_pLocalPluginModel = new CStoreQueryModel;
    m_pLocalPluginModel->setCurrentUser(m_currentUser);
    m_pLocalPluginModel->setQuery(m_dbMgr.getAllLocalPluginsQuery(), m_dbMgr.getLocalPluginsDatabase());
    emit doSetLocalPluginModel(m_pLocalPluginModel);
}

QByteArray CStoreManager::createPluginJson()
{
    QJsonObject plugin;
    int row = m_currentLocalIndex.row();

    //Plugin name - mandatory
    QString name = m_pLocalPluginModel->record(row).value("name").toString();
    if(name.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: plugin name is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["name"] = name;

    //Short description - mandatory
    QString shortDescription = m_pLocalPluginModel->record(row).value("shortDescription").toString();
    if(shortDescription.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: short description is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["short_description"] = shortDescription;

    //Description - mandatory
    QString description = m_pLocalPluginModel->record(row).value("description").toString();
    if(description.isEmpty())
        description = shortDescription;

    if(description.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: description is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["description"] = description;

    //Keywords - optional
    QString keywords = m_pLocalPluginModel->record(row).value("keywords").toString();
    if(keywords.isEmpty() == false)
        plugin["keywords"] = keywords;

    //Authors - mandatory
    QString authors = m_pLocalPluginModel->record(row).value("authors").toString();
    if(authors.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: author is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["authors"] = authors;

    //Article - optional
    QString article = m_pLocalPluginModel->record(row).value("article").toString();
    if(article.isEmpty() == false)
        plugin["article"] = article;

    //Journal - optional
    QString journal = m_pLocalPluginModel->record(row).value("journal").toString();
    if(journal.isEmpty() == false)
        plugin["journal"] = journal;

    //Year - optional
    plugin["year"] = m_pLocalPluginModel->record(row).value("year").toInt();

    //Online documentation - optional
    QString docLink = m_pLocalPluginModel->record(row).value("docLink").toString();
    if(docLink.isEmpty() == false)
        plugin["docLink"] = docLink;

    //License - optional
    QString license = m_pLocalPluginModel->record(row).value("license").toString();
    if(license.isEmpty() == false)
        plugin["license"] = license;

    //Repository - optional
    QString repository = m_pLocalPluginModel->record(row).value("repository").toString();
    if(repository.isEmpty() == false)
        plugin["repository"] = repository;

    //Date of creation
    QString createdDate = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("createdDate").toString();
    if(createdDate.isEmpty() == false)
        plugin["createdDate"] = createdDate;

    //Date of modification
    plugin["modifiedDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    //Version - mandatory
    QString version = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("version").toString();
    if(version.isEmpty())
        throw CException(CoreExCode::INVALID_USAGE, tr("Publication failed: version is mandatory").toStdString(), __func__, __FILE__, __LINE__);

    plugin["version"] = version;

    //Ikomia version - App & API
    QString ikomiaVersion = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("ikomiaVersion").toString();
    if(ikomiaVersion.isEmpty())
        ikomiaVersion = Utils::IkomiaApp::getCurrentVersionNumber();

    plugin["ikomiaVersion"] = ikomiaVersion;

    //Program language
    plugin["language"] = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("language").toInt();

    //Operating System - mandatory
    plugin["os"] = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("os").toInt();

    QJsonDocument doc(plugin);
    return doc.toJson();
}

QNetworkReply *CStoreManager::checkReply(int type) const
{
    auto it = m_mapTypeRequest.find(type);
    if(it == m_mapTypeRequest.end())
        return nullptr;

    QNetworkReply* pReply = it.value();
    if(pReply->error() != QNetworkReply::NoError)
    {
        qCCritical(logStore).noquote() << pReply->errorString();
        pReply->deleteLater();
        return nullptr;
    }
    return pReply;
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

void CStoreManager::updateServerPlugin()
{
    //Http request to update plugin
    QByteArray jsonString = createPluginJson();
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/").arg(m_currentPluginServerId));
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
        throw CException(HttpExCode::INVALID_URL, url.errorString().toStdString(), __func__, __FILE__, __LINE__);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->put(request, jsonString);
    m_mapTypeRequest.insert(UPDATE_PLUGIN, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onUpdatePluginDone);
}

void CStoreManager::updateLocalPlugin()
{
    try
    {
        int pluginId = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("id").toInt();
        QString pluginName = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("name").toString();
        int oldServerId = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("serverId").toInt();

        if(oldServerId == -1)
            m_dbMgr.setLocalPluginServerInfo(pluginId, pluginName, m_currentPluginServerId, m_currentUser);

        m_dbMgr.updateLocalPluginModifiedDate(pluginId);
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << e.what();
    }
}

void CStoreManager::fillServerPluginModel(QNetworkReply *pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }

    if(doc.isArray() == false)
    {
        qCCritical(logStore).noquote() << tr("Invalid JSON document structure");
        m_pProgressMgr->endInfiniteProgress();
        clearContext();
        return;
    }

    m_jsonServerPlugins = doc.array();

    auto pIconMgr = new CStoreOnlineIconManager(m_pNetworkMgr, m_currentUser);
    connect(pIconMgr, &CStoreOnlineIconManager::doIconsLoaded, [this, pIconMgr]
    {
        pIconMgr->deleteLater();
        validateServerPluginModel();
    });

    try
    {
        pIconMgr->loadIcons(&m_jsonServerPlugins);
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
    }
}

void CStoreManager::validateServerPluginModel()
{
    m_pProgressMgr->endInfiniteProgress();
    try
    {
        //Add plugins to local db
        m_dbMgr.insertPlugins(m_jsonServerPlugins);
        //Mise à jour des infos des plugins locaux (synchronisation)
        m_dbMgr.updateMemoryLocalPluginsInfo();
        //Réinitialisation des données du modèle pour les plugins locaux
        if(m_pLocalPluginModel)
            m_pLocalPluginModel->setQuery(m_dbMgr.getAllLocalPluginsQuery(), m_dbMgr.getLocalPluginsDatabase());

        //Création du model pour les plugins du serveur
        createServerQueryModel();
    }
    catch(std::exception& e)
    {
        qCCritical(logStore).noquote() << QString::fromStdString(e.what());
    }
}

void CStoreManager::generateZipFile()
{
    assert(m_pProgressMgr);

    auto name = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("name").toString();
    auto language = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("language").toInt();
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

void CStoreManager::extractZipFile(const QString& src, const QString& dstDir)
{
    m_pProgressMgr->launchInfiniteProgress(tr("Plugin extraction..."), false);

    QFutureWatcher<QStringList>* pWatcher = new QFutureWatcher<QStringList>(this);
    connect(pWatcher, &QFutureWatcher<bool>::finished, [this, pWatcher, src, dstDir]
    {
        //Delete archive file
        QFile file(src);
        file.remove();

        m_pProgressMgr->endInfiniteProgress();
        QStringList files = pWatcher->result();
        onPluginExtractionDone(files, dstDir);
    });

    //Extract plugin into separate thread
    auto future = QtConcurrent::run([src, dstDir]
    {
        return JlCompress::extractDir(src, dstDir);
    });
    pWatcher->setFuture(future);
}

void CStoreManager::publishPluginToServer()
{
    assert(m_pNetworkMgr);

    //Http request to create new plugin
    QByteArray jsonString = createPluginJson();
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/api/plugin/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
        throw CException(HttpExCode::INVALID_URL, url.errorString().toStdString(), __func__, __FILE__, __LINE__);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->post(request, jsonString);
    m_mapTypeRequest.insert(PUBLISH_PLUGIN, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onPublishPluginDone);
}

void CStoreManager::uploadPluginPackage()
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    //Http request to send plugin file
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/package/").arg(m_currentPluginServerId));
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    m_pTranferFile = new QFile(m_currentPluginPackageFile);
    m_pTranferFile->open(QIODevice::ReadOnly);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/zip"));
    QString formData = QString("form-data; name=\"packageFile\"; filename=\"%1\"").arg(m_currentPluginPackageFile);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, m_pTranferFile->size());
    filePart.setBodyDevice(m_pTranferFile);
    pMultiPart->append(filePart);

    //Init progress: size in ko
    m_pProgressMgr->launchProgress(&m_progressSignal, m_pTranferFile->size() / 1024, tr("Uploading plugin..."), false);

    auto pReply = m_pNetworkMgr->put(request, pMultiPart);
    pMultiPart->setParent(pReply);
    m_mapTypeRequest.insert(UPLOAD_PACKAGE, pReply);
    connect(pReply, &QNetworkReply::uploadProgress, this, &CStoreManager::onUploadProgress);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onUploadPackageDone);
}

void CStoreManager::uploadPluginIcon()
{
    assert(m_pNetworkMgr);
    assert(m_pProgressMgr);

    //Http request to send plugin file
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/icon/").arg(m_currentPluginServerId));
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    //Get full path to icon file
    QString iconPath = m_pLocalPluginModel->record(m_currentLocalIndex.row()).value("iconPath").toString();
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
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    QString formData = QString("form-data; name=\"iconFile\"; filename=\"%1\"").arg(iconPath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, pIconFile->size());
    filePart.setBodyDevice(pIconFile);
    pMultiPart->append(filePart);
    pIconFile->setParent(pMultiPart);

    //Init progress: size in ko
    m_pProgressMgr->launchProgress(&m_progressSignal, pIconFile->size() / 1024, tr("Uploading icon..."), false);

    auto pReply = m_pNetworkMgr->put(request, pMultiPart);
    pMultiPart->setParent(pReply);
    m_mapTypeRequest.insert(UPLOAD_ICON, pReply);
    connect(pReply, &QNetworkReply::uploadProgress, this, &CStoreManager::onUploadProgress);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onUploadIconDone);
}

void CStoreManager::downloadPluginPackage(const QString &packageUrl)
{
    assert(m_pNetworkMgr);

    QFileInfo urlInfo(packageUrl);
    m_currentPluginPackageFile = urlInfo.fileName();

    //Http request to download plugin package (zip)
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + packageUrl);
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
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->get(request);
    m_mapTypeRequest.insert(DOWNLOAD_PACKAGE, pReply);
    connect(pReply, &QNetworkReply::downloadProgress, this, &CStoreManager::onDownloadProgress);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onDownloadPackageDone);
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

void CStoreManager::deletePlugin()
{
    assert(m_pNetworkMgr);

    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/").arg(m_currentPluginServerId));
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        clearContext();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->deleteResource(request);
    // Update local database if plugin is really removed from server
    m_mapTypeRequest.insert(DELETE_PLUGIN, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CStoreManager::onDeletePlugin);
}

void CStoreManager::deleteTranferFile()
{
    m_pTranferFile->remove();
    delete m_pTranferFile;
    m_pTranferFile = nullptr;
    m_currentPluginPackageFile.clear();
}

void CStoreManager::clearContext()
{
    m_currentLocalIndex = QModelIndex();
    m_currentServerIndex = QModelIndex();
    m_currentPluginPackageFile.clear();
    m_currentPluginServerId = -1;
    m_bBusy = false;
}

void CStoreManager::finalizePluginInstall(const CTaskInfo& info, const CUser& user)
{
    //Insert or update plugin to file database
    m_dbMgr.insertPlugin(m_currentPluginServerId, info, user);
    //Reload process library
    m_pProgressMgr->launchInfiniteProgress(tr("Reloading plugin..."), false);

    bool bLoaded = m_pProcessMgr->reloadPlugin(QString::fromStdString(info.m_name), info.m_language);
    if(bLoaded)
    {
        createLocalPluginModel();
        qCInfo(logStore).noquote() << tr("Plugin was successfully installed");
    }
    else
        qCWarning(logStore).noquote() << tr("Plugin was successfully installed but failed to load. Try to restart Ikomia Studio.");

    //Clean
    clearContext();
    m_pProgressMgr->endInfiniteProgress();
}

#include "moc_CStoreManager.cpp"
