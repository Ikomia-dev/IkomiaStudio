#ifndef CWORKFLOWSCALEMANAGER_H
#define CWORKFLOWSCALEMANAGER_H

#include "Model/User/CUser.h"
#include "CProgressSignalHandler.h"

class CProgressBarManager;


class CWorkflowScaleManager : public QObject
{
    Q_OBJECT

    enum class RequestType : int
    {
        GET_PROJECTS,
        CREATE_PROJECT,
        PUBLISH_WORKFLOW
    };

    public:

        CWorkflowScaleManager();

        void        setManagers(QNetworkAccessManager* pNetMgr, CProgressBarManager *pProgressMgr);
        void        setCurrentUser(const CUser &user);

        void        requestProjects();

        void        publishWorkflow(const QString& path, bool bNewProject, const QString& projectName,
                                    const QString& projectDescription, const QString& projectNamespace);

    signals:

        void        doSetProjects(const QJsonArray& projects);

    private slots:

        void        onUploadProgress(qint64 bytesSent, qint64 bytesTotal);

    private:

        QJsonObject getJsonObject(QNetworkReply *pReply, const QString& errorMsg) const;

        void        onReplyReceived(QNetworkReply* pReply, RequestType requestType);

        void        requestProjects(const QString& strUrl);

        void        fillProjects(QNetworkReply* pReply);

        void        createProject(const QString& name, const QString& description, const QString& namespaceName);

        void        addProject(QNetworkReply* pReply);

        void        publishWorkflowPackage(const QString& projectUrl);

        void        clearContext();

    private:

        QNetworkAccessManager*  m_pNetworkMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CProgressSignalHandler  m_progressSignal;
        QFile*                  m_pPackageFile = nullptr;
        CUser                   m_user;
        QJsonArray              m_projects;
        QString                 m_wfPath;
        QString                 m_zipPath;
};

#endif // CWORKFLOWSCALEMANAGER_H
