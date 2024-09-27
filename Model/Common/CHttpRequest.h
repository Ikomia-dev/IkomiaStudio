#ifndef CHTTPREQUEST_H
#define CHTTPREQUEST_H

#include <QNetworkRequest>
#include <QHttpPart>
#include "Model/User/CUser.h"

class CHttpRequest : public QNetworkRequest
{
    public:

        CHttpRequest(const QString& url);
        CHttpRequest(const QString& url, const QString& contentType);
        CHttpRequest(const QString& url, const CUser& user);
        CHttpRequest(const QString& url, const QString& contentType, const CUser& user);

        void        setUserAuth(const CUser& user);

        QHttpPart   createFilePart(QFile* pFile, const QString& name, const QString &type);

    private:

        void        setValidUrl(const QString& url);
};

#endif // CHTTPREQUEST_H
