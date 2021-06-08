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

#include "CStoreOnlineIconManager.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpPart>
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Core/CTaskInfo.h"

CStoreOnlineIconManager::CStoreOnlineIconManager(QNetworkAccessManager *pNetworkMgr, const CUser &user)
{
    m_pNetworkMgr = pNetworkMgr;
    m_currentUser = user;
}

void CStoreOnlineIconManager::loadIcons(QJsonArray* pPlugins)
{
    assert(m_pNetworkMgr);

    m_pPlugins = pPlugins;
    int currentOS = Utils::OS::getCurrent();
    m_nbIcons = m_nbLoadedIcons = 0;
    connect(m_pNetworkMgr, &QNetworkAccessManager::finished, this, &CStoreOnlineIconManager::onReplyFinished);

    //Count the number of icons to load
    for(int i=0; i<m_pPlugins->size(); ++i)
    {
        QJsonObject plugin = (*m_pPlugins)[i].toObject();
        int pluginOS = plugin["os"].toInt();

        if(pluginOS == OSType::ALL || currentOS == pluginOS)
        {
            if(isIconExists(plugin["name"].toString()) == true)
                setPluginIconPath(i, getPluginIconPath(plugin["name"].toString()));
            else
                m_nbIcons++;
        }
    }

    if(m_nbIcons == 0)
    {
        emit doIconsLoaded();
        return;
    }

    //Load them
    for(int i=0; i<m_pPlugins->size(); ++i)
    {
        QJsonObject plugin = (*m_pPlugins)[i].toObject();
        int pluginOS = plugin["os"].toInt();

        if((pluginOS == OSType::ALL || currentOS == pluginOS) && (isIconExists(plugin["name"].toString()) == false))
        {
            //Http request to get plugin icon url
            QUrlQuery urlQuery(Utils::Network::getBaseUrl() + QString("/api/plugin/%1/icon/").arg(plugin["id"].toInt()));
            QUrl url(urlQuery.query());

            if(url.isValid() == false)
            {
                qCDebug(logStore) << url.errorString();
                return;
            }

            QNetworkRequest request;
            request.setUrl(url);
            request.setRawHeader("Content-Type", "application/json");
            QString token = "Token " + m_currentUser.m_token;
            request.setRawHeader("Authorization", token.toLocal8Bit());

            auto pReply = m_pNetworkMgr->get(request);
            m_mapReplyPluginIndex.insert(pReply, i);
            m_mapReplyType.insert(pReply, GET_ICON_URL);
        }
    }
}

void CStoreOnlineIconManager::setPluginIconPath(int index, const QString &path)
{
    QJsonObject plugin = (*m_pPlugins)[index].toObject();
    plugin["iconPath"] = path;
    (*m_pPlugins)[index] = plugin;
}

QString CStoreOnlineIconManager::getPluginIconPath(const QString &name) const
{
    QString destDir = Utils::IkomiaApp::getQAppFolder() + "/Resources/Tmp/";
    QString iconPath = destDir + QString::fromStdString(Utils::String::httpFormat(name.toStdString())) + ".png";
    return iconPath;
}

bool CStoreOnlineIconManager::isIconExists(const QString &pluginName)
{
    try
    {
        QString iconPath = getPluginIconPath(pluginName);
        boost::filesystem::path boostIconPath(iconPath.toStdString());
        return boost::filesystem::exists(boostIconPath);
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        qCCritical(logStore).noquote() << QString::fromStdString(e.code().message());
        return false;
    }
}

bool CStoreOnlineIconManager::checkReply(QNetworkReply *pReply)
{
    if(pReply == nullptr)
    {
        incrementLoadedIcon();
        return false;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        qCWarning(logStore).noquote() << pReply->errorString();
        incrementLoadedIcon();
        pReply->deleteLater();
        return false;
    }
    return true;
}

void CStoreOnlineIconManager::incrementLoadedIcon()
{
    m_nbLoadedIcons++;

    if(m_nbIcons == m_nbLoadedIcons)
        emit doIconsLoaded();
}

void CStoreOnlineIconManager::downloadPluginIcon(int pluginIndex, const QString &iconUrl)
{
    assert(m_pNetworkMgr);

    //Http request to download plugin icon
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + iconUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logStore) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->get(request);
    m_mapReplyPluginIndex.insert(pReply, pluginIndex);
    m_mapReplyType.insert(pReply, DOWNLOAD_ICON);
}

void CStoreOnlineIconManager::onReplyFinished(QNetworkReply *pReply)
{
    if(pReply == nullptr)
        return;

    int type = m_mapReplyType.value(pReply, -1);
    switch(type)
    {
        case GET_ICON_URL: onGetIconUrlDone(pReply); break;
        case DOWNLOAD_ICON: onDownloadIconDone(pReply); break;
        default: break;
    }
}

void CStoreOnlineIconManager::onGetIconUrlDone(QNetworkReply* pReply)
{
    if(checkReply(pReply) == false)
        return;

    int pluginIndex = m_mapReplyPluginIndex.value(pReply, -1);
    if(pluginIndex == -1)
    {
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCWarning(logStore).noquote() << tr("Icon loading failed: invalid JSON document");
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    if(doc.isObject() == false)
    {
        qCWarning(logStore).noquote() << tr("Icon loading failed: invalid JSON document structure");
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    QJsonObject jsonIcon = doc.object();
    QString url = jsonIcon["iconFile"].toString();

    if(url.isEmpty())
    {
        qCWarning(logStore).noquote() << tr("Icon loading failed: no icon found on server");
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    downloadPluginIcon(pluginIndex, url);
    pReply->deleteLater();
}

void CStoreOnlineIconManager::onDownloadIconDone(QNetworkReply* pReply)
{
    if(checkReply(pReply) == false)
        return;

    int pluginIndex = m_mapReplyPluginIndex.value(pReply, -1);
    if(pluginIndex == -1)
    {
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    QJsonObject plugin = (*m_pPlugins)[pluginIndex].toObject();
    QString pluginName = plugin["name"].toString();
    QString destDir = Utils::IkomiaApp::getQAppFolder() + "/Resources/Tmp/";

    try
    {
        Utils::File::createDirectory(destDir.toStdString());
    }
    catch (const CException& e)
    {
        qCWarning(logStore).noquote() << QString::fromStdString(e.what());
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    //Save icon to disk
    QImage icon;
    QString downloadPath = destDir + QString::fromStdString(Utils::String::httpFormat(pluginName.toStdString())) + ".png";

    bool bLoaded = icon.loadFromData(pReply->readAll());
    if(bLoaded == false)
    {
        qCWarning(logStore).noquote() << tr("Unable to load logo for plugin %1").arg(pluginName);
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    bool bSaved = icon.save(downloadPath);
    if(bSaved == false)
    {
        qCWarning(logStore).noquote() << tr("Unable to save logo for plugin %1").arg(pluginName);
        incrementLoadedIcon();
        pReply->deleteLater();
        return;
    }

    //Set plugin icon path
    setPluginIconPath(pluginIndex, downloadPath);
    incrementLoadedIcon();
    pReply->deleteLater();
}
