#include "CHttpRequest.h"

CHttpRequest::CHttpRequest(const QString &url): QNetworkRequest()
{
    setValidUrl(url);
    setRawHeader("User-Agent", "Ikomia Studio");
    setRawHeader("Content-Type", "application/json");
}

CHttpRequest::CHttpRequest(const QString &url, const CUser &user): QNetworkRequest()
{
    setValidUrl(url);
    setRawHeader("User-Agent", "Ikomia Studio");
    setRawHeader("Content-Type", "application/json");
    setUserAuth(user);
}

void CHttpRequest::setUserAuth(const CUser &user)
{
    if (user.isConnected())
        setRawHeader("Authorization", user.getAuthHeader());
}

QHttpPart CHttpRequest::createFilePart(QFile *pFile, const QString &name, const QString& type)
{
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(type));
    QString formData = QString("form-data; name=\"%1\"; filename=\"%2\"").arg(name).arg(pFile->fileName());
    part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(formData));
    part.setHeader(QNetworkRequest::ContentLengthHeader, pFile->size());
    part.setBodyDevice(pFile);
    return part;
}

void CHttpRequest::setValidUrl(const QString &url)
{
    QUrlQuery urlQuery(url);
    QUrl qurl(urlQuery.query());

    if (qurl.isValid() == false)
        throw CException(CoreExCode::INVALID_PARAMETER, qurl.errorString().toStdString(), __func__, __FILE__, __LINE__);

    setUrl(qurl);
}
