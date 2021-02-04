#ifndef CSTOREONLINEICONMANAGER_H
#define CSTOREONLINEICONMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include "Model/User/CUser.h"

class CStoreOnlineIconManager: public QObject
{
    Q_OBJECT

    public:

        enum Request { GET_ICON_URL,
                       DOWNLOAD_ICON
                     };

        CStoreOnlineIconManager(QNetworkAccessManager *pNetworkMgr, const CUser &user);

        void    loadIcons(QJsonArray* pPlugins);

    signals:

        void    doIconsLoaded();

    private:

        void    setPluginIconPath(int index, const QString& path);

        QString getPluginIconPath(const QString& name) const;

        bool    isIconExists(const QString& pluginName);

        bool    checkReply(QNetworkReply* pReply);

        void    incrementLoadedIcon();

        void    downloadPluginIcon(int pluginIndex, const QString& iconUrl);

    private slots:

        void    onReplyFinished(QNetworkReply *pReply);
        void    onGetIconUrlDone(QNetworkReply *pReply);
        void    onDownloadIconDone(QNetworkReply *pReply);

    private:

        QNetworkAccessManager*      m_pNetworkMgr = nullptr;
        CUser                       m_currentUser;
        QJsonArray*                 m_pPlugins;
        QMap<QNetworkReply*, int>   m_mapReplyType;
        QMap<QNetworkReply*, int>   m_mapReplyPluginIndex;
        int                         m_nbIcons = 0;
        int                         m_nbLoadedIcons = 0;
};

#endif // CSTOREONLINEICONMANAGER_H
