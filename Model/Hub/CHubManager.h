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

#ifndef CHUBMANAGER_H
#define CHUBMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "CHubDbManager.h"
#include "CPluginModel.h"
#include "Model/User/CUser.h"

class QNetworkAccessManager;
class CProcessManager;
class CPluginManager;
class CProgressBarManager;
class CWorkflowManager;
class CProgressCircle;

class CHubManager : public QObject
{
    Q_OBJECT

    enum class HubRequestType : int
    {
        GET_PLUGINS,
        GET_PLUGIN_DETAILS,
        GET_PACKAGE_URL,
        GET_NEXT_PUBLISH_INFO,
        PUBLISH_HUB,
        PUBLISH_WORKSPACE,
        UPDATE_PLUGIN,
        UPLOAD_PACKAGE,
        UPLOAD_ICON,
        UPLOAD_WORKFLOW,
        DOWNLOAD_PACKAGE
    };

    public:

        CHubManager();
        ~CHubManager();

        void            setManagers(QNetworkAccessManager *pNetworkMgr, CProcessManager* pProcessMgr, CPluginManager* pPluginMgr, CProgressBarManager *pProgressMgr, CWorkflowManager *pWorkflowMgr);
        void            setCurrentUser(const CUser& user);

    signals:

        void            doSetPluginModel(CPluginModel* pModel);
        void            doNotifyModelError(CPluginModel* pModel);
        void            doSetNextPublishInfo(const QModelIndex& index, const QJsonObject& publishInfo);

        void            doRestartIkomia();

    public slots:

        void            onRequestHubModel();
        void            onRequestWorkspaceModel();
        void            onRequestLocalModel();
        void            onRequestNextPublishInfo(const QModelIndex& index);
        void            onPublishHub(const QModelIndex& index, const QJsonObject &info);
        void            onPublishWorkspace(const QModelIndex& index, const QString &workspace);
        void            onInstallPlugin(CPluginModel::Type type, const QModelIndex& index);
        void            onHubSearchChanged(const QString& text);
        void            onWorkspaceSearchChanged(const QString& text);
        void            onLocalSearchChanged(const QString& text);

    private slots:

        void            onReplyReceived(QNetworkReply* pReply, CPluginModel *pModel, HubRequestType requestType);
        void            onUpdateProgress(qint64 bytesSent, qint64 bytesTotal, CProgressCircle* pProgress, const QString &msg);

    private:

        void            initTransferProgress(QNetworkReply *pReply, const QString &msg, size_t steps);

        QJsonObject     getJsonObject(QNetworkReply *pReply, const QString& errorMsg) const;
        QString         getQuery(CPluginModel::Type serverType, const QString &text) const;

        void            createHubPluginModel();
        void            createWorkspacePluginModel();
        void            createQueryModel(CPluginModel* pModel);
        QByteArray      createPluginPayload(CPluginModel *pModel);
        QJsonObject     createPluginPackagePayload(CPluginModel *pModel);
        QString         createDemoWorkflow();

        QString         checkPythonPluginDirectory(const QString &directory);
        QString         checkCppPluginDirectory(const QString &directory, const QString& name);
        void            checkPendingUpdates();
        void            checkInstalledModules(const QString &pluginDir);

        void            queryServerPlugins(CPluginModel* pModel, const QString& strUrl);
        void            queryServerPluginDetails(CPluginModel *pModel, QString strUrl);
        void            queryServerInstallPlugin(CPluginModel* pModel, const QString& strUrl);

        void            updateWorkspacePlugin(const QString &name);
        void            updateLocalPlugin();

        void            fillServerPluginModel(CPluginModel *pModel, QNetworkReply *pReply);

        void            addPluginToModel(CPluginModel *pModel, QNetworkReply *pReply);

        void            validateServerPluginModel(CPluginModel *pModel);
        void            validatePluginFolder(CPluginModel *pModel, const QStringList &files, const QString dstDir);

        void            generateZipFile();
        void            extractZipFile(CPluginModel *pModel, const QString &src, const QString &dstDir);

        void            publishToHub(const QModelIndex &index, const QJsonObject &info);
        void            publishToWorkspace(const QModelIndex &index, const QString &workspace);
        void            publishOrUpdateToWorkspace(const QString &zipFile);
        void            publishPluginToWorkspace();
        void            publishPackageToWorkspace();

        void            uploadPluginPackage();
        void            uploadPluginIcon(QNetworkReply *pReply);
        void            uploadPluginIcon(const QString& strUrl);
        void            uploadDemoWorkflow();

        void            downloadPluginPackage(CPluginModel *pModel, QNetworkReply *pReply);

        void            savePluginFolder(CPluginModel *pModel, QNetworkReply *pReply);

        void            installPythonPluginDependencies(CPluginModel *pModel, const QString& directory, const CTaskInfo &info, const CUser &user);

        void            deletePlugin();
        void            deleteTransferFiles();

        void            clearContext(CPluginModel *pModel, bool bError);

        void            finalyzePublishHub();
        void            finalizePublishWorkspace();
        void            finalizePluginInstall(CPluginModel *pModel, const CTaskInfo &info, const CUser &user);

        void            sendNextPublishInfo(CPluginModel *pModel, QNetworkReply* pReply);

        QString         findBestPackageUrl(const QJsonArray& packages);

    private:

        QNetworkAccessManager*  m_pNetworkMgr = nullptr;
        CProcessManager*        m_pProcessMgr = nullptr;
        CPluginManager*         m_pPluginMgr = nullptr;
        CProgressBarManager*    m_pProgressMgr = nullptr;
        CWorkflowManager*       m_pWorkflowMgr = nullptr;
        CHubDbManager           m_dbMgr;
        CPluginModel            m_hubPluginModel;
        CPluginModel            m_workspacePluginModel;
        CPluginModel            m_localPluginModel;
        std::mutex              m_mutex;
        std::vector<QFile*>     m_transferFiles;
        CUser                   m_currentUser;
        bool                    m_bBusy = false;
};

#endif // CHUBMANAGER_H
