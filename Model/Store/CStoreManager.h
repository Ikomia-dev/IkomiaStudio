#ifndef CSTOREMANAGER_H
#define CSTOREMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "CStoreDbManager.h"
#include "Model/Store/CStoreQueryModel.h"
#include "Model/User/CUser.h"

class QNetworkAccessManager;
class CProcessManager;
class CPluginManager;
class CProgressBarManager;

class CStoreManager : public QObject
{
    Q_OBJECT

    enum Request { GET_PLUGINS,
                   GET_PACKAGE_URL,
                   PUBLISH_PLUGIN,
                   UPDATE_PLUGIN,
                   UPLOAD_PACKAGE,
                   UPLOAD_ICON,
                   DOWNLOAD_PACKAGE,
                   DELETE_PLUGIN
                 };

    public:

        CStoreManager();

        void            setManagers(QNetworkAccessManager *pNetworkMgr, CProcessManager* pProcessMgr, CPluginManager* pPluginMgr, CProgressBarManager *pProgressMgr);
        void            setCurrentUser(const CUser& user);

    signals:

        void            doSetServerPluginModel(CStoreQueryModel* pModel);
        void            doSetLocalPluginModel(CStoreQueryModel* pModel);

        void            doRestartIkomia();

    public slots:

        void            onRequestServerModel();
        void            onRequestLocalModel();
        void            onPublishPlugin(const QModelIndex& index);
        void            onInstallPlugin(const QModelIndex& index);
        void            onUpdatePluginInfo(bool bFullEdit, const CProcessInfo& info);
        void            onServerSearchChanged(const QString& text);
        void            onLocalSearchChanged(const QString& text);

    private slots:

        void            onGetPlugins();
        void            onPublishPluginDone();
        void            onPluginCompressionDone(const QString &zipFile);
        void            onUpdatePluginDone();
        void            onUploadPackageDone();
        void            onUploadIconDone();
        void            onGetPackageUrlDone();
        void            onDownloadPackageDone();
        void            onPluginExtractionDone(const QStringList &files, const QString dstDir);
        void            onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
        void            onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void            onDeletePlugin();

    private:

        void            createServerPluginModel();
        void            createServerQueryModel();
        void            createLocalPluginModel();
        QByteArray      createPluginJson();

        QNetworkReply*  checkReply(int type) const;
        QString         checkPythonPluginDirectory(const QString &directory);
        QString         checkCppPluginDirectory(const QString &directory, const QString& name);
        void            checkPendingUpdates();

        void            updateServerPlugin();
        void            updateLocalPlugin();

        void            fillServerPluginModel(QNetworkReply* pReply);

        void            validateServerPluginModel();

        void            generateZipFile();
        void            extractZipFile(const QString &src, const QString &dstDir);

        void            publishPluginToServer();

        void            uploadPluginPackage();
        void            uploadPluginIcon();

        void            downloadPluginPackage(const QString& packageUrl);

        void            installPythonPluginDependencies(const QString& directory);

        void            deletePlugin();
        void            deleteTranferFile();

        void            clearContext();

    private:

        QNetworkAccessManager*      m_pNetworkMgr = nullptr;
        CProcessManager*            m_pProcessMgr = nullptr;
        CPluginManager*             m_pPluginMgr = nullptr;
        CProgressBarManager*        m_pProgressMgr = nullptr;
        CProgressSignalHandler      m_progressSignal;
        QJsonArray                  m_jsonServerPlugins;
        QString                     m_currentPluginPackageFile;
        QMap<int, QNetworkReply*>   m_mapTypeRequest;
        CStoreDbManager             m_dbMgr;
        CStoreQueryModel*           m_pServerPluginModel = nullptr;
        CStoreQueryModel*           m_pLocalPluginModel = nullptr;
        QFile*                      m_pTranferFile = nullptr;
        QModelIndex                 m_currentServerIndex = QModelIndex();
        QModelIndex                 m_currentLocalIndex = QModelIndex();
        CUser                       m_currentUser;
        int                         m_currentPluginServerId = -1;
        bool                        m_bDownloadStarted = false;
        bool                        m_bBusy = false;
};

#endif // CSTOREMANAGER_H
