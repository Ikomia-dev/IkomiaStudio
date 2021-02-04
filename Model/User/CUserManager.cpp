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

void CUserManager::onLoginDone()
{
    auto pReply = checkReply(LOGIN);
    if(pReply == nullptr)
        return;

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document");
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document structure");
        return;
    }
    QJsonObject jsonPlugin = doc.object();
    m_sessionToken = jsonPlugin["key"].toString();
    retrieveUserInfo();
    qCInfo(logUser).noquote() << tr("Connection successfull");
    pReply->deleteLater();

    if(m_loginTmp.isEmpty() == false && m_pwdTmp.isEmpty() == false)
        saveLoginInfo();

    //Notify Matomo server
    PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getMatomoUrl()), MATOMO_APP_ID);
    pPiwikTracker->sendEvent("main/user", "user", "connection", "User_Connected");

    emit doShowNotification(tr("Connection successfull"), Notification::INFO);
    m_pTimerSingleConnection->start(Ikomia::_UserCheckFrequency);
}

void CUserManager::onLogoutDone()
{
    auto pReply = checkReply(LOGOUT);
    if(pReply == nullptr)
        return;

    m_pTimerSingleConnection->stop();
    qCInfo(logUser).noquote() << tr("User sucessfully disconnected");
    m_currentUser.logout();
    m_sessionToken.clear();
    clearUserInfo();
    emit doSetCurrentUser(m_currentUser);
}

void CUserManager::onRetrieveUserInfoDone()
{
    auto pReply = checkReply(GET_USER);
    if(pReply == nullptr)
    {
        manageGetUserInfoError();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document");
        manageGetUserInfoError();
        return;
    }

    if(doc.isObject() == false)
    {
        qCCritical(logUser).noquote().noquote() << tr("Invalid JSON document structure");
        manageGetUserInfoError();
        return;
    }
    QJsonObject jsonUser = doc.object();
    CUser connectedUser;
    connectedUser.m_token = m_sessionToken;
    connectedUser.m_id = jsonUser["pk"].toInt();
    connectedUser.m_name = jsonUser["username"].toString();
    connectedUser.m_firstName = jsonUser["first_name"].toString();
    connectedUser.m_lastName = jsonUser["last_name"].toString();
    connectedUser.m_email = jsonUser["email"].toString();
    connectedUser.m_reputation = jsonUser["reputation"].toInt();

    if(connectedUser != m_currentUser)
    {
        m_currentUser = connectedUser;
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

    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/api/rest-auth/login/");
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
    m_mapTypeRequest.insert(LOGIN, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CUserManager::onLoginDone);
}

void CUserManager::disconnectUser(bool bSynchronous)
{
    assert(m_pNetworkMgr);

    //Http request to logout
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/api/rest-auth/logout/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QString token = "Token " + m_currentUser.m_token;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->post(request, QByteArray());
    if(bSynchronous)
    {
        while(pReply->isFinished() == false)
            QCoreApplication::processEvents();
    }
    else
    {
        m_mapTypeRequest.insert(LOGOUT, pReply);
        connect(pReply, &QNetworkReply::finished, this, &CUserManager::onLogoutDone);
    }
}

QNetworkReply *CUserManager::checkReply(int type) const
{
    auto it = m_mapTypeRequest.find(type);
    if(it == m_mapTypeRequest.end())
        return nullptr;

    QNetworkReply* pReply = it.value();
    auto error = pReply->error();

    if(error != QNetworkReply::NoError)
    {
        if(error == QNetworkReply::ProtocolInvalidOperationError)
            qCCritical(logUser).noquote() << tr("Invalid login or password");
        else
            qCCritical(logUser).noquote() << pReply->errorString();

        pReply->deleteLater();
        return nullptr;
    }
    return pReply;
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

    //Http request to retrieve user info from token
    QUrlQuery urlQuery(Utils::Network::getBaseUrl() + "/api/rest-auth/user/");
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logUser) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    QString token = "Token " + m_sessionToken;
    request.setRawHeader("Authorization", token.toLocal8Bit());

    auto pReply = m_pNetworkMgr->get(request);
    m_mapTypeRequest.insert(GET_USER, pReply);
    connect(pReply, &QNetworkReply::finished, this, &CUserManager::onRetrieveUserInfoDone);
}

void CUserManager::manageGetUserInfoError()
{
    m_pTimerSingleConnection->stop();
    m_currentUser.logout();
    m_sessionToken.clear();
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
