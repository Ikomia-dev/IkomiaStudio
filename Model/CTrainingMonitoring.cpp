#include "CTrainingMonitoring.h"
#include "UtilsTools.hpp"
#include <QtNetwork/QNetworkReply>

CTrainingMonitoring::CTrainingMonitoring(QNetworkAccessManager *pNetworkMgr)
{
    m_pNetworkMgr = pNetworkMgr;

    m_mlflowConfig["host"] = "0.0.0.0";
    m_mlflowConfig["store_uri"] = Utils::MLflow::getBackendStoreURI();
    m_mlflowConfig["artifact_uri"] = Utils::MLflow::getArtifactURI();
    m_mlflowConfig["tracking_uri"] = Utils::MLflow::getTrackingURI();

    m_tensorboardConfig["log_dir"] = Utils::Tensorboard::getLogDirUri();
    m_tensorboardConfig["tracking_uri"] = Utils::Tensorboard::getTrackingURI();
}

void CTrainingMonitoring::setMLflowConfig(const std::string &key, const std::string &value)
{
    m_mlflowConfig[key] = value;
}

void CTrainingMonitoring::setTensorboadConfig(const std::string &key, const std::string &value)
{
    m_tensorboardConfig[key] = value;
}

void CTrainingMonitoring::checkMLflowServer()
{
    assert(m_pNetworkMgr);

    QString trackingUri = QString::fromStdString(m_mlflowConfig.at("tracking_uri"));

    // Create local directory to store MLflow experiments and runs
    Utils::File::createDirectory(m_mlflowConfig.at("store_uri"));

    // Check if server is already running
    QString checkUrl = trackingUri + "/health";
    auto pReply = m_pNetworkMgr->get(QNetworkRequest(QUrl(checkUrl)));

    QEventLoop loop;
    connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    auto statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.toInt() == 200)
    {
        Utils::print(tr("MLflow tracking server is started."), QtInfoMsg);
        return;
    }

    QStringList args;
    QString cmd = "mlflow";
    QString host = QString::fromStdString(m_mlflowConfig.at("host"));
    QString storeUri = QString::fromStdString(m_mlflowConfig.at("store_uri"));
    QString artifactUri = QString::fromStdString(m_mlflowConfig.at("artifact_uri"));

#ifdef Q_OS_WIN
    storeUri.remove(":");
    artifactUri.remove(":");
    storeUri = "file://" + storeUri;
    artifactUri = "file://" + artifactUri;
#endif

    Utils::print("Starting MLflow server...", QtInfoMsg);

    args << "server";
    args << "--backend-store-uri" << storeUri;
    args << "--default-artifact-root" << artifactUri;
    args << "--host" << host;

    auto pProcess = new QProcess(this);
    connect(pProcess, &QProcess::errorOccurred, [&](QProcess::ProcessError error)
    {
        QString msg;
        switch(error)
        {
            case QProcess::FailedToStart:
                msg = tr("Failed to launch MLflow server. Check if the process is already running or if mlflow Python package is correctly installed.");
                break;
            case QProcess::Crashed:
                msg = tr("MLflow server crashed.");
                break;
            case QProcess::Timedout:
                msg = tr("MLflow server do not respond. Process is waiting...");
                break;
            case QProcess::UnknownError:
                msg = tr("MLflow server encountered an unknown error...");
                break;
            default: break;
        }
        Utils::print(msg, QtWarningMsg);
    });
    connect(pProcess, &QProcess::started, [&]
    {
        Utils::print(tr("MLflow server started successfully."), QtInfoMsg);
    });
    pProcess->startDetached(cmd, args);
}

void CTrainingMonitoring::checkTensorboardServer()
{
    assert(m_pNetworkMgr);

    QString logDir = QString::fromStdString(m_tensorboardConfig["log_dir"]);
    QString trackingUri = QString::fromStdString(m_tensorboardConfig["tracking_uri"]);

    // Create local directory to store MLflow experiments and runs
    Utils::File::createDirectory(m_tensorboardConfig["log_dir"]);

    // Check if server is already running
    auto pReply = m_pNetworkMgr->get(QNetworkRequest(QUrl(trackingUri)));
    QEventLoop loop;
    connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    auto statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(statusCode.toInt() == 200)
    {
        Utils::print(tr("Tensorboard tracking server is started."), QtInfoMsg);
        return;
    }

    // Launch server
    Utils::print("Starting Tensorboard server...", QtInfoMsg);
    QStringList args;
    QString cmd = "tensorboard";
    args << "--logdir" << logDir;

    auto pProcess = new QProcess(this);
    connect(pProcess, &QProcess::errorOccurred, [&](QProcess::ProcessError error)
    {
        QString msg;
        switch(error)
        {
            case QProcess::FailedToStart:
                msg = tr("Failed to launch Tensorboard server. Check if the process is already running or if tensorboard Python package is correctly installed.");
                break;
            case QProcess::Crashed:
                msg = tr("Tensorboard server crashed.");
                break;
            case QProcess::Timedout:
                msg = tr("Tensorboard server do not respond. Process is waiting...");
                break;
            case QProcess::UnknownError:
                msg = tr("Tensorboard server encountered an unknown error...");
                break;
            default: break;
        }
        Utils::print(msg, QtWarningMsg);
    });
    connect(pProcess, &QProcess::started, [&]
    {
        Utils::print(tr("Tensorboard server started successfully."), QtInfoMsg);
    });
    pProcess->startDetached(cmd, args);
}
