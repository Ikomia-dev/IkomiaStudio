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

#ifndef CUSERMANAGER_H
#define CUSERMANAGER_H

#include <QObject>
#include <QTimer>
#include <QNetworkCookie>
#include "CUser.h"
#include "Main/AppDefine.hpp"

class QSqlQueryModel;
class CUserSqlQueryModel;
class QNetworkAccessManager;
class QNetworkReply;
class CProgressCircle;

class CUserManager : public QObject
{
    Q_OBJECT

    public:

        enum Role { ADMINISTRATOR, USER };
        enum Request { AUTH_TOKEN, GET_USER, GET_NAMESPACES };

        CUserManager();
        ~CUserManager();

        void            init();

        void            setManagers(QNetworkAccessManager* pNetworkMgr);

        void            notifyViewShow();

        void            beforeAppClose();

    signals:

        void            doSetCurrentUser(const CUser& user);
        void            doShowNotification(const QString& text, Notification type, CProgressCircle* pProgress=nullptr, int duration=Ikomia::_NotifDefaultDuration);


    public slots:

        void            onConnectUser(const QString& username, const QString& pwd, bool bRememberMe);
        void            onDisconnectUser();

    private slots:

        void            onReplyReceived(QNetworkReply* pReply, Request requestType);

    private:

        void            initDb();

        void            createAuthToken(const QString &username, const QString &pwd);

        QByteArray      getBytesFromImage(const QString& path) const;

        void            connectUser(const QString &username, const QString &pwd, const QString &token);
        void            disconnectUser();

        void            checkAutoLogin();

        void            loginDone(QNetworkReply *pReply);

        void            retrieveUserInfo();
        void            retrieveUserNamespaces(const QString &strUrl);

        void            fillUserInfo(QNetworkReply *pReply);
        void            fillUserNamespaces(QNetworkReply* pReply);

        void            manageGetUserInfoError();

        void            saveLoginInfo();

        void            clearUserInfo();

    private:

        QNetworkAccessManager*      m_pNetworkMgr = nullptr;
        CUser                       m_currentUser;
        QString                     m_usernameTmp;
        QString                     m_pwdTmp;
        bool                        m_bRememberMe = false;
        const int                   m_tokenTTL = 28800;
};

#endif // CUSERMANAGER_H
