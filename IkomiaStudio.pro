#-------------------------------------------------
#
# Project created by QtCreator 2017-06-23T11:27:52
#
#-------------------------------------------------
include(IkomiaStudio.pri)

QT += core gui concurrent sql opengl network svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ikomia
TEMPLATE = app
VERSION = 0.4.1.0
DEFINES += BOOST_ALL_NO_LIB __STDC_FORMAT_MACROS
macx: DEFINES += Q_OS_MACOS

# define breakpad server    SOCORRO / CALIPER
DEFINES += SOCORRO
DEFINES += QT_QTPROPERTYBROWSER_IMPORT

# Disable Qt Quick compiler because we don't use QML, but more importantly so that
# the source code of the .js files is not removed from the embedded qrc file.
CONFIG -= qtquickcompiler

# Use Precompiled headers (PCH)ls
CONFIG += precompile_header
PRECOMPILED_HEADER = Main/pch.hpp

# ICON management
win32: RC_ICONS = $$PWD/Images/app.ico
macx: ICON = $$PWD/Images/ikomia.icns

SOURCES += \
        Model/CDbManager.cpp \
        Model/CMainModel.cpp \
        Model/CMultiModel.cpp \
        Model/Crash/QBreakpadHandler.cpp \
        Model/Crash/QBreakpadHttpUploader.cpp \
        Model/Data/CMultiImageModel.cpp \
        Model/Plugin/CPluginManager.cpp \
        Model/Plugin/CPluginPythonDependencyModel.cpp \
        Model/Plugin/CPluginPythonModel.cpp \
        Model/Plugin/CPluginTools.cpp \
        Model/Matomo/piwiktracker.cpp \
        Model/Project/CMultiProjectModel.cpp \
        Model/Project/CProjectDataProxyModel.cpp \
        Model/Project/CProjectManager.cpp \
        Model/Project/CProjectModel.cpp \
        Model/Project/CProjectGraphicsProxyModel.cpp \
        Model/Project/CProjectDbMgrRegistration.cpp \
        Model/Project/CProjectItemDbMgr.cpp \
        Model/Project/CFolderItemDbMgr.cpp \
        Model/Project/CDatasetItemDbMgr.cpp \
        Model/Project/CDimensionItemDbMgr.cpp \
        Model/Project/CProjectExportMgr.cpp \
        Model/Process/CProcessManager.cpp \
        Model/Project/CProjectViewProxyModel.cpp \
        Model/Protocol/CProtocolInput.cpp \
        Model/Protocol/CProtocolManager.cpp \
        Model/Protocol/CProtocolDBManager.cpp \
        Model/Protocol/CProtocolInputViewManager.cpp \
        Model/Protocol/CProtocolRunManager.cpp \
        Model/Render/CRenderManager.cpp \
        Model/Render/C3dAnimation.cpp \
        Model/ProgressBar/CProgressBarManager.cpp \
        Model/Graphics/CGraphicsManager.cpp \
        Model/Graphics/CGraphicsRegistration.cpp \
        Model/Graphics/CGraphicsDbManager.cpp \
        Model/Results/CResultManager.cpp \
        Model/Results/CResultDbManager.cpp \
        Model/User/CUserManager.cpp \
        Model/User/CUserSqlQueryModel.cpp \
        Model/User/CUser.cpp \
        Model/Data/CFeaturesTableModel.cpp \
        Model/Data/CMeasuresTableModel.cpp \
        Model/Data/Video/CLiveStreamItemDbMgr.cpp \
        Model/Data/Video/CVideoItemDbMgr.cpp \
        Model/Data/Video/CVideoManager.cpp \
        Model/Data/Image/CImageItemDbMgr.cpp \
        Model/Data/Image/CImgManager.cpp \
        Model/Data/CMainDataManager.cpp \
        Model/Store/CStoreManager.cpp \
        Model/Store/CStoreQueryModel.cpp \
        Model/Store/CStoreDbManager.cpp \
        Model/Store/CStoreOnlineIconManager.cpp \
        Model/Wizard/CWizardManager.cpp \
        Model/Wizard/CWizardScenario.cpp \
        Model/Wizard/CWizardDbManager.cpp \
        Model/Wizard/CWizardQueryModel.cpp \
        Model/Wizard/CWizardStepModel.cpp \
        Model/Settings/CSettingsManager.cpp \
        View/Common/CCrashReporDlg.cpp \
        View/Common/CImageListView.cpp \
        View/Common/CSvgButton.cpp \
        View/DoubleView/3D/CVolumeViewSync.cpp \
        View/DoubleView/Image/CMultiImageDisplay.cpp \
        View/Modules/PluginManager/CCppNewPluginWidget.cpp \
        View/Modules/PluginManager/CPythonNewPluginDlg.cpp \
        View/Modules/PluginManager/CPythonPluginManagerWidget.cpp \
        View/Modules/PluginManager/CPluginManagerWidget.cpp \
        View/Modules/PluginManager/CPythonPluginMaker.cpp \
        View/Modules/PluginManager/CCppPluginMaker.cpp \
        View/Modules/Protocol/CProtocolView.cpp \
        View/Modules/Protocol/CProtocolScene.cpp \
        View/Modules/Protocol/CProtocolItem.cpp \
        View/Modules/Protocol/CProtocolPortItem.cpp \
        View/Modules/Protocol/CProtocolConnection.cpp \
        View/Modules/Protocol/CProtocolLabelArea.cpp \
        View/Modules/Protocol/CProtocolDummyItem.cpp \
        View/Modules/Protocol/CProtocolModuleWidget.cpp \
        View/Modules/Protocol/CProtocolIOArea.cpp \
        View/Modules/Protocol/CProtocolInputTypeDlg.cpp \
        View/Modules/Protocol/CProtocolNewDlg.cpp \
        View/Modules/Protocol/CGraphicsDeletableButton.cpp \
        View/Modules/CModuleDockWidget.cpp \
        View/Project/CProjectPane.cpp \
        View/Project/CProjectTreeView.cpp \
        View/Project/CDatasetLoadPolicyDlg.cpp \
        View/Process/CProcessPane.cpp \
        View/Process/CProcessParameters.cpp \
        View/Process/CProcessPopupDlg.cpp \
        View/Process/CProcessListView.cpp \
        View/Process/CProcessListViewDelegate.cpp \
        View/Process/CProcessDocFrame.cpp \
        View/Process/CProcessDocContent.cpp \
        View/Process/CProcessDocDlg.cpp \
        View/Process/CProcessEditDocFrame.cpp \
        View/Process/CProcessDocWidget.cpp \
        View/Process/CProcessListPopup.cpp \
        View/Common/CBorderLayout.cpp \
        View/Common/CDockWidgetTitleBar.cpp \
        View/Common/CFramelessHelper.cpp \
        View/Common/CGLWidget.cpp \
        View/Common/CPane.cpp \
        View/Common/CProgressCircle.cpp \
        View/Common/CResizeStackedWidget.cpp \
        View/Common/CRollupWidget.cpp \
        View/Common/CStackedPane.cpp \
        View/Common/CInfoPane.cpp \
        View/Common/CRollupBtn.cpp \
        View/Common/CHtmlDelegate.cpp \
        View/Common/CCameraDlg.cpp \
        View/Common/CProgressCircleManager.cpp \
        View/Common/CBubbleTip.cpp \
        View/Common/CDialog.cpp \
        View/Common/CToolbarBorderLayout.cpp \
        View/Common/CListViewDelegate.cpp \
        View/Main/CCentralViewLayout.cpp \
        View/Main/CMainTitleBar.cpp \
        View/Main/CMainView.cpp \
        View/Main/CUserLoginDlg.cpp \
        View/Main/CResponsiveUI.cpp \
        View/DoubleView/CDoubleView.cpp \
        View/DoubleView/Image/CImageDisplay.cpp \
        View/DoubleView/Image/CImageScene.cpp \
        View/DoubleView/Image/CImageView.cpp \
        View/DoubleView/Image/CImageViewSync.cpp \
        View/DoubleView/Result/CResultTableView.cpp \
        View/DoubleView/Result/CResultTableDisplay.cpp \
        View/DoubleView/Result/CResultsViewer.cpp \
        View/DoubleView/3D/C3dDisplay.cpp \
        View/DoubleView/3D/C3dAnimationDlg.cpp \
        View/DoubleView/CDataDisplay.cpp \
        View/DoubleView/CDataDisplayLayout.cpp \
        View/DoubleView/CDataListView.cpp \
        View/DoubleView/CDataListViewDelegate.cpp \
        View/DoubleView/CDataViewer.cpp \
        View/DoubleView/CPathNavigator.cpp \
        View/DoubleView/CStaticDisplay.cpp \
        View/DoubleView/Plot/CPlotDisplay.cpp \
        View/DoubleView/Video/CVideoDisplay.cpp \
        View/DoubleView/Video/CVideoViewSync.cpp \
        View/DoubleView/CWidgetDataDisplay.cpp \
        View/DoubleView/Image/CImageExportDlg.cpp \
        View/DoubleView/3D/CVolumeDisplay.cpp \
        View/Graphics/CGraphicsToolbar.cpp \
        View/Graphics/CGraphicsLayerChoiceDlg.cpp \
        View/Graphics/CGraphicsPropertiesWidget.cpp \
        View/Graphics/CGraphicsEllipsePropWidget.cpp \
        View/Graphics/CGraphicsPointPropWidget.cpp \
        View/Graphics/CGraphicsRectPropWidget.cpp \
        View/Graphics/CGraphicsPolygonPropWidget.cpp \
        View/Graphics/CGraphicsPolylinePropWidget.cpp \
        View/Graphics/CGraphicsTextPropWidget.cpp \
        View/Preferences/CPreferencesDlg.cpp \
        View/Preferences/CUserManagementWidget.cpp \
        View/Preferences/CNewUserDlg.cpp \
        View/Preferences/CGeneralSettingsWidget.cpp \
        View/Preferences/CProtocolSettingsWidget.cpp \
        View/Protocol/CProtocolPane.cpp \
        View/Protocol/CProtocolInfoDlg.cpp \
        View/Store/CStoreDlg.cpp \
        View/Store/CStorePluginListView.cpp \
        View/Store/CStorePluginListViewDelegate.cpp \
        View/Notifications/CNotificationPopup.cpp \
        View/Notifications/CNotificationPane.cpp \
        View/Wizard/CWizardPane.cpp \
        View/Wizard/CWizardTutoListView.cpp \
        View/Wizard/CWizardTutoListViewDelegate.cpp \
        View/Wizard/CWizardDocPage.cpp \
        View/Wizard/CWizardStepPage.cpp \
        View/Wizard/CWizardDocFrame.cpp \
        View/Wizard/CWizardStepListView.cpp \
        View/Wizard/CWizardStepListViewDelegate.cpp \
        Controller/CMainCtrl.cpp \
        Main/main.cpp \
        Main/LogCategory.cpp

HEADERS += \
        Model/CDbManager.h \
        Model/CMainModel.h \
        Model/CTreeItem.hpp \
        Model/CItem.hpp \
        Model/CMultiModel.h \
        Model/CTreeModel.hpp \
        Model/Crash/QBreakpadHandler.h \
        Model/Crash/QBreakpadHttpUploader.h \
        Model/Crash/Singleton/call_once.h \
        Model/Crash/Singleton/singleton.h \
        Model/Data/CFeaturesTableModel.h \
        Model/Data/CMeasuresTableModel.h \
        Model/Data/CMultiImageModel.h \
        Model/Plugin/CPluginManager.h \
        Model/Plugin/CPluginPythonDependencyModel.h \
        Model/Plugin/CPluginPythonModel.h \
        Model/Plugin/CPluginTools.h \
        Model/Matomo/piwiktracker.h \
        Model/Project/CDimensionItem.hpp \
        Model/Project/CDatasetItem.hpp \
        Model/Project/CMultiProjectModel.h \
        Model/Project/CProjectDataProxyModel.h \
        Model/Project/CProjectDbManager.hpp \
        Model/Project/CProjectItem.hpp \
        Model/Project/CProjectManager.h \
        Model/Project/CProjectUtils.hpp \
        Model/Project/CProjectModel.h \
        Model/Project/CProjectGraphicsProxyModel.h \
        Model/Project/CFolderItem.hpp \
        Model/Project/CProjectDbMgrRegistration.h \
        Model/Project/CProjectItemDbMgr.h \
        Model/Project/CProjectDbMgrInterface.hpp \
        Model/Project/CFolderItemDbMgr.h \
        Model/Project/CDatasetItemDbMgr.h \
        Model/Project/CDimensionItemDbMgr.h \
        Model/Project/CProjectExportMgr.h \
        Model/Process/CProcessItem.hpp \
        Model/Process/CProcessManager.h \
        Model/Process/CProcessModel.hpp \
        Model/Project/CProjectViewProxyModel.h \
        Model/Protocol/CProtocolInput.h \
        Model/Protocol/CProtocolManager.h \
        Model/Protocol/CProtocolDBManager.h \
        Model/Protocol/CProtocolInputViewManager.h \
        Model/Protocol/CProtocolRunManager.h \
        Model/Render/CRenderManager.h \
        Model/Render/C3dAnimation.h \
        Model/ProgressBar/CProgressBarManager.h \
        Model/Graphics/CGraphicsManager.h \
        Model/Graphics/CGraphicsLayerItem.hpp \
        Model/Graphics/CGraphicsRegistration.h \
        Model/Graphics/CGraphicsDbManager.h \
        Model/Graphics/CGraphicsLayerInfo.hpp \
        Model/Graphics/CGraphicsLayerInfo.hpp \
        Model/Results/CResultItem.hpp \
        Model/Results/CResultManager.h \
        Model/Results/CResultDbManager.h \
        Model/User/CUserManager.h \
        Model/User/CUserSqlQueryModel.h \
        Model/User/CUser.h \
        Model/Wizard/CWizardManager.h \
        Model/Wizard/CWizardScenario.h \
        Model/Wizard/CWizardDbManager.h \
        Model/Wizard/CWizardQueryModel.h \
        Model/Wizard/Tutorials/CScenarioSimpleImage.hpp \
        Model/Wizard/Tutorials/CTutoOpticalFlow.hpp \
        Model/Wizard/CWizardStepModel.h \
        Model/Data/Video/CLiveStreamItemDbMgr.h \
        Model/Data/Video/CVideoItem.hpp \
        Model/Data/Video/CVideoItemDbMgr.h \
        Model/Data/Video/CVideoManager.h \
        Model/Data/Image/CImageItem.hpp \
        Model/Data/Image/CImageItemDbMgr.h \
        Model/Data/Image/CImgManager.h \
        Model/Data/Video/CLiveStreamItem.hpp \
        Model/Data/CMainDataManager.h \
        Model/Store/CStoreManager.h \
        Model/Store/CStoreQueryModel.h \
        Model/Store/CStoreDbManager.h \
        Model/Store/CStoreOnlineIconManager.h \
        Model/Settings/CSettingsManager.h \
        Model/Wizard/Tutorials/CTutoStartingHelper.hpp \
        View/Common/CCrashReporDlg.h \
        View/Common/CImageListView.h \
        View/Common/CSvgButton.h \
        View/DoubleView/3D/CVolumeViewSync.h \
        View/DoubleView/Image/CMultiImageDisplay.h \
        View/Modules/CModuleDockWidget.h \
        View/Modules/PluginManager/CCppNewPluginWidget.h \
        View/Modules/PluginManager/CPythonNewPluginDlg.h \
        View/Modules/PluginManager/CPythonPluginManagerWidget.h \
        View/Modules/PluginManager/PluginManagerDefine.hpp \
        View/Modules/Protocol/CProtocolView.h \
        View/Modules/Protocol/CProtocolScene.h \
        View/Modules/Protocol/CProtocolItem.h \
        View/Modules/Protocol/CProtocolPortItem.h \
        View/Modules/Protocol/CProtocolConnection.h \
        View/Modules/Protocol/CProtocolLabelArea.h \
        View/Modules/Protocol/CProtocolDummyItem.h \
        View/Modules/Protocol/CProtocolModuleWidget.h \
        View/Modules/Protocol/CProtocolIOArea.h \
        View/Modules/Protocol/CProtocolInputTypeDlg.h \
        View/Modules/Protocol/CProtocolNewDlg.h \
        View/Modules/Protocol/CGraphicsDeletableButton.h \
        View/Modules/PluginManager/CPluginManagerWidget.h \
        View/Modules/PluginManager/CPythonPluginMaker.h \
        View/Modules/PluginManager/CCppPluginMaker.h \
        View/Project/CProjectPane.h \
        View/Project/CProjectTreeView.h \
        View/Project/CDatasetLoadPolicyDlg.h \
        View/Process/CProcessPane.h \
        View/Process/CProcessParameters.h \
        View/Process/CProcessPopupDlg.h \
        View/Process/CProcessListView.h \
        View/Process/CProcessListViewDelegate.h \
        View/Process/CProcessDocFrame.h \
        View/Process/CProcessDocContent.h \
        View/Process/CProcessDocDlg.h \
        View/Process/CProcessEditDocFrame.h \
        View/Process/CProcessDocWidget.h \
        View/Process/CProcessListPopup.h \
        View/Common/CBorderLayout.h \
        View/Common/CContextMenu.hpp \
        View/Common/CDockWidgetTitleBar.h \
        View/Common/CFramelessHelper.h \
        View/Common/CGLWidget.h \
        View/Common/CGraphicsItemTools.hpp \
        View/Common/CPane.h \
        View/Common/CProgressCircle.h \
        View/Common/CResizeStackedWidget.h \
        View/Common/CRollupWidget.h \
        View/Common/CStackedPane.h \
        View/Common/CInfoPane.h \
        View/Common/CRollupBtn.h \
        View/Common/CHtmlDelegate.h \
        View/Common/CCameraDlg.h \
        View/Common/CProgressCircleManager.h \
        View/Common/CBubbleTip.h \
        View/Common/CDialog.h \
        View/Common/CToolbarBorderLayout.h \
        View/Common/CListViewDelegate.h \
        View/Main/CMainTitleBar.h \
        View/Main/CMainView.h \
        View/Main/CCentralViewLayout.h \
        View/Main/CUserLoginDlg.h \
        View/Main/CResponsiveUI.h \
        View/DoubleView/CDoubleView.h \
        View/DoubleView/Image/CImageDisplay.h \
        View/DoubleView/Image/CImageScene.h \
        View/DoubleView/Image/CImageView.h \
        View/DoubleView/Image/CImageViewSync.h \
        View/DoubleView/Image/CImageExportDlg.h \
        View/DoubleView/3D/C3dDisplay.h \
        View/DoubleView/3D/C3dAnimationDlg.h \
        View/DoubleView/3D/CVolumeDisplay.h \
        View/DoubleView/Result/CResultsViewer.h \
        View/DoubleView/Result/CResultTableView.h \
        View/DoubleView/Result/CResultTableDisplay.h \
        View/DoubleView/CDataDisplay.h \
        View/DoubleView/CDataDisplayLayout.h \
        View/DoubleView/CDataListView.h \
        View/DoubleView/CDataListViewDelegate.h \
        View/DoubleView/CDataViewer.h \
        View/DoubleView/CPathNavigator.h \
        View/DoubleView/CStaticDisplay.h \
        View/DoubleView/Plot/CPlotDisplay.h \
        View/DoubleView/Video/CVideoDisplay.h \
        View/DoubleView/Video/CVideoViewSync.h \
        View/DoubleView/CWidgetDataDisplay.h \
        View/Graphics/CGraphicsToolbar.h \
        View/Graphics/CGraphicsLayerChoiceDlg.h \
        View/Graphics/CGraphicsPropertiesWidget.h \
        View/Graphics/CGraphicsEllipsePropWidget.h \
        View/Graphics/CGraphicsPointPropWidget.h \
        View/Graphics/CGraphicsRectPropWidget.h \
        View/Graphics/CGraphicsPolygonPropWidget.h \
        View/Graphics/CGraphicsPolylinePropWidget.h \
        View/Graphics/CGraphicsTextPropWidget.h \
        View/Preferences/CUserManagementWidget.h \
        View/Preferences/CPreferencesDlg.h \
        View/Preferences/CNewUserDlg.h \
        View/Preferences/CGeneralSettingsWidget.h \
        View/Preferences/CProtocolSettingsWidget.h \
        View/Wizard/CWizardPane.h \
        View/Wizard/CWizardTutoListView.h \
        View/Wizard/CWizardTutoListViewDelegate.h \
        View/Wizard/CWizardDocPage.h \
        View/Wizard/CWizardStepPage.h \
        View/Wizard/CWizardDocFrame.h \
        View/Wizard/CWizardStepListView.h \
        View/Wizard/CWizardStepListViewDelegate.h \
        View/Protocol/CProtocolPane.h \
        View/Protocol/CProtocolInfoDlg.h \
        View/Store/CStoreDlg.h \
        View/Store/CStorePluginListView.h \
        View/Store/CStorePluginListViewDelegate.h \
        View/Notifications/CNotificationPopup.h \
        View/Notifications/CNotificationPane.h \
        Controller/CMainCtrl.h \
        Main/AppDefine.hpp \
        Main/AppTools.hpp \
        Main/forwards.hpp \
        Main/pch.hpp \
        Main/PyLogHook.hpp \
        Main/pch.hpp \
        Main/LogCategory.h

FORMS += \
        CMainView.ui

RESOURCES += \
        res.qrc

#Use of 'lupdate IkomiaStudio.pro' to update translation files
#Use of 'lrelease IkomiaStudio.pro' to generate .qm files
TRANSLATIONS = Translations/ikomia_fr.ts

#Make install directive
target.path = Build/Bin/
INSTALLS += target

#Max OS X: include OpenCL header in .../Developpemen/CL/ -> must be placed manually
macx:INCLUDEPATH += $$PWD/../..

########
#Python
########
LIBS += $$link_python()

########
#BOOST
########
LIBS += $$link_boost()

##########################
#Dynamic link with OpenCV
##########################
win32:CONFIG(release, debug|release): OPENCV_VERSION = $${OPENCV_VERSION}
win32:CONFIG(debug, debug|release): OPENCV_VERSION = $${OPENCV_VERSION}d
LIBS += $$link_opencv()

#####################
#Dynamic link to VTK
#####################
LIBS += $$link_vtk()

########
#OpenCL
########
win32: LIBS += -lOpenCL
else:unix:!macx: LIBS += -lOpenCL
macx: LIBS += -framework OpenCL

########
#OpenGL
########
win32: LIBS += -lOpenGL32

######
#Curl
######
unix: LIBS += -lcurl

#######################
#Dynamic link to Utils
#######################
LIBS += $$link_utils()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/Utils
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/Utils

######################
#Dynamic link to Core
######################
LIBS += $$link_core()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/Core
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/Core

#############################
#Dynamic link to DataProcess
#############################
LIBS += $$link_dataprocess()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/DataProcess
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/DataProcess

########################
#Dynamic link to DataIO
########################
LIBS += $$link_dataio()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/DataIO
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/DataIO

################################
#Dynamic link to DataManagement
################################
LIBS += $$link_datamanagement()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/DataManagement
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/DataManagement

##############################
#Dynamic link to VolumeRender
##############################
LIBS += $$link_volumerender()
LIBS += $$link_dataprocess()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/VolumeRender
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/VolumeRender

##########################
#Dynamic link to DataPlot
##########################
LIBS += $$link_dataplot()
INCLUDEPATH += $$IKOMIA_API_ROOT_PATH/DataPlot
DEPENDPATH += $$IKOMIA_API_ROOT_PATH/DataPlot

#####################
#Dynamic link to Qwt
#####################
centos7 {
    LIBS += -lqwt -lquazip1-qt5 -lbreakpad_client
    INCLUDEPATH += $$IKOMIA_EXT_LIB_PATH/build/breakpad/src/breakpad/src/
    INCLUDEPATH += /work/shared/local/include/QuaZip-Qt5-1.1/quazip
} else {
    win32:CONFIG(release, debug|release): LIBS += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwt
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwtd
    else:unix:!macx: LIBS += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwt
    else:macx: LIBS += -F$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -framework qwt
    INCLUDEPATH += $$IKOMIA_EXT_LIB_PATH/qwt/src

    ########################
    #Dynamic link to Quazip
    ########################
    win32:CONFIG(release, debug|release): LIBS += -L$$IKOMIA_EXT_LIB_PATH/quazip/build/lib/ -lquazip
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$IKOMIA_EXT_LIB_PATH/quazip/build/lib/ -lquazipd
    else:unix:!macx: LIBS += -L$$IKOMIA_EXT_LIB_PATH/quazip/build/lib/ -lquazip
    else:macx: LIBS += -L$$IKOMIA_EXT_LIB_PATH/quazip/build/lib/ -lquazip
    INCLUDEPATH += $$IKOMIA_EXT_LIB_PATH/quazip/quazip
    DEPENDPATH += $$IKOMIA_EXT_LIB_PATH/quazip/build/lib/

    ########################
    #breakpad (static link)
    ########################
    win32:CONFIG(release, debug|release):LIBS += -L$$IKOMIA_EXT_LIB_PATH/breakpad/src/client/windows/Release/lib -lexception_handler -lcrash_generation_client -lcommon
    win32:CONFIG(debug, debug|release):LIBS += -L$$IKOMIA_EXT_LIB_PATH/breakpad/src/client/windows/Debug/lib -lexception_handler -lcrash_generation_client -lcommon
    unix:!macx:LIBS += -L$$IKOMIA_EXT_LIB_PATH/breakpad/src/client/linux/ -lbreakpad_client
    macx:QMAKE_LFLAGS += -F$$IKOMIA_EXT_LIB_PATH/breakpad/src/client/mac/build/Release
    macx:LIBS += -framework Breakpad
    INCLUDEPATH += $$IKOMIA_EXT_LIB_PATH/breakpad/src/
}

######################
#Dynamic link to gmic
######################
LIBS += -lgmic
