/*
 *  Copyright (C) 2009 Aleksey Palazhchenko
 *  Copyright (C) 2014 Sergey Shambir
 *  Copyright (C) 2016 Alexander Makarov
 *
 * This file is a part of Breakpad-qt library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */
 
#include <QCoreApplication>
#include <QString>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "QBreakpadHttpUploader.h"
#include "UtilsTools.hpp"

QBreakpadHttpUploader::QBreakpadHttpUploader(QObject *parent) :
    QObject(parent),
    m_file(0)
{

}

QBreakpadHttpUploader::QBreakpadHttpUploader(const QUrl &url, QObject *parent) :
    QObject(parent),
    m_file(0)
{
    m_request.setUrl(url);
}

QBreakpadHttpUploader::~QBreakpadHttpUploader()
{
	if(m_reply) {
        qWarning().noquote() << "m_reply is not NULL";
		m_reply->deleteLater();
	}

	delete m_file;
}

QString QBreakpadHttpUploader::remoteUrl() const
{
    return m_request.url().toString();
}

void QBreakpadHttpUploader::setUrl(const QUrl &url)
{
    m_request.setUrl(url);
}

void QBreakpadHttpUploader::uploadDump(const QString& abs_file_path)
{
    Q_ASSERT(QDir().exists(abs_file_path));
    QFileInfo fileInfo(abs_file_path);
    QString appName, appVer;

    appName = qApp->applicationName();
    appVer = qApp->applicationVersion();

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //product name parameter
    QHttpPart prodPart;
#if defined(SOCORRO)
    prodPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"prod\""));      // Socorro ?ProductName
#elif defined(CALIPER)
    prodPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"prod\""));     // Caliper
#endif
    prodPart.setBody(appName.toLocal8Bit());
    //product version parameter
    QHttpPart verPart;
#if defined(SOCORRO)
    verPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"ver\""));      // Socorro ?Version
#elif defined(CALIPER)
    verPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"ver\""));     // Caliper
#endif
    verPart.setBody(appVer.toLocal8Bit());

    // file_minidump name & file_binary in one part
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upload_file_minidump\"; filename=\""+ fileInfo.fileName()+ "\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));

    if(m_file)
        m_file->deleteLater();

    m_file = new QFile(abs_file_path);
    if(!m_file->open(QIODevice::ReadOnly)) return;

    filePart.setBodyDevice(m_file);
    m_file->setParent(multiPart);

    QHttpPart logPart;
    logPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"log\"; filename=\"log.txt\""));

    QFile* file = new QFile(Utils::IkomiaApp::getQIkomiaFolder() + "/log.txt");
    if(!file->open(QIODevice::ReadOnly)) return;

    logPart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(prodPart);
    multiPart->append(verPart);
    multiPart->append(filePart);
    multiPart->append(logPart);

    m_request.setRawHeader("User-Agent", appName.toLocal8Bit()+"/"+appVer.toLocal8Bit());
#if defined(SOCORRO)
    m_request.setRawHeader("Host", appName.toLower().toLocal8Bit()+"_reports");
    m_request.setRawHeader("Accept", "*/*");
#endif

    if(m_reply)
    {
        m_reply->close();
        m_reply->deleteLater();
    }

    m_reply = m_manager.post(m_request, multiPart);
    multiPart->setParent(m_reply);

    connect(m_reply, SIGNAL(uploadProgress(qint64, qint64)),
            this,      SLOT(onUploadProgress(qint64,qint64)));

    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this,      SLOT(onError(QNetworkReply::NetworkError)));

    connect(m_reply, SIGNAL(finished()),
            this,      SLOT(onUploadFinished()));

    QEventLoop loop;
    connect(m_reply, SIGNAL(finished()),
            &loop,      SLOT(quit()));
    loop.exec();
}

void QBreakpadHttpUploader::onUploadProgress(qint64 sent, qint64 total)
{
    emit uploadProcess(sent, total);
}

void QBreakpadHttpUploader::onError(QNetworkReply::NetworkError err)
{
    qDebug() << err;
}

void QBreakpadHttpUploader::onUploadFinished()
{
    QString data = (QString)m_reply->readAll();

    if(m_reply->error() != QNetworkReply::NoError)
    {
        emit error(QString(tr("Upload error: %1 - %2")).arg(m_reply->error()).arg(m_reply->errorString()));
    }
    else
    {
        qDebug() << "Upload to " << remoteUrl() << " success!";
        m_file->remove();
        emit finished(data);
	}

    if(m_file)
    {
        delete m_file;
        m_file = nullptr;
    }

    if(m_reply)
    {
        m_reply->close();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}
