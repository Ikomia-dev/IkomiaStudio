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

/**
 * @file      CMainView.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CMainView
 *
 * @details   Details
 */
#include "CMainView.h"
#include "Main/AppTools.hpp"
#include "View/Hub/CHubDlg.h"
#include "View/Process/CProcessPopupDlg.h"
#include "Model/Project/CProjectViewProxyModel.h"
#include "CLogManager.h"

bool CMainView::bUseOpenGL_330;

CMainView::CMainView(QWidget *parent) : QMainWindow(parent), ui(new Ui::CMainView)
{
    setAcceptDrops(true);

    // Disable default title bar
#ifdef Q_OS_MACOS
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
#else
    setWindowFlags(Qt::FramelessWindowHint);

    // Create a small class for moving main window
    m_pMainwViewMover = std::make_unique<CFramelessHelper>();
    m_pMainwViewMover->activateOn(this);
    m_pMainwViewMover->setWidgetMovable(false);
    m_pMainwViewMover->setWidgetResizable(false);
#endif

    ui->setupUi(this);
    setAppStyle("Classic");

    // Read QSettings for the app
    readSettings();

    // Init all widgets
    init();

    // Display all Qt Message in Notification Center
    CLogManager::instance().addOutputManager([&](int type, const QString& msg, const QString& categoryName){
        emit doDisplayLogMsg(type, msg, categoryName);
    });

    statusBar()->showMessage(tr("Ready!"));
}

CMainView::~CMainView()
{
    delete ui;
}

void CMainView::setCustomGeometry(const QRect& rect)
{
    m_oldGeometry = rect;
}

CProjectPane *CMainView::getProjectPane()
{
    return m_pProjectPane;
}

CDoubleView *CMainView::getDoubleView()
{
    return m_pDoubleView;
}

CProcessPane *CMainView::getProcessPane()
{
    return m_pProcessPane;
}

CWorkflowPane *CMainView::getWorkflowPane()
{
    return m_pWorkflowPane;
}

CInfoPane* CMainView::getInfoPane()
{
    return m_pInfoPane;
}

CWizardPane*CMainView::getWizardPane()
{
    return m_pWizardPane;
}

CWorkflowModuleWidget *CMainView::getWorkflowModule()
{
    return m_pWorkflowModule;
}

CGraphicsToolbar *CMainView::getGraphicsToolbar() const
{
    return m_pGraphicsToolbar;
}

QSystemTrayIcon*CMainView::getNotifier()
{
    return m_pNotifier;
}

CNotificationPane* CMainView::getNotificationCenter()
{
    return m_pNotificationPane;
}

QAction* CMainView::getMainToolBarBtn(int id)
{
    QAction* pAct = nullptr;
    switch(id)
    {
        case BTN_OPEN_PROJECT:
            pAct = m_pOpenProjectAct;
            break;
        case BTN_OPEN_IMG:
            pAct = m_pOpenImgAct;
            break;
        case BTN_OPEN_VIDEO:
            pAct = m_pOpenVideoAct;
            break;
        case BTN_OPEN_STREAM:
            pAct = m_pOpenCamAct;
            break;
        case BTN_OPEN_HUB:
            pAct = m_pOpenHubAct;
            break;
    }

    return pAct;
}

QToolButton* CMainView::getBtn(int id)
{
    QToolButton* pBtn = nullptr;
    switch(id)
    {
        // UP BTN
        case BTN_OPEN_PROJECT:
        case BTN_OPEN_IMG:
        case BTN_OPEN_VIDEO:
        case BTN_OPEN_STREAM:
        case BTN_OPEN_HUB:
            pBtn = static_cast<QToolButton*>(getMainToolBarBtn(id)->associatedObjects().back());
            break;
        case BTN_OPEN_LOGIN:
            pBtn = m_pUserLoginBtn;
            break;

        // LEFT BTN
        case BTN_PROJECT_PANE:
            pBtn = m_pProjectBtn;
            break;
        case BTN_PROCESS_PANE:
            pBtn = m_pProcessBtn;
            break;
        case BTN_WORKFLOW_PANE:
            pBtn = m_pWorkflowBtn;
            break;
        case BTN_PROPERTIES_PANE:
            pBtn = m_pPropertiesBtn;
            break;

        // RIGHT BTN
        case BTN_LOG_PANE:
            pBtn = m_pNotifyBtn;
            break;
        case BTN_TUTO_PANE:
            pBtn = m_pTutoBtn;
            break;

        // DOWN
        case BTN_MODULE_WORKFLOW:
            pBtn = m_pModuleDock->getModuleBtn(0);
            break;
        case BTN_MODULE_PYTHON:
            pBtn = m_pModuleDock->getModuleBtn(1);
            break;
    }
    return pBtn;
}

CHubDlg *CMainView::getHubView() const
{
    return m_pHubDlg;
}

CProcessPopupDlg *CMainView::getProcessPopup() const
{
    return m_pProcessDlg;
}

CPreferencesDlg* CMainView::getPreferenceDlg()
{
    return &m_preferencesDlg;
}

CUserLoginDlg* CMainView::getUserLoginDlg() const
{
    return m_pUserLoginDlg;
}

void CMainView::resizeEvent(QResizeEvent* event)
{
    QScreen* pScreen = this->window()->windowHandle()->screen();
    QSize desktopSize = pScreen->availableSize();

    // left and right panes sizes (width) are relative to the app size (width) => 20% of width
    // Update panes sizes
    m_pLeftPanes->setMaximumWidth(this->frameGeometry().width()*0.2);
    m_pRightPanes->setMaximumWidth(this->frameGeometry().width()*0.2);
    // Update panes animation sizes
    m_pLeftPanes->setAnimation("maximumWidth", 0, this->frameGeometry().width()*0.2);
    m_pRightPanes->setAnimation("maximumWidth", 0, this->frameGeometry().width()*0.2);

    // Calculate minimum size height of app + dockwidget in order to avoid unmaximized window
    setMinimumSize(qMin((int)(desktopSize.width()*0.9), sizeHint().width()), qMin((int)(desktopSize.height()*0.9), sizeHint().height() + m_pModuleDock->sizeHint().height()));

    QMainWindow::resizeEvent(event);
}

/***********/
/* PRIVATE */
/***********/

// Init functions
void CMainView::init()
{
    // Main layout with title bar and core view
    m_pMainLayout = new QVBoxLayout;
    m_pErrMsgDlg = new QErrorMessage(this);

    // Init title bar and central view
    initUpView();

    // Init Module view in QDockWidget
    initBottomView();

    // Init notifier in system tray icon and internal notifier
    initNotifications();

    // Init Progress bar
    initProgressBar();

    // Init login dialog
    initLoginDialog();

    // Init Hub view
    initHubView();

    //Init process popup
    initProcessPopup();

    // Init main connections
    initConnections();
}

void CMainView::initDoubleView()
{
    m_pDoubleView = new CDoubleView(this);
    m_pDoubleView->insertIntoSplitter(m_pMainViewSplitter);
    m_pMainViewSplitter->setObjectName("MainSplitter");
}

void CMainView::initTitleBarView()
{
    m_pTitleBarView = new CMainTitleBar(m_titleBarSize, this);
    m_pMainLayout->addWidget(m_pTitleBarView);
}

void CMainView::initCentralView()
{
    m_pCentralView = new QWidget;
    m_pCentralViewLayout = new CCentralViewLayout;
    m_pMainViewSplitter = new QSplitter;

    // Add up tab (toolbar)
    initMainToolBar();

    // Init left tab (Project - Process - Workflows - Information)
    initLeftTab();

    //Init graphics toolbar
    initGraphicsToolbar();

    // Init central view with a double view
    initDoubleView();

    // Init Right tab (Notifications - Tutorials)
    initRightTab();

    //Set main splitter sizes
    // Left tab - Graphics toolbar - Graphics preferences - Image Src - Image Result - Right tab
    m_pMainViewSplitter->setSizes(QList<int>({INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX}));

    m_pCentralViewLayout->addWidget(m_pMainViewSplitter, CBorderLayout::Center);
    m_pCentralView->setLayout(m_pCentralViewLayout);
    m_pMainLayout->addWidget(m_pCentralView);
}

void CMainView::initUpView()
{
#ifndef Q_OS_MACOS
    initTitleBarView();
#endif
    initCentralView();

    auto pCentralWidget = new QWidget(this);
    pCentralWidget->setLayout(m_pMainLayout);
    setCentralWidget(pCentralWidget);
    centralWidget()->layout()->setSpacing(0);
    centralWidget()->layout()->setContentsMargins(0,0,0,0);
    centralWidget()->layout()->setAlignment(Qt::AlignTop);

    // Status Bar
    m_pTimer = new QLabel;
    statusBar()->setStyleSheet("background: rgb(40,40,40);");
    statusBar()->addPermanentWidget(m_pTimer);
}

void CMainView::initBottomView()
{
    m_pModuleDock = new CModuleDockWidget(tr("Modules"), this);
    initWorkflowModule();
    initPluginMakerModule();
    initJupyterLauncher();
    addDockWidget(Qt::BottomDockWidgetArea, m_pModuleDock);
}

void CMainView::initWorkflowModule()
{
    m_pWorkflowModule = new CWorkflowModuleWidget(this);
    m_pModuleDock->addModuleWidget(m_pWorkflowModule, QIcon(":/Images/module_workflow.png"));
}

void CMainView::initPluginMakerModule()
{
    if(m_pPluginMgrModule == nullptr)
    {
        m_pPluginMgrModule = new CPluginManagerWidget;
        m_pModuleDock->addModuleWidget(m_pPluginMgrModule, QIcon(":/Images/plugin-color.png"));

        connect(m_pPluginMgrModule, &CPluginManagerWidget::doNewNotification, [&](const QString &text, Notification type, CProgressCircle* pProgress, int duration)
        {
            onNewNotification(text, type, pProgress, duration);
        });

        //View -> process model
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doGetPythonQueryModel, [&]{ emit doGetPythonQueryModel(); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doReloadAll, [&]{ emit doReloadAllPlugins(); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doReload, [&](const QString& pluginName, int language){ emit doReloadPlugin(pluginName, language); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doInstallRequirements, [&](const QString& pluginName){ emit doInstallPluginRequirements(pluginName); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doEditPythonPlugin, [&](const QString& pluginName){ emit doEditPythonPlugin(pluginName); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doShowLocation, [&](const QString& pluginName, int language){ emit doShowPluginLocation(pluginName, language); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doGetPythonDependencyModel, [&](const QString& pluginName){ emit doGetPythonDependencyModel(pluginName); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doInstallPythonPkg, [&](const QString& moduleName){ emit doInstallPythonPkg(moduleName); });
        connect(m_pPluginMgrModule, &CPluginManagerWidget::doUpdatePythonPkg, [&](const QString& moduleName){ emit doUpdatePythonPkg(moduleName); });

        emit doGetPythonQueryModel();
    }
}

void CMainView::initJupyterLauncher()
{
    //Jupyter Lab launcher
    auto action = [&](){ emit doStartJupyterLab(); };
    m_pModuleDock->addModuleAction(action, QIcon(":/Images/jupyter.png"), tr("Open Jupyter Lab"));
}

void CMainView::initConnections()
{
#ifndef Q_OS_MACOS
    // Title bar connections on close, minimize and expand
    connect(m_pTitleBarView, &CMainTitleBar::close, this, &CMainView::onClose);
    connect(m_pTitleBarView, &CMainTitleBar::minimize, this, &CMainView::onMinimizeWindow);
    connect(m_pTitleBarView, &CMainTitleBar::expand, this, &CMainView::onExpandWindow);
#endif
    // QAction
    connect(m_pNewProjectAct, &QAction::triggered, [this]
    {
        m_pLeftPanes->showPane(m_pProjectPane);
        // Select project btn in left pane
        m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
        emit doNewProject();
    });
    connect(m_pOpenProjectAct, &QAction::triggered, [this]
    {
        QSettings IkomiaSettings;
        QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Open Project"), IkomiaSettings.value(_DefaultDirProject).toString(), tr("db Files (*.db)"), nullptr, CSettingsManager::dialogOptions());
        if(filenames.size() > 0)
        {
            m_lastPathProject = QFileInfo(filenames.first()).path(); // store path for next time
            IkomiaSettings.setValue(_DefaultDirProject, m_lastPathProject);
            openProjectFiles(filenames);
        }
    });
    connect(m_pOpenImgAct, &QAction::triggered, [this]
    {
        QSettings IkomiaSettings;
        QStringList files = QFileDialog::getOpenFileNames(  this, tr("Choose image files"), IkomiaSettings.value(_DefaultDirImg).toString(),
                                                            tr("All images (*.jpg *.JPG *.jpeg *.JPEG *.tif *.TIF *.tiff *.TIFF *.png *.PNG *.bmp *.BMP *.jp2 *.JP2 *.pgm *.PGM *.exr *.EXR *.hdr *.HDR *.ppm *.PPM *.webp *.WEBP)"), nullptr,
                                                            CSettingsManager::dialogOptions() );
        if(files.size() > 0)
        {
           m_lastPathImg = QFileInfo(files.first()).path(); // store path for next time
           IkomiaSettings.setValue(_DefaultDirImg, m_lastPathImg);
           openImages(files);
        }
    });
    connect(m_pOpenVideoAct, &QAction::triggered, [this]
    {
        QSettings IkomiaSettings;
        QStringList files = QFileDialog::getOpenFileNames(  this, tr("Choose video files"), IkomiaSettings.value(_DefaultDirVideo).toString(),
                                                            tr("All videos (*.avi *.mp4 *.webm *.mxf *.mxg *.ts)"), nullptr,
                                                            CSettingsManager::dialogOptions() );
        if(files.size() > 0)
        {
            m_lastPathVideo = QFileInfo(files.first()).path(); // store path for next time
            IkomiaSettings.setValue(_DefaultDirVideo, m_lastPathVideo);
            openVideoFiles(files);
        }
    });
    connect(m_pOpenCamAct, &QAction::triggered, [this]
    {
        QString mode;
        QString name;

        auto bRet = selectCamera(mode, name);
        if(bRet == false)
            return;

        emit doLoadStream(mode, name, m_pProjectPane->getCurrentIndex());
        m_pLeftPanes->showPane(m_pProjectPane);
        // Select project btn in left pane
        m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
    });
    connect(m_pOpenFolderAct, &QAction::triggered, [&]
    {
        QSettings ikomiaSettings;
        QString dir = QFileDialog::getExistingDirectory(this, tr("Choose image folder"), ikomiaSettings.value(_DefaultDirImg).toString(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | CSettingsManager::dialogOptions());
        if(!dir.isEmpty())
        {
            ikomiaSettings.setValue(_DefaultDirImg, dir);
            openFolder(dir);
        }
    });
    connect(m_pOpenHubAct, &QAction::triggered, this, &CMainView::onShowHub);
    connect(m_pPluginAct, &QAction::triggered, [this]{ emit doLoadPlugin(); });
    connect(m_pPreferencesAct, &QAction::triggered, this, &CMainView::onShowPreferences);

    //Project Pane -> double view
    connect(m_pProjectPane, &CProjectPane::doUpdateIndex, m_pDoubleView, &CDoubleView::onUpdateDataViewerIndex);

    // Project Pane -> main view
    connect(m_pProjectPane, &CProjectPane::doNewProject, [this]{ emit doNewProject(); });
    connect(m_pProjectPane, &CProjectPane::doOpenProject, [&]{
        QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Open Project"), "", tr("db Files (*.db)"), nullptr, CSettingsManager::dialogOptions());
        if(filenames.size() > 0)
        {
            emit doLoadProject(filenames);
        }
    });

    //Double view -> project pane
    connect(m_pDoubleView, &CDoubleView::doUpdateIndex, m_pProjectPane, &CProjectPane::onUpdateIndex);
    connect(m_pDoubleView->getDataViewer()->getDataListView()->proxyModel(), &CProjectViewProxyModel::dataChanged, m_pProjectPane, &CProjectPane::onDataChanged);

    //User login
    connect(m_pUserLoginBtn, &QToolButton::clicked, this, &CMainView::showLoginDialog);

    //Procotol module -> module dock widget
    connect(m_pWorkflowModule, &CWorkflowModuleWidget::doUpdateTitle, [&](){ m_pModuleDock->updateWindowTitle(); });

    //Workflow module -> MainView
    connect(m_pWorkflowModule, &CWorkflowModuleWidget::doShowProcessPopup, this, &CMainView::onShowProcessPopup);

    //Graphics toolbar -> MainView
    connect(m_pGraphicsToolbar, &CGraphicsToolbar::doToggleGraphicsProperties, [&]{ toggleGraphicsProperties(); });
    connect(m_pGraphicsToolbar, &CGraphicsToolbar::doSetGraphicsTool, [&](GraphicsShape tool){ m_pGraphicsProperties->setTool(tool); });

    //Image views -> MainView
    connect(m_pDoubleView, &CDoubleView::doToggleGraphicsToolbar, [&]{ toggleGraphicsToolbar(); });
    connect(m_pDoubleView, &CDoubleView::doHideGraphicsToolbar, [&]{ hideGraphicsToolbar(); });

    //Workflow Pane -> MainView
    connect(m_pWorkflowPane, &CWorkflowPane::doOpenWorkflowView, [&]{ m_pModuleDock->showModule(0); });

    // Left buttons
    connect(m_pProjectBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getLeftTab()->setCurrentRow(0); });
    connect(m_pProcessBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getLeftTab()->setCurrentRow(1); });
    connect(m_pWorkflowBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getLeftTab()->setCurrentRow(2); });
    connect(m_pPropertiesBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getLeftTab()->setCurrentRow(3); });

    // Right buttons
    //Notification center
    connect(m_pNotifyBtn, &QToolButton::clicked, this, &CMainView::showNotificationCenter);
    connect(m_pNotifyBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getRightTab()->setCurrentRow(0); });

    // Mainview -> notification pane
    connect(this, &CMainView::doDisplayLogMsg, m_pNotificationPane, &CNotificationPane::onDisplayLogMsg);

    // Tutorials standby
    //connect(m_pTutoBtn, &QToolButton::clicked, [this]{ m_pCentralViewLayout->getRightTab()->setCurrentRow(1); });

    // Mainview -> dataviewer
    connect(this, &CMainView::doCloseApp, m_pDoubleView->getDataViewer(), &CDataViewer::onStopVideo);
}

void CMainView::initMainToolBar()
{
    QSize itemSize = QSize(36, 36);
    m_pCentralViewLayout->addUpperBar(itemSize, 5);

    //Create new project
    const QIcon newIcon(":/Images/new-project.png");
    m_pNewProjectAct = new QAction(newIcon, tr("New project"), this);
    m_pNewProjectAct->setStatusTip(tr("Create new project"));
    m_pCentralViewLayout->addActionToMenu(m_pNewProjectAct);

    //Open project
    const QIcon openProjectIcon(":/Images/open-project.png");
    m_pOpenProjectAct = new QAction(openProjectIcon, tr("&Open project..."), this);
    m_pOpenProjectAct->setStatusTip(tr("Open a project"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenProjectAct);
    QToolButton* pOpenProject = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, openProjectIcon);
    pOpenProject->setToolTip(tr("Open project"));
    pOpenProject->setDefaultAction(m_pOpenProjectAct);

    //Open image(s)
    const QIcon openImgIcon(":/Images/open-image.png");
    m_pOpenImgAct = new QAction(openImgIcon, tr("&Open image..."), this);
    m_pOpenImgAct->setStatusTip(tr("Open an image"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenImgAct);
    QToolButton* pOpenImg = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, openImgIcon);
    pOpenImg->setToolTip(tr("Open image"));
    pOpenImg->setDefaultAction(m_pOpenImgAct);

    //Open video(s)
    const QIcon openVideoIcon(":/Images/open-video.png");
    m_pOpenVideoAct = new QAction(openVideoIcon, tr("&Open video..."), this);
    m_pOpenVideoAct->setStatusTip(tr("Open a video"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenVideoAct);
    QToolButton* pOpenVideo = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, openVideoIcon);
    pOpenVideo->setToolTip(tr("Open video"));
    pOpenVideo->setDefaultAction(m_pOpenVideoAct);

    //Open camera
    const QIcon openCamIcon(":/Images/webcam.png");
    m_pOpenCamAct = new QAction(openCamIcon, tr("&Open camera..."), this);
    m_pOpenCamAct->setStatusTip(tr("Open a camera"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenCamAct);
    QToolButton* pOpenCam = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, openCamIcon);
    pOpenCam->setToolTip(tr("Open camera"));
    pOpenCam->setDefaultAction(m_pOpenCamAct);

    //Open folder
    const QIcon openFolderIcon(":/Images/folder.png");
    m_pOpenFolderAct = new QAction(openFolderIcon, tr("&Open folder..."), this);
    m_pOpenFolderAct->setStatusTip(tr("Open image folder"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenFolderAct);

    //Open plugin Hub
    const QIcon hubIcon(":/Images/hub-color.png");
    m_pOpenHubAct = new QAction(hubIcon, tr("&Open Ikomia HUB..."), this);
    m_pOpenHubAct->setStatusTip(tr("Open Ikomia HUB"));
    m_pCentralViewLayout->addActionToMenu(m_pOpenHubAct);
    QToolButton* pOpenHub = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, hubIcon, Qt::AlignRight);
    pOpenHub->setToolTip(tr("Open Ikomia HUB"));
    pOpenHub->setDefaultAction(m_pOpenHubAct);

    //Load plugins : action only
    const QIcon pluginImgIcon(":/Images/update-color.png");
    m_pPluginAct = new QAction(pluginImgIcon, tr("&Load plugins..."), this);
    m_pPluginAct->setStatusTip(tr("Load plugins"));
    m_pCentralViewLayout->addActionToMenu(m_pPluginAct);

    //General preferences : action only
    const QIcon preferencesIcon(":/Images/preferences-color.png");
    m_pPreferencesAct = new QAction(preferencesIcon, tr("&Preferences..."), this);
    m_pPreferencesAct->setStatusTip(tr("Show preferences"));
    m_pCentralViewLayout->addActionToMenu(m_pPreferencesAct);

    //User login
    m_pUserLoginBtn = m_pCentralViewLayout->addButtonToUpperBar("", itemSize, QIcon(":/Images/avatar-offline.png"), Qt::AlignRight);
    m_pUserLoginBtn->setToolTip(tr("Offline"));
}

void CMainView::initLeftTab()
{
    // Add left tab with fixed width
    m_pCentralViewLayout->addLeftBar(QSize(50, 50), 5);

    initLeftPanes();

    // Add button to show/hide project pane
    m_pProjectBtn = createLeftTabBtn(tr("Projects"), tr("Projects"), QIcon(":/Images/projects.png"), m_pProjectPane);

    // Add button to show/hide process pane
    m_pProcessBtn = createLeftTabBtn(tr("Process"), tr("Process"), QIcon(":/Images/process.png"), m_pProcessPane);

    // Add button to show/hide protocol pane
    m_pWorkflowBtn = createLeftTabBtn(tr("Workflows"), tr("Workflows"), QIcon(":/Images/workflow.png"), m_pWorkflowPane);

    // Add button to show/hide protocol pane
    m_pPropertiesBtn = createLeftTabBtn(tr("Properties"), tr("Data properties"), QIcon(":/Images/info-color.png"), m_pInfoPane); 
}

void CMainView::initRightTab()
{
    // Add right tab with fixed width
    m_pCentralViewLayout->addRightBar(QSize(50, 50), 5);

    initRightPanes();

    // Add button to show/hide notification pane
    m_pNotifyBtn = createRightTabBtn(tr("Logs"), tr("Show notification center"), QIcon(":/Images/notification.png"), m_pNotificationPane);

    // Tutorials standby
    // Add button to show/hide tutorial pane
    //m_pTutoBtn = createRightTabBtn(tr("Tutorials"), tr("Show tutorials"), QIcon(":/Images/preferences-color.png"), m_pWizardPane);
}

void CMainView::initLeftPanes()
{
    assert(m_pMainViewSplitter != nullptr);

    m_pProjectPane = new CProjectPane;
    m_pProcessPane = new CProcessPane;
    m_pWorkflowPane = new CWorkflowPane;
    m_pInfoPane = new CInfoPane;

    m_pLeftPanes = new CStackedPane;
    m_pLeftPanes->setAnimation("maximumWidth", 0, 400);
    m_pLeftPanes->addPane(m_pProjectPane);
    m_pLeftPanes->addPane(m_pProcessPane);
    m_pLeftPanes->addPane(m_pWorkflowPane);
    m_pLeftPanes->addPane(m_pInfoPane);

    m_pMainViewSplitter->addWidget(m_pLeftPanes);
}

void CMainView::initRightPanes()
{
    assert(m_pMainViewSplitter != nullptr);

    m_pRightPanes = new CStackedPane;
    m_pRightPanes->setAnimation("maximumWidth", 0, 400);

    m_pNotificationPane = new CNotificationPane;
    m_pRightPanes->addPane(m_pNotificationPane);
    // Tutorials standby
    //m_pWizardPane = new CWizardPane;
    //m_pRightPanes->addPane(m_pWizardPane);

    m_pMainViewSplitter->addWidget(m_pRightPanes);
}

void CMainView::initGraphicsToolbar()
{
    m_pGraphicsToolbar = new CGraphicsToolbar(30);
    m_pGraphicsProperties = new CGraphicsPropertiesWidget;
    m_pMainViewSplitter->addWidget(m_pGraphicsToolbar);
    m_pMainViewSplitter->addWidget(m_pGraphicsProperties);
    m_pGraphicsToolbar->hide();
    m_pGraphicsProperties->hide();
}

void CMainView::initNotifications()
{
    if(QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_pNotifier = new QSystemTrayIcon(QIcon(":/Images/app.png"), this);
        m_pNotifier->show();
        connect(m_pNotifier, &QSystemTrayIcon::activated, [this]{
            auto title = "Ikomia Studio";
            auto msg = QString("Version %1").arg(QString::fromStdString(Utils::IkomiaApp::getCurrentVersionName()));
            m_pNotifier->showMessage(title, msg, QIcon(":/Images/app.png"));
        });
    }
    else
    {
        qDebug() << "No system tray icon.";
    }
}

void CMainView::initProgressBar()
{
    auto pal = qApp->palette();
    auto colorProgress = pal.highlight().color();
    auto colorInner = pal.base().color();
    auto colorOuter = pal.alternateBase().color();
    m_circleProgressMgr.setProgressSize(64,64);
    m_circleProgressMgr.setColorBase(colorProgress);
    m_circleProgressMgr.setColorBg(colorOuter);
    m_circleProgressMgr.setColorInner(colorInner);
}

void CMainView::initHubView()
{
    m_pHubDlg = new CHubDlg(this);
}

void CMainView::initProcessPopup()
{
    //No parent to avoid incompatibility between CFramelessHelper and modality on Windows
    m_pProcessDlg = new CProcessPopupDlg;
}

void CMainView::initLoginDialog()
{
    m_pUserLoginDlg = new CUserLoginDlg(this);
    m_pUserLoginDlg->adjustSize();

    connect(m_pUserLoginDlg, &CUserLoginDlg::doConnectUser, [&](const QString& login, const QString& pwd, bool bRememberMe)
    {
        emit doConnectUser(login, pwd, bRememberMe);
        m_pUserLoginDlg->hide();
    });
    connect(m_pUserLoginDlg, &CUserLoginDlg::doDisconnectUser, [&]
    {
        emit doDisconnectUser();
        m_pUserLoginDlg->hide();
    });
}

void CMainView::initAllPath()
{
    m_lastPathImg = QDir::homePath();
    m_lastPathVideo = QDir::homePath();
    m_lastPathProject = QDir::homePath();
}

QToolButton* CMainView::createLeftTabBtn(const QString& name, const QString& tooltip, const QIcon& icon, QWidget* pane)
{
    auto pBtn = m_pCentralViewLayout->addButtonToLeftBar(name, QSize(40,50), icon);
    assert(pBtn != nullptr);

    pBtn->setToolTip(tooltip);

    connect(pBtn, &QToolButton::clicked, this, [=]
    {
        if(m_pProcessPane->getProcessParameters())
            m_pProcessPane->getProcessParameters()->hide();
        if(m_pProcessPane->getListPopup())
            m_pProcessPane->getListPopup()->hide();
        m_pLeftPanes->togglePane(pane);
    });
    return pBtn;
}

QToolButton* CMainView::createRightTabBtn(const QString& name, const QString& tooltip, const QIcon& icon, QWidget* pane)
{
    auto pBtn = m_pCentralViewLayout->addButtonToRightBar(name, QSize(40,50), icon);
    assert(pBtn != nullptr);

    pBtn->setToolTip(tooltip);

    connect(pBtn, &QToolButton::clicked, this, [=]
    {
        if(m_pProcessPane->getProcessParameters())
            m_pProcessPane->getProcessParameters()->hide();
        if(m_pProcessPane->getListPopup())
            m_pProcessPane->getListPopup()->hide();

        m_pRightPanes->togglePane(pane);
    });

    return pBtn;
}

void CMainView::animateWidget(QWidget* pWidget)
{
    assert(pWidget != nullptr);

    QPropertyAnimation* animation = new QPropertyAnimation(pWidget, "maximumWidth");
    animation->setDuration(300);
    if(pWidget->maximumWidth() != 0)
    {
        animation->setStartValue(2000);
        animation->setEndValue(0);
    }
    else
    {
        animation->setStartValue(0);
        animation->setEndValue(2000);
    }
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

// App Properties
void CMainView::readSettings()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    setWindowTitle(QApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::primaryScreen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }
}

void CMainView::setAppStyle(const QString& style)
{
    if(m_style == style)
        return;

    m_style = style;

    if(m_style == "Classic")
    {
        qApp->setStyleSheet(qApp->styleSheet());
        qApp->setStyle(QStyleFactory::create("Fusion"));

        // First, set the global palette colors
        // Note: the following colors may be wrong, please test and fix these on Linux with an empty mainstyle.css
        QPalette p = qApp->palette();

        QColor selCol, sunkCol, baseCol, raisedCol, txtCol, intCol, kfCol, disCol, eCol, altCol, lightSelCol;
        selCol = QColor(255,122,0);
        sunkCol = QColor(70,70,70);
        baseCol = QColor(48,48,48);
        raisedCol = QColor(70,70,70);
        txtCol = QColor(179,181,180);
        intCol = Qt::white;
        kfCol = Qt::white;
        disCol = Qt::black;
        eCol = Qt::white;
        altCol = QColor(200,200,200);
        lightSelCol = QColor(255,122,0);

        p.setBrush(QPalette::Window, sunkCol);
        p.setBrush(QPalette::WindowText, txtCol);
        p.setBrush(QPalette::Base, baseCol);
        p.setBrush(QPalette::AlternateBase, raisedCol);
        p.setBrush(QPalette::ToolTipBase, baseCol);
        p.setBrush(QPalette::ToolTipText, txtCol);
        p.setBrush(QPalette::Text, txtCol);
        p.setBrush(QPalette::Button, baseCol);
        p.setBrush(QPalette::ButtonText, txtCol);
        p.setBrush(QPalette::Light, baseCol.lighter());
        p.setBrush(QPalette::Midlight, baseCol.lighter());
        p.setBrush(QPalette::Mid, selCol.lighter(100)); // For QToolBox tab line
        p.setBrush(QPalette::Dark, baseCol.darker(150));
        p.setBrush(QPalette::Shadow, sunkCol);
        p.setBrush(QPalette::BrightText, txtCol );
        p.setColor(QPalette::Highlight, lightSelCol);
        p.setColor(QPalette::HighlightedText, Qt::black);
        p.setBrush(QPalette::Link, selCol); // can only be set via palette
        p.setBrush(QPalette::LinkVisited, selCol); // can only be set via palette
        qApp->setPalette( p );

        QFile qss;
        qss.setFileName( QString::fromUtf8(":mainstyle.css") );

        if ( qss.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QTextStream in(&qss);
            QString content = QString::fromUtf8("QWidget { font-family: \"%1\"; font-size: %2pt; }\n"
                                                "QListView { font-family: \"%1\"; font-size: %2pt; }\n" // .. or the items in the QListView get the wrong font
                                                "QComboBox::drop-down { font-family: \"%1\"; font-size: %2pt; }\n" // ... or the drop-down doesn't get the right font
                                                "QInputDialog { font-family: \"%1\"; font-size: %2pt; }\n" // ... or the label doesn't get the right font
                                                ).arg(fontInfo().family()).arg(fontInfo().pointSize());
            content += in.readAll();
            qApp->setStyleSheet( content
                                 .arg( selCol.name() ) // %1: selection-color
                                 .arg( baseCol.name() ) // %2: medium background
                                 .arg( raisedCol.name() ) // %3: soft background
                                 .arg( sunkCol.name() ) // %4: strong background
                                 .arg( txtCol.name() ) // %5: text colour
                                 .arg( intCol.name() ) // %6: interpolated value color
                                 .arg( kfCol.name() ) // %7: keyframe value color
                                 .arg( disCol.name() ) // %8: disabled editable text
                                 .arg( eCol.name() ) // %9: expression background color
                                 .arg( altCol.name() ) // %10 = altered text color
                                 .arg( lightSelCol.name() ) ); // %11 = mouse over selection color
        }
        else
            qDebug() << "Style sheet not loaded.";
    }
    else
    {
        qApp->setStyleSheet(qApp->styleSheet());
        qApp->setPalette( qApp->style()->standardPalette() );
    }
}

void CMainView::onSetGraphicsContext(GraphicsContextPtr &context)
{
    m_pGraphicsToolbar->setContext(context);
    m_pGraphicsProperties->setContext(context);
}

void CMainView::onSetCurrentUser(const CUser& user)
{
    m_currentUser = user;

    initPluginMakerModule();

    if(m_pTitleBarView)
        m_pTitleBarView->setUser(user.m_name);

    if(m_pUserLoginDlg)
        m_pUserLoginDlg->setCurrentUser(user.m_name);

    if(m_pHubDlg)
        m_pHubDlg->setCurrentUser(user);

    if(m_pWorkflowModule)
        m_pWorkflowModule->setCurrentUser(user);

    if(m_pProcessPane)
        m_pProcessPane->setCurrentUser(user);

    if(m_pProcessDlg)
        m_pProcessDlg->setCurrentUser(user);

    manageUserStatus();
}

void CMainView::onSetWorkflowChangedIcon()
{
    if(m_pModuleDock->isModuleOpen(0))
        m_pModuleDock->getModuleBtn(0)->setIcon(QIcon(":/Images/module_workflow_warning.png"));
}

void CMainView::onGraphicsContextChanged()
{
    m_pGraphicsToolbar->contextChanged();
    m_pGraphicsProperties->contextChanged();
}

void CMainView::onNewNotification(const QString &text, Notification type, CProgressCircle* pProgress, int duration)
{
    QString title;
    QIcon icon;

    switch(type)
    {
        case Notification::INFO:
            title = tr("Information");
            icon = QIcon(":/Images/task-complete.png");
            break;
        case Notification::WARNING:
            title = tr("Warning");
            icon = QIcon(":/Images/task-error.png");
            break;
        case Notification::CRITICAL:
            title = tr("Critical");
            icon = QIcon(":/Images/task-undone.png");
            break;
        case Notification::DEBUG:
            title = tr("Debug");
            icon = QIcon(":/Images/task-undone.png");
            break;
        case Notification::EXCEPTION:
            title = tr("Exception");
            icon = QIcon(":/Images/task-undone.png");
            break;
    }
    showNotification(title, text, icon, pProgress, duration);
}

void CMainView::onNewInfiniteProgress(const QString &text, CProgressCircle* pProgress)
{
    CNotificationPopup* pNotifPopupProgress = m_notifFactory.create(this);
    pNotifPopupProgress->setPadding(m_notifPopupPadding);
    pNotifPopupProgress->addWidget(pProgress, 0, 0);
    pNotifPopupProgress->setDescriptionPos(0, 1);
    pNotifPopupProgress->showPopup(tr("Information"), text, QIcon(":/Images/task-complete.png"));
    m_popupList.append(pNotifPopupProgress);
    pProgress->startInfinite();

    //Update popup padding for next popup position
    m_notifPopupPadding += pNotifPopupProgress->size().height();

    //Add message to notification center and alert user with icon change
    m_pNotificationPane->addNotification(text);
    m_pNotifyBtn->setIcon(QIcon(":/Images/new-notification.png"));

    connect(pNotifPopupProgress, &CNotificationPopup::doClose, [this, pNotifPopupProgress]
    {
        m_notifPopupPadding -= pNotifPopupProgress->size().height();
        manageNotificationPos(pNotifPopupProgress);
    });

    //Display end of process for small amount of time (3 s)
    connect(pProgress, &CProgressCircle::doFinish, [pNotifPopupProgress]
    {
        pNotifPopupProgress->setDescription(tr("Successfully finished."));
        QTimer::singleShot(3000, pNotifPopupProgress, &CNotificationPopup::onClosePopup);
    });
}

void CMainView::onShowErrorMessage(const QString& errMsg)
{
    m_pErrMsgDlg->showMessage(errMsg);
}

void CMainView::onSetPythonQueryModel(CPluginPythonModel *pModel)
{
    if(m_pPluginMgrModule)
        m_pPluginMgrModule->setPythonQueryModel(pModel);
}

void CMainView::onSetPythonDependencyModel(CPluginPythonDependencyModel *pModel)
{
    if(m_pPluginMgrModule)
        m_pPluginMgrModule->setPythonDependencyModel(pModel);
}

void CMainView::onCloseProject(bool bCurrentImgClosed)
{
    if(bCurrentImgClosed == true)
        hideGraphicsToolbar();

    m_pDoubleView->onCloseProject(bCurrentImgClosed);
}

void CMainView::onRestartIkomia()
{
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void CMainView::onClose()
{
    if(m_pHubDlg)
        delete m_pHubDlg;

    if(m_pProcessDlg)
        delete m_pProcessDlg;

    if(m_pNotifier)
        delete m_pNotifier;

    // Notify that app is closing -> stop all videos
    emit doCloseApp();

    // Clear tmp folder
    clearTmpFolder();

    close();
}

// Private slots
void CMainView::onMinimizeWindow()
{
    showMinimized();
}

void CMainView::onExpandWindow()
{
    if (!m_bIsMaximized)
    {
        m_oldGeometry = geometry();
        showMaximized();
        m_pMainwViewMover->setWidgetMovable(false);
        m_pMainwViewMover->setWidgetResizable(false);
        m_bIsMaximized = true;
        //showCustomMaximized();
    }
    else
    {
        showNormal();
        setGeometry(m_oldGeometry);
        //centralWidget()->adjustSize();
        //adjustSize();
        m_pMainwViewMover->setWidgetMovable(true);
        m_pMainwViewMover->setWidgetResizable(true);
        m_bIsMaximized = false;
    }
}

void CMainView::onShowPreferences()
{
    m_preferencesDlg.exec();
}

void CMainView::onShowHub()
{
    if(m_pHubDlg->isHidden())
        m_pHubDlg->show();
    else
        m_pHubDlg->hide();
}

void CMainView::onShowProcessPopup()
{
    // Notify opening for switching connection between processModel/processTreeview/popupTreeview
    emit doBeforeProcessPopupOpen();
    //Show it
    m_pProcessDlg->exec();
    // Notify closing for switching connection between processModel/processTreeview/popupTreeview
    emit doAfterProcessPopupClose();
    // Update tree and list views with current search text
    m_pProcessPane->update();
}

// Other
bool CMainView::eventFilter(QObject* obj, QEvent* event)
{
    return QMainWindow::eventFilter(obj, event);
}

bool CMainView::selectCamera(QString& mode, QString& name)
{
    CCameraDlg dlg(this);

    // Add available camera here
    dlg.findCamera();

    if(dlg.exec() == QDialog::Rejected)
        return false;

    mode = dlg.getCameraMode();
    name = dlg.getCameraName();

    return true;
}

void CMainView::toggleGraphicsToolbar()
{
    if(m_pGraphicsToolbar->isHidden())
        m_pGraphicsToolbar->show();
    else
        hideGraphicsToolbar();
}

void CMainView::toggleGraphicsProperties()
{
    if(m_pGraphicsProperties->isHidden())
        m_pGraphicsProperties->show();
    else
        m_pGraphicsProperties->hide();
}

void CMainView::showNotification(const QString &title, const QString &description, const QIcon &icon, CProgressCircle *pProgress, int duration)
{
    if(pProgress)
        showNotificationProgress(title, description, pProgress, icon);
    else
        showNotificationText(title, description, icon, duration);
}

void CMainView::showNotificationText(const QString& title, const QString& description, const QIcon& icon, int duration)
{
    QLabel* pIcon = new QLabel;
    pIcon->setAlignment(Qt::AlignCenter);
    pIcon->setPixmap(QPixmap(":/Images/notification.png").scaled(32,32));
    pIcon->setFixedSize(32, 32);
    CNotificationPopup* pNotifPopup = m_notifFactory.create(this);
    pNotifPopup->setPadding(m_notifPopupPadding);
    pNotifPopup->addWidget(pIcon, 0, 0);
    pNotifPopup->setDescriptionPos(0, 1);
    pNotifPopup->showPopup(title, description, icon);
    m_popupList.append(pNotifPopup);

    m_notifPopupPadding += pNotifPopup->size().height();
    m_pNotifyBtn->setIcon(QIcon(":/Images/new-notification.png"));

    m_pNotificationPane->addNotification(description);

    connect(pNotifPopup, &CNotificationPopup::doClose, [this, pNotifPopup]
    {
        m_notifPopupPadding -= pNotifPopup->size().height();
        manageNotificationPos(pNotifPopup);
    });
    QTimer::singleShot(duration, pNotifPopup, &CNotificationPopup::onClosePopup);
}

void CMainView::showNotificationProgress(const QString& title, const QString& description, CProgressCircle* pProgress, const QIcon& icon)
{
    //Create popup window with circle progress bar
    CNotificationPopup* pNotifPopupProgress = m_notifFactory.create(this);
    pNotifPopupProgress->setPadding(m_notifPopupPadding);
    pNotifPopupProgress->addWidget(pProgress, 0, 0);
    pNotifPopupProgress->setDescriptionPos(0, 1);
    pNotifPopupProgress->showPopup(title, description, icon);
    m_popupList.append(pNotifPopupProgress);

    //Update popup padding for next popup position
    m_notifPopupPadding += pNotifPopupProgress->size().height();

    //Add message to notification center and alert user with icon change
    m_pNotificationPane->addNotification(description);
    m_pNotifyBtn->setIcon(QIcon(":/Images/new-notification.png"));    

    //Connections
    connect(pNotifPopupProgress, &CNotificationPopup::doClose, [this, pProgress, pNotifPopupProgress]
    {
        m_notifPopupPadding -= pNotifPopupProgress->size().height();
        manageNotificationPos(pNotifPopupProgress);
    });

    // Display end of process for small amount of time (3 s)
    connect(pProgress, &CProgressCircle::doFinish, [=]
    {
        pNotifPopupProgress->setDescription(tr("Successfully finished."));
        QTimer::singleShot(1500, pNotifPopupProgress, &CNotificationPopup::onClosePopup);
    });
    connect(pProgress, &CProgressCircle::doAbort, [=]
    {
        pNotifPopupProgress->setDescription(tr("The process has been aborted."));
        QTimer::singleShot(1500, pNotifPopupProgress, &CNotificationPopup::onClosePopup);
    });
    connect(pProgress, &CProgressCircle::doSetMessage, [pNotifPopupProgress](const QString& msg)
    {
        pNotifPopupProgress->setDescription(msg);
    });
}

void CMainView::showNotificationCenter()
{
    m_pNotifyBtn->setIcon(QIcon(":/Images/notification.png"));
}

void CMainView::manageNotificationPos(CNotificationPopup* pNotif)
{
    auto ind = m_popupList.indexOf(pNotif);
    pNotif->hide();

    for(int i=ind; i<m_popupList.size(); ++i)
    {
        auto pPopup = m_popupList.at(i);
        pPopup->move(pPopup->x(), pPopup->y()-pNotif->height());
    }
    m_popupList.removeOne(pNotif);
}

void CMainView::manageUserStatus()
{
    if(m_currentUser.m_name.isEmpty())
    {
        m_pUserLoginBtn->setIcon(QIcon(":/Images/avatar-offline.png"));
        m_pUserLoginBtn->setToolTip(tr("Offline"));
    }
    else
    {
        m_pUserLoginBtn->setIcon(QIcon(":/Images/avatar-online.png"));
        m_pUserLoginBtn->setToolTip(tr("Online"));
    }
}

void CMainView::manageOpenFiles(const QStringList& files)
{
    if(files.size() > 0)
    {
        QStringList imageTypes;
        imageTypes << "jpeg" << "jpg" << "png" << "tif" << "tiff" << "bmp" << "jp2" << "pgm" << "exr" << "hdr" << "ppm";

        QStringList videoTypes;
        videoTypes << "avi" << "mp4" << "webm";

        QStringList projectTypes;
        projectTypes << "db";

        QStringList imageFiles, videoFiles, projectFiles;
        for(auto it : files)
        {
            QFileInfo info(it);
            if (info.exists())
            {
                if (imageTypes.contains(info.suffix().trimmed(), Qt::CaseInsensitive))
                    imageFiles << it;
                else if (videoTypes.contains(info.suffix().trimmed(), Qt::CaseInsensitive))
                    videoFiles << it;
                else if (projectTypes.contains(info.suffix().trimmed(), Qt::CaseInsensitive))
                    projectFiles << it;
            }
        }

        if(imageFiles.size() > 0)
            openImages(imageFiles);

        if(videoFiles.size() > 0)
            openVideoFiles(videoFiles);

        if(projectFiles.size() > 0)
            openProjectFiles(projectFiles);
    }
}

void CMainView::clearTmpFolder()
{
    QString path = Utils::IkomiaApp::getQIkomiaFolder() + "/tmp";
    QDir dir(path);
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);

    foreach(QString dirFile, dir.entryList())
        dir.remove(dirFile);
}

void CMainView::showLoginDialog()
{
    if(m_pUserLoginDlg->isHidden() == false)
        m_pUserLoginDlg->hide();
    else
    {
        //Set position
        auto upperBarRect = m_pCentralViewLayout->getUpperBarRect();
        const QRect dlgRect = m_pUserLoginDlg->geometry();
        auto top = upperBarRect.bottom();
        m_pUserLoginDlg->move(upperBarRect.right() - dlgRect.width(), top);

        //Show it
        m_pUserLoginDlg->show();
    }
}

void CMainView::showCustomMaximized()
{
    QScreen* pScreen = this->window()->windowHandle()->screen();
    QSize desktopSize = pScreen->availableSize();
    resize(desktopSize);
    m_bIsMaximized = true;
}

void CMainView::hideGraphicsToolbar()
{
    m_pGraphicsToolbar->hide();
    m_pGraphicsProperties->hide();
}

void CMainView::openImages(const QStringList& files)
{
    try
    {
        DatasetLoadPolicy policy = m_pProjectPane->getDatasetLoadPolicy(static_cast<size_t>(TreeItemType::NONE), files);
        emit doLoadImages(files, policy, m_pProjectPane->getCurrentIndex());
        m_pLeftPanes->showPane(m_pProjectPane);
        // Select project btn in left pane
        m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
    }
    catch(std::exception& e)
    {
        qDebug() << e.what();
    }
}

void CMainView::openVideoFiles(const QStringList& files)
{
    try
    {
        emit doLoadVideos(files, m_pProjectPane->getCurrentIndex());
    }
    catch(std::exception& e)
    {
        qDebug() << e.what();
    }
    m_pLeftPanes->showPane(m_pProjectPane);
    // Select project btn in left pane
    m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
}

void CMainView::openProjectFiles(const QStringList& files)
{
    emit doLoadProject(files);
    m_pLeftPanes->showPane(m_pProjectPane);
    // Select project btn in left pane
    m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
}

void CMainView::openFolder(const QString &dir)
{
    emit doLoadFolder(dir, m_pProjectPane->getCurrentIndex());
    m_pLeftPanes->showPane(m_pProjectPane);
    m_pCentralViewLayout->getLeftTab()->setCurrentRow(0);
}

void CMainView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void CMainView::dropEvent(QDropEvent* e)
{
    const QMimeData* mimeData = e->mimeData();

    // check for our needed mime type, here a file or a list of files
    if (mimeData->hasUrls())
    {
        QStringList pathList;

        // extract the local paths of the files
        foreach(const QUrl & url, e->mimeData()->urls())
            pathList.append(url.toLocalFile());

        // call a function to open the files
        manageOpenFiles(pathList);
    }
}

void CMainView::openHUB(const QString &algoName)
{
    if (m_pHubDlg)
        m_pHubDlg->showAlgorithm(algoName);
}

#include "moc_CMainView.cpp"
