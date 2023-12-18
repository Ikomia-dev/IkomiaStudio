/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CMAINVIEW_H
#define CMAINVIEW_H

#include <QMainWindow>
#include <QtWidgets>
#include "ui_CMainView.h"
#include "CMainTitleBar.h"
#include "CCentralViewLayout.h"
#include "CUserLoginDlg.h"
#include "CResponsiveUI.h"
#include "Main/forwards.hpp"
#include "Main/AppDefine.hpp"
#include "View/DoubleView/Result/CResultsViewer.h"
#include "View/DoubleView/Image/CImageDisplay.h"
#include "View/DoubleView/3D/C3dDisplay.h"
#include "View/DoubleView/CDataListView.h"
#include "View/DoubleView/CDataViewer.h"
#include "View/DoubleView/CDoubleView.h"
#include "View/Common/CToolbarBorderLayout.h"
#include "View/Common/CStackedPane.h"
#include "View/Common/CCameraDlg.h"
#include "View/Common/CInfoPane.h"
#include "View/Common/CFramelessHelper.h"
#include "View/Common/CProgressCircleManager.h"
#include "View/Notifications/CNotificationPopup.h"
#include "View/Notifications/CNotificationPane.h"
#include "View/Project/CProjectPane.h"
#include "View/Project/CDatasetLoadPolicyDlg.h"
#include "View/Process/CProcessPane.h"
#include "View/Workflow/CWorkflowPane.h"
#include "View/Modules/Workflow/CWorkflowView.h"
#include "View/Modules/CModuleDockWidget.h"
#include "View/Modules/Workflow/CWorkflowModuleWidget.h"
#include "View/Modules/PluginManager/CPluginManagerWidget.h"
#include "View/Graphics/CGraphicsToolbar.h"
#include "View/Graphics/CGraphicsPropertiesWidget.h"
#include "View/Preferences/CPreferencesDlg.h"
#include "View/Wizard/CWizardPane.h"
#include "Graphics/CGraphicsContext.h"

/**
 * @file      CMainView.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CMainView definition
 *
 * @details   Details
 */

class CHubDlg;
class CProcessPopupDlg;

namespace Ui
{
    class CMainView;
}

/**
 * @brief
 *
 */
class CMainView : public QMainWindow
{
    Q_OBJECT

    public:

        static bool bUseOpenGL_330;

        enum {
            BTN_OPEN_PROJECT,
            BTN_OPEN_IMG,
            BTN_OPEN_VIDEO,
            BTN_OPEN_STREAM,
            BTN_OPEN_HUB,
            BTN_OPEN_LOGIN,
            BTN_PROJECT_PANE,
            BTN_PROCESS_PANE,
            BTN_WORKFLOW_PANE,
            BTN_PROPERTIES_PANE,
            BTN_LOG_PANE,
            BTN_TUTO_PANE,
            BTN_MODULE_WORKFLOW,
            BTN_MODULE_PYTHON
        };

        /**
         * @brief
         *
         * @param parent
         */
        explicit CMainView(QWidget *parent = 0);

        ~CMainView();

        // Setters
        void                    setCustomGeometry(const QRect& rect);

        //Getters
        CProjectPane*           getProjectPane();
        CDoubleView*            getDoubleView();
        CProcessPane*           getProcessPane();
        CWorkflowPane*          getWorkflowPane();
        CInfoPane*              getInfoPane();
        CWizardPane*            getWizardPane();
        CWorkflowModuleWidget*  getWorkflowModule();
        CGraphicsToolbar*       getGraphicsToolbar() const;
        QSystemTrayIcon*        getNotifier();
        CNotificationPane*      getNotificationCenter();
        QAction*                getMainToolBarBtn(int id);
        QToolButton*            getBtn(int id);
        CHubDlg*                getHubView() const;
        CProcessPopupDlg*       getProcessPopup() const;
        CPreferencesDlg*        getPreferenceDlg();
        CUserLoginDlg*          getUserLoginDlg() const;

        void                    resizeEvent(QResizeEvent* event) override;
        void                    dragEnterEvent(QDragEnterEvent* e);
        void                    dropEvent(QDropEvent* e);

        void                    openHUB(const QString& algoName);

    signals:

        void                    doNewProject();

        void                    doLoadProject(QStringList filenames);
        void                    doLoadPlugin();
        void                    doLoadImages(QStringList index, const DatasetLoadPolicy& policy, const QModelIndex& currentIndex);
        void                    doLoadVideos(QStringList index, const QModelIndex& currentIndex);
        void                    doLoadStream(const QString& cameraId, const QString& cameraName, const QModelIndex& currentIndex);
        void                    doLoadFolder(const QString& dir, const QModelIndex& currentIndex);

        void                    doDeleteProgress(uint id);

        void                    doConnectUser(const QString& login, const QString& pwd, bool bRememberMe);
        void                    doDisconnectUser();

        void                    doBeforeProcessPopupOpen();
        void                    doAfterProcessPopupClose();

        void                    doCloseApp();

        void                    doDisplayLogMsg(int type, const QString& msg, const QString& categoryName);

        void                    doStartJupyterLab();

        //Plugin manager module
        void                    doGetPythonQueryModel();
        void                    doReloadAllPlugins();
        void                    doReloadPlugin(const QString& pluginName, int language);
        void                    doEditPythonPlugin(const QString& pluginName);
        void                    doShowPluginLocation(const QString& pluginName, int language);
        void                    doGetPythonDependencyModel(const QString& pluginName);
        void                    doInstallPythonPkg(const QString& moduleName);
        void                    doUpdatePythonPkg(const QString& moduleName);

    public slots:

        void                    onSetGraphicsContext(GraphicsContextPtr& context);
        void                    onSetCurrentUser(const CUser& user);
        void                    onSetWorkflowChangedIcon();

        void                    onGraphicsContextChanged();

        void                    onNewNotification(const QString& text, Notification type, CProgressCircle* pProgress = nullptr, int duration=1500);
        void                    onNewInfiniteProgress(const QString& text, CProgressCircle* pProgress);

        void                    onShowErrorMessage(const QString& errMsg);

        //Plugin manager module
        void                    onSetPythonQueryModel(CPluginPythonModel* pModel);
        void                    onSetPythonDependencyModel(CPluginPythonDependencyModel* pModel);

        void                    onCloseProject(bool bCurrentImgClosed);

        void                    onRestartIkomia();

    private slots:

        void                    onClose();
        void                    onMinimizeWindow();
        void                    onExpandWindow();
        void                    onShowPreferences();
        void                    onShowHub();
        void                    onShowProcessPopup();

    private:

        void                    init();
        void                    initDoubleView();
        void                    initTitleBarView();
        void                    initCentralView();
        void                    initUpView();
        void                    initBottomView();
        void                    initWorkflowModule();
        void                    initPluginMakerModule();
        void                    initJupyterLauncher();
        void                    initConnections();
        void                    initMainToolBar();
        void                    initLeftTab();
        void                    initRightTab();
        void                    initLeftPanes();
        void                    initRightPanes();
        void                    initGraphicsToolbar();
        void                    initNotifications();
        void                    initProgressBar();
        void                    initHubView();
        void                    initProcessPopup();
        void                    initLoginDialog();
        void                    initAllPath();

        QToolButton*            createLeftTabBtn(const QString& name, const QString& tooltip, const QIcon& icon, QWidget* pane);
        QToolButton*            createRightTabBtn(const QString& name, const QString& tooltip, const QIcon& icon, QWidget* pane);

        void                    animateWidget(QWidget* pWidget);

        void                    readSettings();

        void                    setAppStyle(const QString& style);

        bool                    eventFilter(QObject* obj, QEvent* event);

        void                    runTest();

        bool                    selectCamera(QString& mode, QString& name);

        void                    toggleGraphicsToolbar();
        void                    toggleGraphicsProperties();

        void                    showNotification(const QString& title, const QString& description, const QIcon& icon, CProgressCircle* pProgress, int duration);
        void                    showNotificationText(const QString& title, const QString& description, const QIcon& icon, int duration);
        void                    showNotificationProgress(const QString& title, const QString& description, CProgressCircle* pProgress, const QIcon& icon);
        void                    showNotificationCenter();
        void                    showLoginDialog();
        void                    showCustomMaximized();

        void                    hideGraphicsToolbar();

        void                    openImages(const QStringList& files);
        void                    openVideoFiles(const QStringList& files);
        void                    openProjectFiles(const QStringList& files);
        void                    openFolder(const QString& dir);

        void                    manageNotificationPos(CNotificationPopup* pNotif);
        void                    manageUserStatus();
        void                    manageOpenFiles(const QStringList& files);

        void                    clearTmpFolder();

    private:

        Ui::CMainView*                      ui;
        std::unique_ptr<CFramelessHelper>   m_pMainwViewMover = nullptr;
        CMainTitleBar*                      m_pTitleBarView = nullptr;
        QWidget*                            m_pCentralView = nullptr;
        QVBoxLayout*                        m_pMainLayout = nullptr;
        CModuleDockWidget*                  m_pModuleDock = nullptr;
        CCentralViewLayout*                 m_pCentralViewLayout = nullptr;
        QSplitter*                          m_pMainViewSplitter = nullptr;
        QAction*                            m_pOpenProjectAct = nullptr;
        QAction*                            m_pOpenImgAct = nullptr;
        QAction*                            m_pOpenVideoAct = nullptr;
        QAction*                            m_pOpenCamAct = nullptr;
        QAction*                            m_pOpenFolderAct = nullptr;
        QAction*                            m_pNewProjectAct = nullptr;
        QAction*                            m_pOpenHubAct = nullptr;
        QAction*                            m_pPluginAct = nullptr;
        QAction*                            m_pPreferencesAct = nullptr;
        CStackedPane*                       m_pLeftPanes = nullptr;
        CStackedPane*                       m_pRightPanes = nullptr;
        CProjectPane*                       m_pProjectPane = nullptr;
        CProcessPane*                       m_pProcessPane = nullptr;
        CWorkflowPane*                      m_pWorkflowPane = nullptr;
        CWizardPane*                        m_pWizardPane = nullptr;
        CInfoPane*                          m_pInfoPane = nullptr;
        CProgressCircleManager              m_circleProgressMgr;
        CDoubleView*                        m_pDoubleView = nullptr;
        CGraphicsToolbar*                   m_pGraphicsToolbar = nullptr;
        CGraphicsPropertiesWidget*          m_pGraphicsProperties = nullptr;
        CWorkflowModuleWidget*              m_pWorkflowModule = nullptr;
        CPluginManagerWidget*               m_pPluginMgrModule = nullptr;
        QLabel*                             m_pTimer = nullptr;
        QSystemTrayIcon*                    m_pNotifier = nullptr;
        QToolButton*                        m_pNotifyBtn = nullptr;
        QToolButton*                        m_pTutoBtn = nullptr;
        QToolButton*                        m_pUserLoginBtn = nullptr;
        CNotificationPane*                  m_pNotificationPane = nullptr;
        CProgressCircle*                    m_currentProgress = nullptr;
        CNotificationPopupFactory           m_notifFactory;
        CPreferencesDlg                     m_preferencesDlg;
        CUserLoginDlg*                      m_pUserLoginDlg = nullptr;
        CHubDlg*                            m_pHubDlg = nullptr;
        CProcessPopupDlg*                   m_pProcessDlg = nullptr;
        QToolButton*                        m_pProjectBtn = nullptr;
        QToolButton*                        m_pProcessBtn = nullptr;
        QToolButton*                        m_pWorkflowBtn = nullptr;
        QToolButton*                        m_pPropertiesBtn = nullptr;
        QErrorMessage*                      m_pErrMsgDlg = nullptr;
        CUser                               m_currentUser;
        QString                             m_style = "";
        QString                             m_lastPathImg = "";
        QString                             m_lastPathVideo = "";
        QString                             m_lastPathProject = "";
        int                                 m_currWidth = 2000;
        int                                 m_titleBarSize = 20;
        int                                 m_notifPopupPadding = m_titleBarSize;
        QList<CNotificationPopup*>          m_popupList;
        bool                                m_bIsMaximized = true;
        QRect                               m_oldGeometry;
};

#endif // CMAINVIEW_H
