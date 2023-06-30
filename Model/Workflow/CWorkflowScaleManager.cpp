#include "CWorkflowScaleManager.h"
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpPart>
#include "UtilsTools.hpp"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Model/Workflow/CWorkflowPackage.h"

CWorkflowScaleManager::CWorkflowScaleManager()
{
}

void CWorkflowScaleManager::setManagers(QNetworkAccessManager *pNetMgr)
{
    m_pNetworkMgr = pNetMgr;
}

void CWorkflowScaleManager::setCurrentUser(const CUser &user)
{
    m_user = user;
}

void CWorkflowScaleManager::onReplyReceived(QNetworkReply *pReply, RequestType requestType)
{
    if (pReply == nullptr)
    {
        qCCritical(logStore).noquote() << "Invalid reply from Ikomia Scale";
        return;
    }

    if(pReply->error() != QNetworkReply::NoError)
    {
        qCCritical(logStore).noquote() << pReply->errorString();
        QString content(pReply->readAll());
        Utils::print(content.toStdString(), QtDebugMsg);
        pReply->deleteLater();
        clearContext();
        return;
    }

    switch(requestType)
    {
        case RequestType::GET_PROJECTS:
            fillProjects(pReply);
            break;
        case RequestType::CREATE_PROJECT:
            addProject(pReply);
            break;
        case RequestType::PUBLISH_WORKFLOW:
            clearContext();
            break;
    }
    pReply->deleteLater();
}

void CWorkflowScaleManager::requestProjects()
{
    m_projects = QJsonArray();
    requestProjects(Utils::Network::getBaseUrl() + "/v1/projects/");
}

void CWorkflowScaleManager::publishWorkflow(const QString &path, bool bNewProject, const QString &projectName,
                                            const QString &projectDescription, const QString &projectNamespace)
{
    if (Utils::File::isFileExist(path.toStdString()) == false)
    {
        std::string error = "Workflow publication failed, file " + path.toStdString() + " does not exist";
        throw CException(CoreExCode::INVALID_FILE, error, __FILE__, __func__, __LINE__);
    }
    m_wfPath = path;

    if (bNewProject)
        createProject(projectName, projectDescription, projectNamespace);
    else
    {
        // Get project URL
        QString projectUrl;
        for (int i=0; i<m_projects.size(); ++i)
        {
            QJsonObject project = m_projects[i].toObject();
            if (project["name"].toString() == projectName)
            {
                projectUrl = project["url"].toString();
                break;
            }
        }

        if (projectUrl.isEmpty() == false)
            publishWorkflowPackage(projectUrl);
        else
        {
            std::string error = "Workflow publication failed, project " + projectName.toStdString() + " can't be found";
            throw CException(CoreExCode::INVALID_PARAMETER, error, __FILE__, __func__, __LINE__);
        }
    }
}

QJsonObject CWorkflowScaleManager::getJsonObject(QNetworkReply *pReply, const QString &errorMsg) const
{
    QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
    if(doc.isNull())
    {
        qCCritical(logStore).noquote() << errorMsg << tr(": invalid JSON document");
        return QJsonObject();
    }

    if(doc.isObject() == false)
    {
        qCCritical(logStore).noquote() << errorMsg << tr(":invalid JSON document structure");
        return QJsonObject();
    }
    return doc.object();
}

void CWorkflowScaleManager::requestProjects(const QString &strUrl)
{
    assert(m_pNetworkMgr);
    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logWorkflow) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_user.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);

    auto pReply = m_pNetworkMgr->get(request);
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, RequestType::GET_PROJECTS);
    });
}

void CWorkflowScaleManager::fillProjects(QNetworkReply *pReply)
{
    QJsonObject jsonPage = getJsonObject(pReply, tr("Error while retrieving project list"));
    if (jsonPage.isEmpty())
        return;

    int count = jsonPage["count"].toInt();
    if (jsonPage["next"].isNull() == false)
    {
        QString url = Utils::Network::getBaseUrl() + QString("/v1/projects/?page_size=%1").arg(count);
        requestProjects(url);
    }
    else if (jsonPage["count"] == 0)
    {
        emit doSetProjects(m_projects);
    }
    else
    {
        m_projects = jsonPage["results"].toArray();
        emit doSetProjects(m_projects);
    }
}

void CWorkflowScaleManager::createProject(const QString &name, const QString &description, const QString &namespaceName)
{
    CUserNamespace ns = m_user.getNamespace(namespaceName);
    QString strUrl = ns.m_url + "projects/";

    QUrlQuery urlQuery(strUrl);
    QUrl url(urlQuery.query());

    if(url.isValid() == false)
    {
        qCDebug(logWorkflow) << url.errorString();
        return;
    }

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Content-Type", "application/json");

    QVariant cookieHeaders;
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_user.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_user.getSessionCookie("csrftoken"));

    QJsonObject project;
    project["name"] = name;
    project["description"] = description;
    QJsonDocument payload(project);

    auto pReply = m_pNetworkMgr->post(request, payload.toJson());
    connect(pReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pReply, RequestType::CREATE_PROJECT);
    });
}

void CWorkflowScaleManager::addProject(QNetworkReply *pReply)
{
    QJsonObject project = getJsonObject(pReply, tr("Error while creating project"));
    if (project.isEmpty())
        return;

    m_projects.append(project);
    publishWorkflowPackage(project["url"].toString());
}

void CWorkflowScaleManager::publishWorkflowPackage(const QString &projectUrl)
{
    CWorkflowPackage package(m_wfPath);
    m_zipPath = package.create();

    QUrlQuery urlQuery(projectUrl + "workflows/");
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
    cookieHeaders.setValue<QList<QNetworkCookie>>(m_user.m_sessionCookies);
    request.setHeader(QNetworkRequest::CookieHeader, cookieHeaders);
    request.setRawHeader("X-CSRFToken", m_user.getSessionCookie("csrftoken"));

    // Build multi-part request
    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // Plugin package zip part
    m_pPackageFile = new QFile(m_zipPath);
    m_pPackageFile->open(QIODevice::ReadOnly);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/zip"));
    QString formData = QString("form-data; name=\"archive\"; filename=\"%1\"").arg(m_zipPath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, m_pPackageFile->size());
    filePart.setBodyDevice(m_pPackageFile);
    pMultiPart->append(filePart);

    auto pNewReply = m_pNetworkMgr->post(request, pMultiPart);
    pMultiPart->setParent(pNewReply);

    connect(pNewReply, &QNetworkReply::finished, [=](){
       this->onReplyReceived(pNewReply, RequestType::PUBLISH_WORKFLOW);
    });
}

void CWorkflowScaleManager::clearContext()
{
    // Cleanup
    if (m_pPackageFile)
    {
        m_pPackageFile->remove();
        delete m_pPackageFile;
        m_pPackageFile = nullptr;
    }
    boost::filesystem::remove_all(m_wfPath.toStdString());
    boost::filesystem::remove_all(m_zipPath.toStdString());
}
