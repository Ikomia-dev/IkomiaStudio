// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CMainModel.h"
#include "PythonThread.hpp"
#include "Main/AppTools.hpp"
#include "Main/LogCategory.h"
#include "Model/Matomo/piwiktracker.h"
#include "CLogManager.h"
#include "Model/Common/CTrainingMonitoring.h"

#define TYTI_PYLOGHOOK_USE_BOOST
#include "Main/PyLogHook.hpp"

void logMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    CLogManager::instance().handleMessage(type, context, msg);
}

CMainModel::CMainModel()
{
    //Initialisation is made in the init() method
}

CMainModel::~CMainModel()
{
    CPyEnsureGIL gil;
    m_userMgr.beforeAppClose();
}

CProjectManager* CMainModel::getProjectManager()
{
    return &m_projectMgr;
}

CProcessManager* CMainModel::getProcessManager()
{
    return &m_processMgr;
}

CWorkflowManager* CMainModel::getWorkflowManager()
{
    return &m_workflowMgr;
}

CRenderManager* CMainModel::getRenderManager()
{
    return &m_renderMgr;
}

CProgressBarManager* CMainModel::getProgressManager()
{
    return &m_progressMgr;
}

CGraphicsManager* CMainModel::getGraphicsManager()
{
    return &m_graphicsMgr;
}

CResultManager* CMainModel::getResultManager()
{
    return &m_resultsMgr;
}

CUserManager *CMainModel::getUserManager()
{
    return &m_userMgr;
}

CMainDataManager* CMainModel::getDataManager()
{
    return &m_dataMgr;
}

CHubManager *CMainModel::getHubManager()
{
    return &m_hubMgr;
}

CSettingsManager*CMainModel::getSettingsManager()
{
    return &m_settingsMgr;
}

CPluginManager *CMainModel::getPluginManager()
{
    return &m_pluginMgr;
}

void CMainModel::init()
{
    //Take care of initialization order
    initLogFile();
    initDb();
    initSettingsManager();
    initPython();
    initProjectManager();
    initPluginManager();
    initProcessManager();
    initWorkflowManager();
    initGraphicsManager();
    initResultsManager();
    initRenderManager();
    initDataManager();
    initUserManager();
    initHubManager();
    initMatomo();
    initConnections();
}

void CMainModel::notifyViewShow()
{
    m_processMgr.notifyViewShow();
    m_workflowMgr.notifyViewShow();
    m_graphicsMgr.notifyViewShow();
    m_userMgr.notifyViewShow();
    m_pluginMgr.notifyViewShow();
    m_settingsMgr.notifyViewShow();
}

void CMainModel::initConnections()
{
    connect(&m_userMgr, &CUserManager::doSetCurrentUser, this, &CMainModel::onSetCurrentUser);
}

void CMainModel::onOpenImage(const QModelIndex& index)
{
    Q_UNUSED(index)
}

void CMainModel::onSetCurrentUser(const CUser &user)
{
    //Take care of initialization order
    m_pluginMgr.setCurrentUser(user);
    m_processMgr.setCurrentUser(user);
    m_hubMgr.setCurrentUser(user);
    m_workflowMgr.setCurrentUser(user);
}

void CMainModel::onStartJupyterLab()
{
    QStringList args;
    QString cmd = "jupyter-lab";
    QString notebookDir = QString::fromStdString(Utils::Jupyter::getNotebookDir());
    QString url = QString::fromStdString(Utils::Jupyter::getServerUri());
    auto redirectPolicy = m_networkMgr.redirectPolicy();
    m_networkMgr.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    // Check if server is already running
    auto pReply = m_networkMgr.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    auto statusCode = pReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(statusCode.toInt() == 200)
    {
        m_networkMgr.setRedirectPolicy(redirectPolicy);
        Utils::OS::openUrl(url.toStdString());
        return;
    }

    // Launch server
    args << "--notebook-dir" << notebookDir;

    auto pProcess = new QProcess(this);
    connect(pProcess, &QProcess::errorOccurred, [&](QProcess::ProcessError error)
    {
        QString msg;
        switch(error)
        {
            case QProcess::FailedToStart:
                msg = tr("Failed to start Jupyter server. Check if the process is already running or if jupyterlab Python package is correctly installed.");
                break;
            case QProcess::Crashed:
                msg = tr("JupyterLab server crashed.");
                break;
            case QProcess::Timedout:
                msg = tr("JupyterLab server do not respond. Process is waiting...");
                break;
            case QProcess::UnknownError:
                msg = tr("JupyterLab server encountered an unknown error...");
                break;
            default: break;
        }

        qInfo().noquote() << msg;
    });
    connect(pProcess, &QProcess::started, [&]
    {
        qInfo().noquote() << tr("JupyterLab server started successfully.");
    });
    pProcess->startDetached(cmd, args);
    m_networkMgr.setRedirectPolicy(redirectPolicy);
}

void CMainModel::initLogFile()
{
    m_logFilePath = Utils::IkomiaApp::getIkomiaFolder() + "/log.txt";

    QFile file(QString::fromStdString(m_logFilePath));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;

    // Write all Qt Message in log file
    qInstallMessageHandler(logMessageOutput);
    CLogManager::instance().addOutputManager(std::bind(&CMainModel::writeLogMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Utils::File::createDirectory(Utils::IkomiaApp::getIkomiaFolder() + "/Resources/Tmp");
    CLogManager::instance().setStdRedirection(Utils::IkomiaApp::getQIkomiaFolder() + "/Resources/Tmp/stdout_err.txt");
}

void CMainModel::initDb()
{
    m_dbMgr.init();
}

void CMainModel::initProjectManager()
{
    m_projectMgr.setManagers(&m_graphicsMgr, &m_renderMgr, &m_resultsMgr, &m_workflowMgr, &m_progressMgr, &m_dataMgr);
}

void CMainModel::initProcessManager()
{
    emit doSetSplashMessage(tr("Load process library and plugins..."), Qt::AlignCenter, qApp->palette().highlight().color());
    QCoreApplication::processEvents();
    m_processMgr.setManagers(&m_pluginMgr, &m_workflowMgr);
    m_processMgr.init();
}

void CMainModel::initWorkflowManager()
{
    m_workflowMgr.setManagers(&m_networkMgr, &m_processMgr, &m_projectMgr, &m_graphicsMgr,
                              &m_resultsMgr, &m_dataMgr, &m_progressMgr, &m_settingsMgr);

    CTrainingMonitoring monitor(&m_networkMgr);
    monitor.checkMLflowServer();
    // Disable Tensorboard launch at startup as it causes failure on
    // plugin installation with Tensorboard or tb-nightly dependency.
    // Moreover there is no impact since no C++ train algorithm use
    // Tensorboard for logging. Python-based training algorithm manage
    // server lifecycle thanks to TrainProcess class implementation
    //monitor.checkTensorboardServer();
}

void CMainModel::initGraphicsManager()
{
    m_graphicsMgr.setManagers(&m_projectMgr, &m_workflowMgr);
}

void CMainModel::initResultsManager()
{
    m_resultsMgr.init();
    m_resultsMgr.setManagers(&m_projectMgr, &m_workflowMgr, &m_graphicsMgr, &m_renderMgr, &m_dataMgr, &m_progressMgr);
}

void CMainModel::initRenderManager()
{
    m_renderMgr.setManagers(&m_progressMgr);
}

void CMainModel::initDataManager()
{
    m_dataMgr.setManagers(&m_projectMgr, &m_workflowMgr, &m_graphicsMgr, &m_resultsMgr, &m_renderMgr, &m_progressMgr);
}

void CMainModel::initUserManager()
{
    m_userMgr.init();
    m_userMgr.setManagers(&m_networkMgr);
}

void CMainModel::initHubManager()
{
    m_hubMgr.setManagers(&m_networkMgr, &m_processMgr, &m_pluginMgr, &m_progressMgr, &m_workflowMgr);
}

void CMainModel::initSettingsManager()
{
    // init settings manager
    m_settingsMgr.init();
}

void CMainModel::initPluginManager()
{
    m_pluginMgr.setRegistry(&m_processMgr.m_registry);
}

void CMainModel::initPython()
{
    checkUserInstall();

    emit doSetSplashMessage(tr("Configure Python environment..."), Qt::AlignCenter, qApp->palette().highlight().color());
    QCoreApplication::processEvents();
    std::string pythonExe = Utils::Python::getPythonInterpreterPath();
    std::string pluginsPath = Utils::IkomiaApp::getIkomiaFolder() + "/Plugins/Python";

#if defined(Q_OS_LINUX)
    std::string delimiter = ":";
#elif defined(Q_OS_WIN64)
    std::string delimiter = ";";
#endif

    std::string modulePath = Utils::Python::getPythonLibPath() + delimiter;
    modulePath += Utils::Python::getPythonDynLoadPath() + delimiter;
    modulePath += Utils::Python::getPythonSitePackagesPath() + delimiter;
    // API dev folder: optional if Ikomia dev package has been installed into the venv (pip install -e .)
    modulePath += Utils::Python::getPythonIkomiaApiFolder() + delimiter;
    modulePath += pluginsPath;

    // Add python bin path to $PATH (for tools like MLflow)
    QString pathenv = qEnvironmentVariable("PATH");
    pathenv = QString::fromStdString(Utils::Python::getPythonBinPath() + delimiter) + pathenv;
    qputenv("PATH", pathenv.toUtf8());

    // Embedded Python executable
#ifdef PY37
    // Python 3.7 C API
    auto pythonExeSize = pythonExe.size();
    Py_SetProgramName(Py_DecodeLocale(pythonExe.c_str(), &pythonExeSize));

    // Set Python path
    auto modulePathSize = modulePath.size();
    Py_SetPath(Py_DecodeLocale(modulePath.c_str(), &modulePathSize));

    //Initialize Python interpreter
    Py_Initialize();
#else
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    status = PyConfig_SetBytesString(&config, &config.program_name, pythonExe.c_str());
    if (PyStatus_Exception(status))
    {
        qCritical() << "Failed to set Python program name";
        PyConfig_Clear(&config);
        return;
    }

    status = PyConfig_SetBytesString(&config, &config.pythonpath_env, modulePath.c_str());
    if (PyStatus_Exception(status))
    {
        qCritical() << "Failed to set Python path";
        PyConfig_Clear(&config);
        return;
    }

    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status))
    {
        qCritical() << "Failed to initialize Python";
        PyConfig_Clear(&config);
        return;
    }
    PyConfig_Clear(&config);
#endif

    try
    {
        // Set multiprocessing executable to launch python interpreter
        // for subprocess instead of Ikomia App...
        boost::python::object main_module = boost::python::import("__main__");
        boost::python::object main_namespace = main_module.attr("__dict__");
        boost::python::object multiprocessing = boost::python::import("multiprocessing");
        multiprocessing.attr("set_executable")(pythonExe);
        main_namespace["multiprocessing"] = multiprocessing;
    }
    catch(boost::python::error_already_set&)
    {
        qCritical() << QString::fromStdString(Utils::Python::handlePythonException());
    }

    //Set sys.stdout and sys.stderr redirection
    tyti::pylog::redirect_stdout([&](const char* w){ qCInfo(logPython).noquote() << QString(w); });
    tyti::pylog::redirect_stderr([&](const char* w){ qCCritical(logPython).noquote() << QString(w); });

    //Release GIL
    PyThreadState* st = PyEval_SaveThread();
    Q_UNUSED(st);
}

void CMainModel::initMatomo()
{
    PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getPiwikUrl()), _piwik_site_id);
    pPiwikTracker->sendEvent("main", "general", "start", "application-started");
}

void CMainModel::writeLogMsg(int type, const QString& msg, const QString& categoryName)
{
    Q_UNUSED(categoryName);
    Q_UNUSED(type);

    QFile file(QString::fromStdString(m_logFilePath));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    auto date = QDate::currentDate();
    auto time = QTime::currentTime();
    QString str = date.toString(Qt::ISODate) + " " + time.toString(Qt::ISODate) + ": " + msg;

    QTextStream out(&file);
    out << str << "\n";
    file.close();
}

void CMainModel::checkUserInstall()
{
    QString appFolder = Utils::IkomiaApp::getQIkomiaFolder();

#if defined(Q_OS_WIN64)
    QString srcPythonFolder = QCoreApplication::applicationDirPath() + "/Python";
    QString srcSitePackagesFolder = QCoreApplication::applicationDirPath() + "/Python/lib/site-packages";
    QString userSitePackagesFolder = appFolder + "/Python/lib/site-packages";
    QString srcApiFolder = QCoreApplication::applicationDirPath() + "/Api";
    QString srcResourcesFolder = QCoreApplication::applicationDirPath() + "/Resources";
#elif defined(Q_OS_LINUX)
    QString srcPythonFolder = "/opt/Ikomia/Python";
    QString srcSitePackagesFolder = "/opt/Ikomia/Python/lib/python" + QString::fromStdString(Utils::Python::getPythonVersion()) + "/site-packages";
    QString userSitePackagesFolder = appFolder + "/Python/lib/python" + QString::fromStdString(Utils::Python::getPythonVersion()) + "/site-packages";
    QString srcApiFolder = "/opt/Ikomia/Api";
    QString srcResourcesFolder = "/opt/Resources";
#endif

    //Python
    QString userPythonFolder = appFolder + "/Python";
    if (QDir(srcPythonFolder).exists() && QDir(userPythonFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Python environment...\n(Please be patient, this may take a while)"), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();

        //Copy Python directory
        Utils::File::copyDirectory(srcPythonFolder, userPythonFolder, true);
        //Install Python required packages
        installPythonRequirements();
    }
    else if (QDir(srcSitePackagesFolder).exists() && QDir(userSitePackagesFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Python environment...\n(Please be patient, this may take a while)"), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();

        //Copy Python directory
        Utils::File::copyDirectory(srcSitePackagesFolder, userSitePackagesFolder, true);
        //Install Python required packages
        installPythonRequirements();
    }

    //Copy Ikomia API
    QString userApiFolder = appFolder + "/Api";
    if (QDir(srcApiFolder).exists() && QDir(userApiFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Ikomia API..."), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();
        Utils::File::copyDirectory(srcApiFolder, userApiFolder, true);
    }

    // Copy Resources
    QString userResourcesFolder = appFolder + "/Resources";
    if (QDir(srcResourcesFolder).exists() && QDir(userResourcesFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Ikomia Resources..."), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();
        Utils::File::copyDirectory(srcResourcesFolder, userResourcesFolder, true);
    }

    QString userGmicFolder = Utils::IkomiaApp::getGmicFolder();
    if (QDir(srcResourcesFolder).exists() && QDir(userGmicFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Gmic resources..."), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();
        Utils::File::copyDirectory(srcResourcesFolder + "/gmic", userGmicFolder, true);
    }
}

void CMainModel::installPythonRequirements()
{
    emit doSetSplashMessage(tr("Install Python packages...\n(Please be patient, this may take a while)"), Qt::AlignCenter, qApp->palette().highlight().color());
    QCoreApplication::processEvents();

    QString userPythonFolder = Utils::IkomiaApp::getQIkomiaFolder() + "/Python";
    QString requirementsPath = userPythonFolder + "/requirements.txt";

    if (QFile::exists(requirementsPath))
        Utils::Python::installRequirements(requirementsPath);
}
