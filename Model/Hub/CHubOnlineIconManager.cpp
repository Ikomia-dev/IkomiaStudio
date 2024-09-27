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

#include "CHubOnlineIconManager.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpPart>
#include "CPluginModel.h"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Model/Common/CHttpRequest.h"
#include "Task/CTaskInfo.h"

CHubOnlineIconManager::CHubOnlineIconManager(CPluginModel *pModel, QNetworkAccessManager *pNetworkMgr, const CUser &user)
{
    m_pModel = pModel;
    m_pNetworkMgr = pNetworkMgr;
    m_currentUser = user;
}

void CHubOnlineIconManager::loadIcons()
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
            try
            {
                //Http request to get plugin icon url
                CHttpRequest request(plugin["icon"].toString());
                auto pReply = m_pNetworkMgr->get(request);
                connect(pReply, &QNetworkReply::finished, [=](){
                    this->onReplyReceived(pReply, i);
                });
            }
            catch (CException& e)
            {
                qCDebug(logHub) << "Algorithm " << plugin["name"].toString() << "does not have icon." << e.what();
                incrementLoadedIcon();
                continue;
            }
        }
    }
}

QString CHubOnlineIconManager::getPluginIconPath(const QString &name) const
{
    QString destDir = Utils::IkomiaApp::getQIkomiaFolder() + "/Resources/Tmp/";
    QString iconPath = destDir + QString::fromStdString(Utils::String::httpFormat(name.toStdString())) + ".png";
    return iconPath;
}

bool CHubOnlineIconManager::isIconExists(const QString &pluginName)
{
    try
    {
        QString iconPath = getPluginIconPath(pluginName);
        boost::filesystem::path boostIconPath(iconPath.toStdString());
        return boost::filesystem::exists(boostIconPath);
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        qCCritical(logHub).noquote() << QString::fromStdString(e.code().message());
        return false;
    }
}

void CHubOnlineIconManager::onReplyReceived(QNetworkReply *pReply, int pluginIndex)
{
    if (pReply == nullptr)
    {
        qCCritical(logHub).noquote() << "Invalid reply from Ikomia Scale";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        qCCritical(logHub).noquote() << pReply->errorString();
        pReply->deleteLater();
        incrementLoadedIcon();
        return;
    }

    savePluginIcon(pReply, pluginIndex);
    pReply->deleteLater();
}

void CHubOnlineIconManager::incrementLoadedIcon()
{
    m_nbLoadedIcons++;

    if(m_nbIcons == m_nbLoadedIcons)
        emit doIconsLoaded();
}

void CHubOnlineIconManager::savePluginIcon(QNetworkReply* pReply, int pluginIndex)
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
        qCWarning(logHub).noquote() << QString::fromStdString(e.what());
        incrementLoadedIcon();
        return;
    }

    //Save icon to disk
    QImage icon;
    QString downloadPath = destDir + QString::fromStdString(Utils::String::httpFormat(pluginName.toStdString())) + ".png";

    bool bLoaded = icon.loadFromData(pReply->readAll());
    if(bLoaded == false)
    {
        qCWarning(logHub).noquote() << tr("Unable to load logo for plugin %1").arg(pluginName);
        incrementLoadedIcon();
        return;
    }

    bool bSaved = icon.save(downloadPath);
    if(bSaved == false)
    {
        qCWarning(logHub).noquote() << tr("Unable to save logo for plugin %1").arg(pluginName);
        incrementLoadedIcon();
        return;
    }

    //Set plugin icon path
    m_pModel->setPluginField(pluginIndex, "icon_path", downloadPath);
    incrementLoadedIcon();
}
