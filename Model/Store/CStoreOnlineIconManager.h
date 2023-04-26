/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSTOREONLINEICONMANAGER_H
#define CSTOREONLINEICONMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "Model/User/CUser.h"

class CPluginModel;

class CStoreOnlineIconManager: public QObject
{
    Q_OBJECT

    public:

        CStoreOnlineIconManager(CPluginModel* pModel, QNetworkAccessManager *pNetworkMgr, const CUser &user);

        void    loadIcons();

    signals:

        void    doIconsLoaded();

    private:

        QString getPluginIconPath(const QString& name) const;

        bool    isIconExists(const QString& pluginName);

        void    incrementLoadedIcon();

        void    savePluginIcon(QNetworkReply *pReply, int pluginIndex);

    private slots:

        void    onReplyReceived(QNetworkReply* pReply, int pluginIndex);

    private:

        QNetworkAccessManager*      m_pNetworkMgr = nullptr;
        CUser                       m_currentUser;
        CPluginModel*               m_pModel = nullptr;
        int                         m_nbIcons = 0;
        int                         m_nbLoadedIcons = 0;
};

#endif // CSTOREONLINEICONMANAGER_H
