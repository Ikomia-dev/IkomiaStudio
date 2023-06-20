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
#include "CPluginModel.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Task/CTaskInfo.h"

CStoreOnlineIconManager::CStoreOnlineIconManager(CPluginModel *pModel, QNetworkAccessManager *pNetworkMgr, const CUser &user)
{
    m_pModel = pModel;
    m_pNetworkMgr = pNetworkMgr;
    m_currentUser = user;
}

void CStoreOnlineIconManager::loadIcons()
{
    assert(m_pNetworkMgr);
    assert(m_pModel);

    m_nbIcons = m_nbLoadedIcons = 0;
    QJsonArray plugins = m_pModel->getJsonPlugins();

    //Count the number of icons to load
    for(int i=0; i<plugins.size(); ++i)
    {
        QJsonObject plugin = plugins[i].toObject();
        if(isIconExists(plugin["name"].toString()) == true)
            m_pModel->setPluginField(i, "icon_path", getPluginIconPath(plugin["name"].toString()));
        else
            m_nbIcons++;
    }

    if(m_nbIcons == 0)
    {
        emit doIconsLoaded();
        return;
    }

    //Load them
    for(int i=0; i<plugins.size(); ++i)
    {
        QJsonObject plugin = plugins[i].toObject();
        if(isIconExists(plugin["name"].toString()) == false)
        {
            //Http request to get plugin icon url
            QUrlQuery urlQuery(plugin["icon"].toString());
            QUrl url(urlQuery.query());

            if(url.isValid() == false)
            {
                qCDebug(logStore) << "Algorithm " << plugin["name"].toString() << "does not have icon." << url.errorString();
                incrementLoadedIcon();
                continue;
            }

            QNetworkRequest request;
            request.setUrl(url);

            if (m_pModel->getType() == CPluginModel::Type::WORKSPACE)
            {
                QVariant cookieHeaders;
                cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
                request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
            }

            auto pReply = m_pNetworkMgr->get(request);
            connect(pReply, &QNetworkReply::finished, [=](){
               this->onReplyReceived(pReply, i);
            });
        }
    }
}

QString CStoreOnlineIconManager::getPluginIconPath(const QString &name) const
{
    QString destDir = Utils::IkomiaApp::getQIkomiaFolder() + "/Resources/Tmp/";
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

void CStoreOnlineIconManager::onReplyReceived(QNetworkReply *pReply, int pluginIndex)
{
    if (pReply == nullptr)
    {
        qCCritical(logStore).noquote() << "Invalid reply from Ikomia Scale";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        qCCritical(logStore).noquote() << pReply->errorString();
        pReply->deleteLater();
        return;
    }

    savePluginIcon(pReply, pluginIndex);
    pReply->deleteLater();
}

void CStoreOnlineIconManager::incrementLoadedIcon()
{
    m_nbLoadedIcons++;

    if(m_nbIcons == m_nbLoadedIcons)
        emit doIconsLoaded();
}

void CStoreOnlineIconManager::savePluginIcon(QNetworkReply* pReply, int pluginIndex)
{
    QJsonArray plugins = m_pModel->getJsonPlugins();
    QJsonObject plugin = plugins[pluginIndex].toObject();
    QString pluginName = plugin["name"].toString();
    QString destDir = Utils::IkomiaApp::getQIkomiaFolder() + "/Resources/Tmp/";

    try
    {
        Utils::File::createDirectory(destDir.toStdString());
    }
    catch (const CException& e)
    {
        qCWarning(logStore).noquote() << QString::fromStdString(e.what());
        incrementLoadedIcon();
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
        return;
    }

    bool bSaved = icon.save(downloadPath);
    if(bSaved == false)
    {
        qCWarning(logStore).noquote() << tr("Unable to save logo for plugin %1").arg(pluginName);
        incrementLoadedIcon();
        return;
    }

    //Set plugin icon path
    m_pModel->setPluginField(pluginIndex, "icon_path", downloadPath);
    incrementLoadedIcon();
}
