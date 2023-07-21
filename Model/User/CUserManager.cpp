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

#include "CUserManager.h"
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Model/Matomo/piwiktracker.h"
#include "CUserSqlQueryModel.h"
#include "qaesencryption.h"

CUserManager::CUserManager()
{
    m_pTimerSingleConnection = new QTimer(this);
}

CUserManager::~CUserManager()
{
}

void CUserManager::init()
{
    initDb();
    initConnections();
}

void CUserManager::setManagers(QNetworkAccessManager *pNetworkMgr)
{
    m_pNetworkMgr = pNetworkMgr;
}

void CUserManager::notifyViewShow()
{
    checkAutoLogin();
}

void CUserManager::beforeAppClose()
{
    disconnectUser(true);
}

void CUserManager::onConnectUser(const QString &login, const QString &pwd, bool bRememberMe)
{
    if(bRememberMe)
    {
        m_loginTmp = login;
        m_pwdTmp = pwd;
    }
    else
        clearUserInfo();

    connectUser(login, pwd);
}

void CUserManager::onDisconnectUser()
{
    disconnectUser(false);
}

void CUserManager::onReplyReceived(QNetworkReply *pReply, Request requestType)
{
    if (pReply == nullptr)
    {
        qCCritical(logHub).noquote() << "Invalid reply from Ikomia Scale";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        if(pReply->error() == QNetworkReply::ProtocolInvalidOperationError)
            qCCritical(logUser).noquote() << tr("Invalid login or password");
        else
            qCCritical(logUser).noquote() << pReply->errorString();

        pReply->deleteLater();
        return;
    }

    switch(requestType)
    {
        case Request::LOGIN:
            loginDone(pReply);
            break;
        case Request::GET_USER:
            fillUserInfo(pReply);
            break;
        case Request::GET_NAMESPACES:
            fillUserNamespaces(pReply);
            break;
        case Request::LOGOUT:
            logoutDone();
            break;
    }
    pReply->deleteLater();
}

void CUserManager::loginDone(QNetworkReply* pReply)
{
    QVariant cookieHeader = pReply->header(QNetworkRequest::SetCookieHeader);
    if (!cookieHeader.isValid())
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid connection reply from Ikomia Scale");
        return;
    }

    m_sessionCookies = cookieHeader.value<QList<QNetworkCookie>>();
    retrieveUserInfo();
    qCInfo(logUser).noquote() << tr("Connection successfull");

    if(m_loginTmp.isEmpty() == false && m_pwdTmp.isEmpty() == false)
        saveLoginInfo();

    //Notify Matomo server
    PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getMatomoUrl()), MATOMO_APP_ID);
    pPiwikTracker->sendEvent("main/user", "user", "connection", "User_Connected");

    emit doShowNotification(tr("Connection successfull"), Notification::INFO);
    m_pTimerSingleConnection->start(Ikomia::_UserCheckFrequency);
}

void CUserManager::logoutDone()
{
    m_pTimerSingleConnection->stop();
    qCInfo(logUser).noquote() << tr("User sucessfully disconnected");
    m_currentUser.logout();
    m_sessionCookies.clear();
    clearUserInfo();
    emit doSetCurrentUser(m_currentUser);
}

void CUserManager::fillUserInfo(QNetworkReply* pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document while getting user information");
        manageGetUserInfoError();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document structure while getting user information");
        manageGetUserInfoError();
        return;
    }
    QJsonObject jsonUser = doc.object();
    CUser connectedUser;
    connectedUser.m_name = jsonUser["username"].toString();
    connectedUser.m_firstName = jsonUser["first_name"].toString();
    connectedUser.m_lastName = jsonUser["last_name"].toString();
    connectedUser.m_email = jsonUser["email"].toString();
    connectedUser.m_url = jsonUser["url"].toString();
    connectedUser.m_namespaceUrl = jsonUser["namespace"].toString();
    connectedUser.m_sessionCookies = m_sessionCookies;

    if(connectedUser != m_currentUser)
    {
        m_currentUser = connectedUser;
        retrieveUserNamespaces(Utils::Network::getBaseUrl() + "/v1/namespaces/");
    }
}

void CUserManager::fillUserNamespaces(QNetworkReply *pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logHub).noquote() << tr("Invalid JSON document while retrieving namespaces");
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logHub).noquote() << tr("Invalid JSON document structure while retrieving namespaces");
        return;
    }

    QJsonObject jsonPage = doc.object();
    if (jsonPage["next"].isNull() == false)
    {
        int count = jsonPage["count"].toInt();
        retrieveUserNamespaces(QString(Utils::Network::getBaseUrl() + "/v1/namespaces/?page_size=%1").arg(count));
    }
    else
    {
        QJsonArray namespaces = jsonPage["results"].toArray();
        for (int i=0; i<namespaces.size(); i++)
        {
            QJsonObject ns = namespaces[i].toObject();
            m_currentUser.addNamespace(ns);
        }
        emit doSetCurrentUser(m_currentUser);
    }
}

void CUserManager::onCheckSingleConnection()
{
    retrieveUserInfo();
}

void CUserManager::initDb()
{
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
    {
        qCCritical(logUser).noquote() << db.lastError().text();
        return;
    }

    QStringList tables = db.tables(QSql::Tables);
    if(tables.contains("user") == false)
    {
        QSqlQuery q(db);
        if(!q.exec(QString("CREATE TABLE user (id INTEGER PRIMARY KEY, login BLOB, password BLOB);")))
            qCCritical(logUser).noquote() << q.lastError().text();
    }
}

void CUserManager::initConnections()
{
    connect(m_pTimerSingleConnection, &QTimer::timeout, this, &CUserManager::onCheckSingleConnection);
}

QByteArray CUserManager::getBytesFromImage(const QString &path) const
{
    QPixmap pixmap(path);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return bytes;
}

void CUserManager::connectUser(const QString &login, const QString &pwd)
{
    assert(m_pNetworkMgr);

    //Http request to login
    QJsonObject jsonLogin;
    jsonLogin["username"] = login;
    jsonLogin["password"] = pwd;
    QJsonDocument jsonDoc(jsonLogin);

    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/v1/accounts/signin/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    auto pReply = m_pNetworkMgr->post(request, jsonDoc.toJson());
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, Request::LOGIN);
    });
}

void CUserManager::disconnectUser(bool bSynchronous)
{
    assert(m_pNetworkMgr);

    //Http request to logout
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/v1/accounts/signout/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_currentUser.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_currentUser.getSessionCookie("csrftoken"));

    auto pReply = m_pNetworkMgr->post(request, QByteArray());
    if(bSynchronous)
    {
        while(pReply->isFinished() == false)
            QCoreApplication::processEvents();
    }
    else
    {
        connect(pReply, &QNetworkReply::finished, [=](){
           this->onReplyReceived(pReply, Request::LOGOUT);
        });
    }
}

void CUserManager::checkAutoLogin()
{
    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
    {
        qCCritical(logUser).noquote() << db.lastError().text();
        return;
    }

    QSqlQuery q(db);
    if(!q.exec("SELECT login, password FROM user;"))
    {
        qCCritical(logUser).noquote() << q.lastError().text();
        return;
    }

    //It's a one row table
    if(q.next())
    {
        QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
        QByteArray hashKey = QCryptographicHash::hash(getBytesFromImage(":/Images/key.png"), QCryptographicHash::Sha256);
        QByteArray hashIV = QCryptographicHash::hash(getBytesFromImage(":/Images/iv.png"), QCryptographicHash::Md5);
        QByteArray decodedLogin = encryption.decode(q.value(0).toByteArray(), hashKey, hashIV);
        QByteArray decodedPwd = encryption.decode(q.value(1).toByteArray(), hashKey, hashIV);
        QString strLogin = QString(encryption.removePadding(decodedLogin));
        QString strPwd = QString(encryption.removePadding(decodedPwd));
        connectUser(strLogin, strPwd);
    }
}

void CUserManager::retrieveUserInfo()
{
    assert(m_pNetworkMgr);

    //Http request to retrieve logged user info
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/v1/users/me/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, Request::GET_USER);
    });
}

void CUserManager::retrieveUserNamespaces(const QString& strUrl)
{
    assert(m_pNetworkMgr);

    //Http request to retrieve logged user namespaces
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, Request::GET_NAMESPACES);
    });
}

void CUserManager::manageGetUserInfoError()
{
    m_pTimerSingleConnection->stop();
    m_currentUser.logout();
    m_sessionCookies.clear();
    emit doSetCurrentUser(m_currentUser);
    emit doShowNotification(tr("You have been disconnected.\nMain reasons are internet connection issues or concurrent login for a single account."),
                            Notification::INFO,
                            nullptr,
                            5000);
}

void CUserManager::saveLoginInfo()
{
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray hashKey = QCryptographicHash::hash(getBytesFromImage(":/Images/key.png"), QCryptographicHash::Sha256);
    QByteArray hashIV = QCryptographicHash::hash(getBytesFromImage(":/Images/iv.png"), QCryptographicHash::Md5);
    QByteArray encodedLogin = encryption.encode(m_loginTmp.toLocal8Bit(), hashKey, hashIV);
    QByteArray encodedPwd = encryption.encode(m_pwdTmp.toLocal8Bit(), hashKey, hashIV);

    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
    {
        qCCritical(logUser).noquote() << db.lastError().text();
        return;
    }

    //We save only one user
    clearUserInfo();

    QSqlQuery q(db);
    q.prepare("INSERT INTO user (login, password) VALUES (:login, :password);");
    q.bindValue(":login", encodedLogin);
    q.bindValue(":password", encodedPwd);

    if(!q.exec())
        qCCritical(logUser).noquote() << q.lastError().text();
}

void CUserManager::clearUserInfo()
{
    m_loginTmp.clear();
    m_pwdTmp.clear();

    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
    {
        qCCritical(logUser).noquote() << db.lastError().text();
        return;
    }

    QSqlQuery q(db);
    if(!q.exec("DELETE FROM user;"))
        qCCritical(logUser).noquote() << q.lastError().text();
}

#include "moc_CUserManager.cpp"
