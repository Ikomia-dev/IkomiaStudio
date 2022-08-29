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
#include "Model/Matomo/piwiktracker.h"
#include "CLogManager.h"
#include "CTrainingMonitoring.h"

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
    return &m_protocolMgr;
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

CStoreManager *CMainModel::getStoreManager()
{
    return &m_storeMgr;
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
    initStoreManager();
    initMatomo();
    initConnections();
}

void CMainModel::notifyViewShow()
{
    m_processMgr.notifyViewShow();
    m_protocolMgr.notifyViewShow();
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
    m_storeMgr.setCurrentUser(user);
    m_protocolMgr.setCurrentUser(user);
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
    m_projectMgr.setManagers(&m_graphicsMgr, &m_renderMgr, &m_resultsMgr, &m_protocolMgr, &m_progressMgr, &m_dataMgr);
}

void CMainModel::initProcessManager()
{
    emit doSetSplashMessage(tr("Load process library and plugins..."), Qt::AlignCenter, qApp->palette().highlight().color());
    QCoreApplication::processEvents();
    m_processMgr.setManagers(&m_pluginMgr, &m_protocolMgr);
    m_processMgr.init();
}

void CMainModel::initWorkflowManager()
{
    m_protocolMgr.setManagers(&m_processMgr, &m_projectMgr, &m_graphicsMgr,
                              &m_resultsMgr, &m_dataMgr, &m_progressMgr, &m_settingsMgr);

    CTrainingMonitoring monitor(&m_networkMgr);
    monitor.checkMLflowServer();
    monitor.checkTensorboardServer();
}

void CMainModel::initGraphicsManager()
{
    m_graphicsMgr.setManagers(&m_projectMgr, &m_protocolMgr);
}

void CMainModel::initResultsManager()
{
    m_resultsMgr.init();
    m_resultsMgr.setManagers(&m_projectMgr, &m_protocolMgr, &m_graphicsMgr, &m_renderMgr, &m_dataMgr, &m_progressMgr);
}

void CMainModel::initRenderManager()
{
    m_renderMgr.setManagers(&m_progressMgr);
}

void CMainModel::initDataManager()
{
    m_dataMgr.setManagers(&m_projectMgr, &m_protocolMgr, &m_graphicsMgr, &m_resultsMgr, &m_renderMgr, &m_progressMgr);
}

void CMainModel::initUserManager()
{
    m_userMgr.init();
    m_userMgr.setManagers(&m_networkMgr);
}

void CMainModel::initStoreManager()
{
    m_storeMgr.setManagers(&m_networkMgr, &m_processMgr, &m_pluginMgr, &m_progressMgr);
}

void CMainModel::initSettingsManager()
{
    // init settings manager
    m_settingsMgr.init();
}

void CMainModel::initPluginManager()
{
    m_pluginMgr.setRegistrator(&m_processMgr.m_processRegistrator);
}

void CMainModel::initPython()
{
    checkUserInstall();

    emit doSetSplashMessage(tr("Configure Python environment..."), Qt::AlignCenter, qApp->palette().highlight().color());
    QCoreApplication::processEvents();
    QString pythonPath = Utils::IkomiaApp::getQIkomiaFolder() + "/Python";
    std::string pythonExe;
    std::string pythonLib;
    std::string pythonDynload;
    std::string pythonSitePackages;
    std::string ikomiaApi;
    std::string pluginsPath = Utils::IkomiaApp::getIkomiaFolder() + "/Plugins/Python";

    // Set program if existing
    QDir pythonDir(pythonPath);
    if(pythonDir.exists())
    {
        //Define embedded Python paths
#if defined(Q_OS_MACOS)
        QString delimiter = ":";
        pythonExe = pythonPath.toStdString() + "/bin/python" + Utils::Python::_python_bin_prod_version;
        pythonLib = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + ":";
        pythonDynload = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + "/lib-dynload:";
        pythonSitePackages = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + "/site-packages:";
        pluginsPath += ":";
#elif defined(Q_OS_LINUX)
        QString delimiter = ":";
        pythonExe = pythonPath.toStdString() + "/bin/python" + Utils::Python::_python_bin_prod_version;
        pythonLib = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + ":";
        pythonDynload = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + "/lib-dynload:";
        pythonSitePackages = pythonPath.toStdString() + "/lib/python" + Utils::Python::_python_lib_prod_version + "/site-packages:";
        pluginsPath += ":";
#elif defined(Q_OS_WIN64)
        QString delimiter = ";";
        pythonExe = pythonPath.toStdString() + "/python.exe";
        pythonLib = pythonPath.toStdString() + "/lib;";
        pythonDynload = pythonPath.toStdString() + "/DLLs;";
        pythonSitePackages = pythonPath.toStdString() + "/lib/site-packages;";
        pluginsPath += ";";
#endif
        //Embedded Python executable
        auto s = pythonExe.size();
        Py_SetProgramName(Py_DecodeLocale(pythonExe.c_str(), &s));

        //Set Python path
        std::string modulePath = pythonLib;
        modulePath += pythonDynload;
        modulePath += pythonSitePackages;
        modulePath += pluginsPath;

#ifndef QT_DEBUG
        modulePath += Utils::IkomiaApp::getIkomiaFolder() + "/Api";
#else
    #ifdef Q_OS_WIN64
        modulePath += "C:/Developpement/Ikomia/Build/Lib/Python;";
    #else
        modulePath += QDir::homePath().toStdString() + "/Developpement/IkomiaApi:";
    #endif
#endif

        auto sp = modulePath.size();
        Py_SetPath(Py_DecodeLocale(modulePath.c_str(), &sp));

        //Add $HOME/python/bin to $PATH
        QString pathenv = qEnvironmentVariable("PATH");
        pathenv = pythonPath + "/bin" + delimiter + pathenv;
        qputenv("PATH", pathenv.toUtf8());
    }
    else
    {
        qWarning().noquote() << "Embedded Python not found:" << pythonPath;

#if defined(Q_OS_MACOS)
        pythonExe = "/bin/python" + Utils::Python::_python_bin_prod_version;
        pythonLib = "/lib/python" + Utils::Python::_python_lib_prod_version + ":";
        pythonDynload = "/lib/python" + Utils::Python::_python_lib_prod_version + "/lib-dynload:";
        pythonSitePackages = "/lib/python" + Utils::Python::_python_lib_prod_version + "/site-packages:";
        ikomiaApi = QDir::homePath().toStdString() + "/Developpement/IkomiaApi/Build/Lib/Python:";
#elif defined(Q_OS_LINUX)
        pythonExe = "/usr/bin/python" + Utils::Python::getDevBinVersion();
        pythonLib = ":/usr/lib/python" + Utils::Python::getDevLibVersion() + ":";
        pythonDynload = "/usr/lib/python" + Utils::Python::getDevLibVersion() + "/lib-dynload:";
        pythonSitePackages = "/usr/lib/python" + Utils::Python::getDevLibVersion() + "/site-packages:";
        pythonSitePackages += "/usr/local/lib/python" + Utils::Python::getDevLibVersion() + "/dist-packages:";
        pythonSitePackages += "/usr/lib/python3/dist-packages:";
        ikomiaApi = QDir::homePath().toStdString() + "/Developpement/IkomiaApi:";
#elif defined(Q_OS_WIN64)
        std::string programFilesPath = getenv("PROGRAMFILES");
        std::string pythonFolder = programFilesPath + "/Python" + Utils::Python::getDevBinVersion();
        pythonExe = pythonFolder + "/python.exe";
        pythonLib = ";" + pythonFolder + "/Lib;";
        pythonDynload = pythonFolder + "/DLLs;";
        pythonSitePackages = pythonFolder + "/Lib/site-packages;";
        ikomiaApi = "C:/Developpement/IkomiaApi/Build/Lib/Python;";
#endif
        auto s = pythonExe.size();
        Py_SetProgramName(Py_DecodeLocale(pythonExe.c_str(), &s));

        //Set Python path
        std::string modulePath = pythonExe;
        modulePath += pythonLib;
        modulePath += pythonDynload;
        modulePath += pythonSitePackages;
        modulePath += ikomiaApi;
        modulePath += pluginsPath;
        auto sp = modulePath.size();
        Py_SetPath(Py_DecodeLocale(modulePath.c_str(), &sp));
    }

    //Initialize Python interpreter
    Py_Initialize();

    // Set multiprocessing executable to launch python interpreter
    // for subprocess instead of Ikomia App...
    try
    {
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

    //Threading
    if(PyEval_ThreadsInitialized() == 0)
        PyEval_InitThreads();

    //Release GIL
    PyThreadState* st = PyEval_SaveThread();
    Q_UNUSED(st);

    //Set sys.stdout and sys.stderr redirection
    CPyEnsureGIL gil;
    tyti::pylog::redirect_stdout([&](const char* w){ qInfo().noquote() << QString(w); });
    tyti::pylog::redirect_stderr([&](const char* w){ qCritical().noquote() << QString(w); });
}

void CMainModel::initMatomo()
{
    PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getMatomoUrl()), MATOMO_APP_ID);
    pPiwikTracker->sendVisit("main", "Application_Started");
}

void CMainModel::writeLogMsg(int type, const QString& msg, const QString& categoryName)
{
    Q_UNUSED(categoryName);

    QFile file(QString::fromStdString(m_logFilePath));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QString msgType;
    switch(type)
    {
        case QtDebugMsg: msgType = "DEBUG:"; break;
        case QtInfoMsg: msgType = "INFO:"; break;
        case QtWarningMsg: msgType = "WARNING:"; break;
        case QtCriticalMsg: msgType = "CRITICAL:"; break;
        case QtFatalMsg: msgType = "FATAL:"; break;
        default: msgType = "INFO:"; break;
    }

    auto date = QDate::currentDate();
    auto time = QTime::currentTime();
    QString str = msgType + date.toString() + " " + time.toString() + ": " + msg;

    QTextStream out(&file);
    out << str << "\n";
    file.close();
}

void CMainModel::checkUserInstall()
{
#if defined(Q_OS_WIN64)
    QString srcPythonFolder = QCoreApplication::applicationDirPath() + "/Python";
    QString srcApiFolder = QCoreApplication::applicationDirPath() + "/Api";
    QString srcResourcesFolder = QCoreApplication::applicationDirPath() + "/Resources";
#elif defined(Q_OS_LINUX)
    QString srcPythonFolder = "/opt/Ikomia/Python";
    QString srcApiFolder = "/opt/Ikomia/Api";
    QString srcResourcesFolder = "/opt/Resources";
#elif defined(Q_OS_MACOS)
    QString srcPythonFolder = "/usr/local/Ikomia/Python";
    QString srcApiFolder = "/usr/local/Ikomia/Api";
    QString srcResourcesFolder = "/usr/local/Resources";
#endif

    //Python
    QString appFolder = Utils::IkomiaApp::getQIkomiaFolder();
    QString userPythonFolder = appFolder + "/Python";

    if(QDir(userPythonFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Python environment...\n(Please be patient, this may take a while)"), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();

        //Copy Python directory
        Utils::File::copyDirectory(srcPythonFolder, userPythonFolder, true);
        //Install Python required packages
        installPythonRequirements();
    }

    //Copy Ikomia API
    QString userApiFolder = appFolder + "/Api";
    if(QDir(userApiFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Ikomia API..."), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();
        Utils::File::copyDirectory(srcApiFolder, userApiFolder, true);
    }

    // Copy Resources
    QString userResourcesFolder = appFolder + "/Resources";
    if(QDir(userResourcesFolder).exists() == false)
    {
        emit doSetSplashMessage(tr("Install Ikomia Resources..."), Qt::AlignCenter, qApp->palette().highlight().color());
        QCoreApplication::processEvents();
        Utils::File::copyDirectory(srcResourcesFolder, userResourcesFolder, true);
    }

    QString userGmicFolder = Utils::IkomiaApp::getGmicFolder();
    if(QDir(userGmicFolder).exists() == false)
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
