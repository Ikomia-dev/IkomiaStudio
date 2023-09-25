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
}

CUserManager::~CUserManager()
{
}

void CUserManager::init()
{
    initDb();
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
}

void CUserManager::onConnectUser(const QString &username, const QString &pwd, bool bRememberMe)
{
    m_bRememberMe = bRememberMe;
    if(!m_bRememberMe)
        clearUserInfo();

    connectUser(username, pwd, "");
}

void CUserManager::onDisconnectUser()
{
    disconnectUser();
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
            qCCritical(logUser).noquote() << tr("Invalid username or password");
        else
            qCCritical(logUser).noquote() << pReply->errorString();

        pReply->deleteLater();

        if (requestType == Request::GET_USER)
            createAuthToken(m_usernameTmp, m_pwdTmp);

        return;
    }

    switch(requestType)
    {
        case Request::AUTH_TOKEN:
            loginDone(pReply);
            break;
        case Request::GET_USER:
            fillUserInfo(pReply);
            break;
        case Request::GET_NAMESPACES:
            fillUserNamespaces(pReply);
            break;
    }
    pReply->deleteLater();
}

void CUserManager::loginDone(QNetworkReply* pReply)
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document while getting authentication token");
        manageGetUserInfoError();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document structure while getting authentication token");
        manageGetUserInfoError();
        return;
    }
    QJsonObject jsonToken = doc.object();
    m_currentUser.m_token = jsonToken["clear_token"].toString();
    retrieveUserInfo();
    qCInfo(logUser).noquote() << tr("Connection successfull");

    if(m_bRememberMe)
        saveLoginInfo();

    //Notify Matomo server
    PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getMatomoUrl()), MATOMO_APP_ID);
    pPiwikTracker->sendEvent("main/user", "user", "connection", "User_Connected");

    emit doShowNotification(tr("Connection successfull"), Notification::INFO);
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

    if(connectedUser != m_currentUser)
    {
        connectedUser.m_token = m_currentUser.m_token;
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
        if(!q.exec(QString("CREATE TABLE user (id INTEGER PRIMARY KEY, login BLOB, password BLOB, token BLOB);")))
            qCCritical(logUser).noquote() << q.lastError().text();
    }
}

void CUserManager::createAuthToken(const QString &username, const QString &pwd)
{
    // Basic authentication header
    QString concatenated = username + ":" + pwd;
    QByteArray authData = concatenated.toLocal8Bit().toBase64();
    QString headerAuthData = "Basic " + authData;

    // Request data for token creation
    QJsonObject jsonData;
    jsonData["name"] = "Ikomia Studio token";
    jsonData["ttl"] = m_tokenTTL;
    QJsonDocument jsonDoc(jsonData);

    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/v1/users/me/tokens/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "Ikomia Studio");
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("Authorization", headerAuthData.toLocal8Bit());

    auto pReply = m_pNetworkMgr->post(request, jsonDoc.toJson());
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, Request::AUTH_TOKEN);
    });
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

void CUserManager::connectUser(const QString &username, const QString &pwd, const QString& token)
{
    assert(m_pNetworkMgr);
    m_usernameTmp = username;
    m_pwdTmp = pwd;
    m_currentUser.m_token = token;

    if (token.isEmpty())
        createAuthToken(username, pwd);
    else
        retrieveUserInfo();
}

void CUserManager::disconnectUser()
{
    assert(m_pNetworkMgr);
    qCInfo(logUser).noquote() << tr("User sucessfully disconnected");
    m_currentUser.logout();
    m_usernameTmp.clear();
    m_pwdTmp.clear();
    clearUserInfo();
    emit doSetCurrentUser(m_currentUser);
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
    if(!q.exec("SELECT login, password, token FROM user;"))
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
        QByteArray decodedToken = encryption.decode(q.value(2).toByteArray(), hashKey, hashIV);
        QString strLogin = QString(encryption.removePadding(decodedLogin));
        QString strPwd = QString(encryption.removePadding(decodedPwd));
        QString strToken = QString(encryption.removePadding(decodedToken));
        m_bRememberMe = true;
        connectUser(strLogin, strPwd, strToken);
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
    request.setRawHeader("User-Agent", "Ikomia Studio");

    if (m_currentUser.isConnected())
        request.setRawHeader("Authorization", m_currentUser.getAuthHeader());

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
    request.setRawHeader("User-Agent", "Ikomia Studio");
    request.setRawHeader("Authorization", m_currentUser.getAuthHeader());

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, Request::GET_NAMESPACES);
    });
}

void CUserManager::manageGetUserInfoError()
{
    m_currentUser.logout();
    emit doSetCurrentUser(m_currentUser);
    emit doShowNotification(tr("Authentication failed"), Notification::INFO, nullptr, 5000);
}

void CUserManager::saveLoginInfo()
{
    QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
    QByteArray hashKey = QCryptographicHash::hash(getBytesFromImage(":/Images/key.png"), QCryptographicHash::Sha256);
    QByteArray hashIV = QCryptographicHash::hash(getBytesFromImage(":/Images/iv.png"), QCryptographicHash::Md5);
    QByteArray encodedUsername = encryption.encode(m_usernameTmp.toLocal8Bit(), hashKey, hashIV);
    QByteArray encodedPwd = encryption.encode(m_pwdTmp.toLocal8Bit(), hashKey, hashIV);
    QByteArray encodedToken = encryption.encode(m_currentUser.m_token.toLocal8Bit(), hashKey, hashIV);

    auto db = Utils::Database::connect(Utils::Database::getMainPath(), Utils::Database::getMainConnectionName());
    if(db.isValid() == false)
    {
        qCCritical(logUser).noquote() << db.lastError().text();
        return;
    }

    //We save only one user
    clearUserInfo();

    QSqlQuery q(db);
    q.prepare("INSERT INTO user (login, password, token) VALUES (:login, :password, :token);");
    q.bindValue(":login", encodedUsername);
    q.bindValue(":password", encodedPwd);
    q.bindValue(":token", encodedToken);

    if(!q.exec())
        qCCritical(logUser).noquote() << q.lastError().text();
}

void CUserManager::clearUserInfo()
{
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
