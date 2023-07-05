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

#include "Controller/CMainCtrl.h"
#include "Model/Wizard/CWizardManager.h"
#include "VolumeRender.h"
#include <QApplication>
#include <QColorDialog>
#include <QTextBlock>
#include <clocale>
#include <QtOpenGL>
#include "Main/AppTools.hpp"
#include "View/Common/CCrashReporDlg.h"
#include "Model/Crash/QBreakpadHandler.h"
#include "Model/Matomo/piwiktracker.h"

inline int myErrorHandler(int /*status*/, const char* /*func_name*/, const char* /*err_msg*/,
                   const char* /*file_name*/, int /*line*/, void*)
{
    // Do whatever you want here
    return 0;
}

int main(int argc, char *argv[])
{
    const QColor orange(255,122,0,255);

#ifdef Q_OS_MACOS
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    qInfo().noquote() << "3D rendering needs OpenGL 3.3 minimum...";
    QSurfaceFormat::setDefaultFormat(format);
#endif

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication app(argc, argv);

    // Set thread pool stack size to 8Mb for OpenBlas compatibility (Python module)
    // Linux is already at 8 Mb but OSX is at 512 kb
    QThreadPool::globalInstance()->setStackSize(8*1024*1024);

    // Meta registration
    qRegisterMetaType<WorkflowVertex>("WorkflowVertex");
    qRegisterMetaType<CWorkflowTask::State>("CWorkflowTask::State");
    qRegisterMetaType<QVector<int>>();
    qRegisterMetaType<QTextBlock>("QTextBlock");
    qRegisterMetaType<QTextCursor>("QTextCursor");
    qRegisterMetaType<CMat>("CMat");

    //Set locale LC_Numeric to force . as decimal separator
    std::setlocale(LC_NUMERIC, "C");
    auto locale = QLocale();

    // Translation - .qm files must be in the same directory as the executable
    QTranslator appTranslator;
    if(appTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "ikomia", "_"))
        app.installTranslator(&appTranslator);

    QTranslator utilsTranslator;
    if(utilsTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "utils", "_"))
        app.installTranslator(&utilsTranslator);

    QTranslator coreTranslator;
    if(coreTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "core", "_"))
        app.installTranslator(&coreTranslator);

    QTranslator dataioTranslator;
    if(dataioTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "dataio", "_"))
        app.installTranslator(&dataioTranslator);

    QTranslator datamanagementTranslator;
    if(datamanagementTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "datamanagement", "_"))
        app.installTranslator(&datamanagementTranslator);

    QTranslator dataplotTranslator;
    if(dataplotTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "dataplot", "_"))
        app.installTranslator(&dataplotTranslator);

    QTranslator dataprocessTranslator;
    if(dataprocessTranslator.load(locale, Utils::IkomiaApp::getTranslationsFolder() + "dataprocess", "_"))
        app.installTranslator(&dataprocessTranslator);

    QTranslator volumerenderTranslator;
    if(volumerenderTranslator.load(locale,Utils::IkomiaApp::getTranslationsFolder() +  "volumerender", "_"))
        app.installTranslator(&volumerenderTranslator);

    // Create Splash screen
    QPixmap pixmap(":/Images/splash.png");
    QSplashScreen *pSplash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    pSplash->show();

    // Show splash message at center
    pSplash->showMessage(QObject::tr("Welcome to Ikomia Studio"), Qt::AlignCenter, orange);
    app.processEvents();

    // OpenCV redirectoring error (assert)
    cv::redirectError(myErrorHandler); ///< Call this once somewhere
    // OpenCV restoring default behavior
    //cv::redirectError(cv::ErrorCallback())

    // Setup App global font properties
#ifdef Q_OS_WIN
    QFontDatabase::addApplicationFont(":/Fonts/SourceSansPro-Regular.ttf");
#else
    //QFontDatabase::addApplicationFont(":/Fonts/SourceSansPro-Regular.otf");
#endif

    QFont font = app.font();
    auto ptSize = font.pointSize();
    font.setFamily("source sans pro");
    font.setPointSize(ptSize);
    QApplication::setFont(font);

    // QSettings
    QApplication::setWindowIcon(QIcon(":/Images/app.ico"));
    QApplication::setOrganizationName("Ikomia");
    QApplication::setOrganizationDomain("ikomia.ai");
    QApplication::setApplicationVersion(Utils::IkomiaApp::getCurrentVersionName());
    QApplication::setApplicationName("Ikomia Studio");

    // Instantiate CMainView first to have initialization messages in the notification center
    CMainView view;

    // Manage if Ikomia has crashed during last session
    QBreakpadInstance.setDumpPath(Utils::IkomiaApp::getQIkomiaFolder() + "/BugReports");
    QBreakpadInstance.setUploadUrl(QUrl(Utils::Network::getBreakPadServerUrl()));

    auto dumpList = QBreakpadInstance.dumpFileList();
    if(!dumpList.empty())
    {
        CCrashReporDlg dlg;
        int ret = dlg.exec();
        if(ret == QDialog::Accepted)
        {
            PiwikTracker* pPiwikTracker = new PiwikTracker(qApp, QUrl(Utils::Network::getMatomoUrl()), MATOMO_APP_ID);
            pPiwikTracker->sendVisit("main", "Application_Crashed");
        }
    }

    // Instantiate CMainModel et CMainCtrl
    CMainModel model;
    CMainCtrl ctrl(&model, &view, pSplash);

    // Finish splash screen when CMainView is ready to show
    pSplash->finish(&view);

    // Show CMainView and notify that it is shown
    ctrl.show();

    return app.exec();
}
