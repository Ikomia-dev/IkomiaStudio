cmake_minimum_required(VERSION 3.16)
set(IKOMIA_VERSION 0.10.0)
project(IkomiaStudio VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/Build)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/Bin)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
include(GNUInstallDirs)
include(Versions.cmake)
include(LocalSettings.cmake)

# Qt
find_package(Qt5 REQUIRED COMPONENTS Core Concurrent Gui Widgets Network OpenGL Sql Svg LinguistTools)

# Python
if(CENTOS7)
    set(Python3_DIR "/work/shared/local")
endif()

find_package(Python3 ${PYTHON_VERSION_DOT} EXACT REQUIRED COMPONENTS Interpreter Development NumPy)

# OpenMP
find_package(OpenMP REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# OpenCV
if(WIN32)
    if(${IK_CPU})
        set(OpenCV_DIR "C:/Program Files/OpenCV/cpu/x64/vc${MSVC_VERSION}/lib")
    else()
        set(OpenCV_DIR "C:/Program Files/OpenCV/cuda/x64/vc${MSVC_VERSION}/lib")
    endif()
endif()

find_package(OpenCV ${OPENCV_VERSION} REQUIRED)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if(CENTOS7)
    set(BUILD_SHARED_LIBS ON)
    set(BOOST_PYTHON_TARGET Boost::python)
    find_package(Boost REQUIRED COMPONENTS system filesystem python)
else()
    set(BOOST_PYTHON_TARGET Boost::python${PYTHON_VERSION_NO_DOT})
    find_package(Boost REQUIRED COMPONENTS system filesystem python${PYTHON_VERSION_NO_DOT})
endif()



###################################################
##                                               ##
##          Looking for the VTK library          ##
##                                               ##
###################################################

find_package(VTK ${VTK_MAJOR_MINOR_VERSION} REQUIRED
    COMPONENTS
        vtkCommonCore
        vtkCommonColor
        vtkCommonDataModel
        vtkCommonExecutionModel
        vtkCommonMath
        vtkCommonSystem
        vtkCommonTransforms
        vtkDICOM
        vtkFiltersCore
        vtkFiltersGeneral
        vtkFiltersImaging
        vtkGUISupportQt
        vtkImagingColor
        vtkImagingCore
        vtkImagingGeneral
        vtkImagingMath
        vtkImagingSources
        vtkInteractionImage
        vtkInteractionStyle
        vtkIOCore
        vtkIOImage
        vtkRenderingContext2D
        vtkRenderingCore
        vtkRenderingImage
        vtkRenderingOpenGL2
        vtksys
        vtkViewsCore
)

if(VTK_FOUND)
    include(${VTK_USE_FILE})
    message("-- Found VTK: " ${VTK_USE_FILE} " (found suitable exact version \"${VTK_VERSION}\")")
else()
	message ("-- VTK not found")
endif(VTK_FOUND)



add_executable(IkomiaStudio WIN32 MACOSX_BUNDLE
    Controller/CMainCtrl.cpp
    Controller/CMainCtrl.h
    Main/AppDefine.hpp
    Main/AppTools.hpp
    Main/LogCategory.cpp
    Main/LogCategory.h
    Main/PyLogHook.hpp
    Main/forwards.hpp
    Main/main.cpp
    Main/pch.hpp
    Model/CDbManager.cpp
    Model/CDbManager.h
    Model/CItem.hpp
    Model/CMainModel.cpp
    Model/CMainModel.h
    Model/CMultiModel.cpp
    Model/CMultiModel.h
    Model/CTrainingMonitoring.cpp
    Model/CTrainingMonitoring.h
    Model/CTreeItem.hpp
    Model/CTreeModel.hpp
    Model/Crash/QBreakpadHandler.cpp
    Model/Crash/QBreakpadHandler.h
    Model/Crash/QBreakpadHttpUploader.cpp
    Model/Crash/QBreakpadHttpUploader.h
    Model/Crash/Singleton/call_once.h
    Model/Crash/Singleton/singleton.h
    Model/Data/CFeaturesTableModel.cpp
    Model/Data/CFeaturesTableModel.h
    Model/Data/CMainDataManager.cpp
    Model/Data/CMainDataManager.h
    Model/Data/CMeasuresTableModel.cpp
    Model/Data/CMeasuresTableModel.h
    Model/Data/CMultiImageModel.cpp
    Model/Data/CMultiImageModel.h
    Model/Data/Image/CImageItem.hpp
    Model/Data/Image/CImageItemDbMgr.cpp
    Model/Data/Image/CImageItemDbMgr.h
    Model/Data/Image/CImgManager.cpp
    Model/Data/Image/CImgManager.h
    Model/Data/Video/CLiveStreamItem.hpp
    Model/Data/Video/CLiveStreamItemDbMgr.cpp
    Model/Data/Video/CLiveStreamItemDbMgr.h
    Model/Data/Video/CVideoItem.hpp
    Model/Data/Video/CVideoItemDbMgr.cpp
    Model/Data/Video/CVideoItemDbMgr.h
    Model/Data/Video/CVideoManager.cpp
    Model/Data/Video/CVideoManager.h
    Model/Graphics/CGraphicsDbManager.cpp
    Model/Graphics/CGraphicsDbManager.h
    Model/Graphics/CGraphicsLayerInfo.hpp
    Model/Graphics/CGraphicsLayerItem.hpp
    Model/Graphics/CGraphicsManager.cpp
    Model/Graphics/CGraphicsManager.h
    Model/Matomo/piwiktracker.cpp
    Model/Matomo/piwiktracker.h
    Model/Plugin/CPluginManager.cpp
    Model/Plugin/CPluginManager.h
    Model/Plugin/CPluginPythonDependencyModel.cpp
    Model/Plugin/CPluginPythonDependencyModel.h
    Model/Plugin/CPluginPythonModel.cpp
    Model/Plugin/CPluginPythonModel.h
    Model/Process/CProcessItem.hpp
    Model/Process/CProcessManager.cpp
    Model/Process/CProcessManager.h
    Model/Process/CProcessModel.hpp
    Model/ProgressBar/CProgressBarManager.cpp
    Model/ProgressBar/CProgressBarManager.h
    Model/Project/CDatasetItem.hpp
    Model/Project/CDatasetItemDbMgr.cpp
    Model/Project/CDatasetItemDbMgr.h
    Model/Project/CDimensionItem.hpp
    Model/Project/CDimensionItemDbMgr.cpp
    Model/Project/CDimensionItemDbMgr.h
    Model/Project/CFolderItem.hpp
    Model/Project/CFolderItemDbMgr.cpp
    Model/Project/CFolderItemDbMgr.h
    Model/Project/CMultiProjectModel.cpp
    Model/Project/CMultiProjectModel.h
    Model/Project/CProjectDataProxyModel.cpp
    Model/Project/CProjectDataProxyModel.h
    Model/Project/CProjectDbManager.hpp
    Model/Project/CProjectDbMgrInterface.hpp
    Model/Project/CProjectDbMgrRegistration.cpp
    Model/Project/CProjectDbMgrRegistration.h
    Model/Project/CProjectExportMgr.cpp
    Model/Project/CProjectExportMgr.h
    Model/Project/CProjectGraphicsProxyModel.cpp
    Model/Project/CProjectGraphicsProxyModel.h
    Model/Project/CProjectItem.hpp
    Model/Project/CProjectItemDbMgr.cpp
    Model/Project/CProjectItemDbMgr.h
    Model/Project/CProjectManager.cpp
    Model/Project/CProjectManager.h
    Model/Project/CProjectModel.cpp
    Model/Project/CProjectModel.h
    Model/Project/CProjectUtils.hpp
    Model/Project/CProjectViewProxyModel.cpp
    Model/Project/CProjectViewProxyModel.h
    Model/Render/C3dAnimation.cpp
    Model/Render/C3dAnimation.h
    Model/Render/CRenderManager.cpp
    Model/Render/CRenderManager.h
    Model/Results/CResultDbManager.cpp
    Model/Results/CResultDbManager.h
    Model/Results/CResultItem.hpp
    Model/Results/CResultManager.cpp
    Model/Results/CResultManager.h
    Model/Settings/CSettingsManager.cpp
    Model/Settings/CSettingsManager.h
    Model/Hub/CHubDbManager.cpp
    Model/Hub/CHubDbManager.h
    Model/Hub/CHubManager.cpp
    Model/Hub/CHubManager.h
    Model/Hub/CHubOnlineIconManager.cpp
    Model/Hub/CHubOnlineIconManager.h
    Model/Hub/CHubQueryModel.cpp
    Model/Hub/CHubQueryModel.h
    Model/Hub/CPluginModel.h
    Model/Hub/CPluginModel.cpp
    Model/User/CUser.cpp Model/User/CUser.h
    Model/User/CUserManager.cpp
    Model/User/CUserManager.h
    Model/User/CUserSqlQueryModel.cpp
    Model/User/CUserSqlQueryModel.h
    Model/User/CUserNamespace.h
    Model/User/CUserNamespace.cpp
    Model/Wizard/CWizardDbManager.cpp
    Model/Wizard/CWizardDbManager.h
    Model/Wizard/CWizardManager.cpp
    Model/Wizard/CWizardManager.h
    Model/Wizard/CWizardQueryModel.cpp
    Model/Wizard/CWizardQueryModel.h
    Model/Wizard/CWizardScenario.cpp
    Model/Wizard/CWizardScenario.h
    Model/Wizard/CWizardStepModel.cpp
    Model/Wizard/CWizardStepModel.h
    Model/Wizard/Tutorials/CScenarioSimpleImage.hpp
    Model/Wizard/Tutorials/CTutoOpticalFlow.hpp
    Model/Wizard/Tutorials/CTutoStartingHelper.hpp
    Model/Workflow/CWorkflowDBManager.cpp
    Model/Workflow/CWorkflowDBManager.h
    Model/Workflow/CWorkflowInput.cpp
    Model/Workflow/CWorkflowInput.h
    Model/Workflow/CWorkflowInputViewManager.cpp
    Model/Workflow/CWorkflowInputViewManager.h
    Model/Workflow/CWorkflowManager.cpp
    Model/Workflow/CWorkflowManager.h
    Model/Workflow/CWorkflowRunManager.cpp
    Model/Workflow/CWorkflowRunManager.h
    Model/Workflow/CWorkflowScaleManager.h
    Model/Workflow/CWorkflowScaleManager.cpp
    Model/Workflow/CWorkflowPackage.h
    Model/Workflow/CWorkflowPackage.cpp
    View/Common/CBorderLayout.cpp
    View/Common/CBorderLayout.h
    View/Common/CBubbleTip.cpp
    View/Common/CBubbleTip.h
    View/Common/CCameraDlg.cpp
    View/Common/CCameraDlg.h
    View/Common/CContextMenu.hpp
    View/Common/CCrashReporDlg.cpp
    View/Common/CCrashReporDlg.h
    View/Common/CDialog.cpp
    View/Common/CDialog.h
    View/Common/CDockWidgetTitleBar.cpp
    View/Common/CDockWidgetTitleBar.h
    View/Common/CFramelessHelper.cpp
    View/Common/CFramelessHelper.h
    View/Common/CGLWidget.cpp
    View/Common/CGLWidget.h
    View/Common/CGraphicsItemTools.hpp
    View/Common/CHtmlDelegate.cpp
    View/Common/CHtmlDelegate.h
    View/Common/CImageListView.cpp
    View/Common/CImageListView.h
    View/Common/CInfoPane.cpp
    View/Common/CInfoPane.h
    View/Common/CListViewDelegate.cpp
    View/Common/CListViewDelegate.h
    View/Common/CPane.cpp
    View/Common/CPane.h
    View/Common/CProgressCircle.cpp
    View/Common/CProgressCircle.h
    View/Common/CProgressCircleManager.cpp
    View/Common/CProgressCircleManager.h
    View/Common/CResizeStackedWidget.cpp
    View/Common/CResizeStackedWidget.h
    View/Common/CRollupBtn.cpp
    View/Common/CRollupBtn.h
    View/Common/CRollupWidget.cpp
    View/Common/CRollupWidget.h
    View/Common/CStackedPane.cpp
    View/Common/CStackedPane.h
    View/Common/CSvgButton.cpp
    View/Common/CSvgButton.h
    View/Common/CToolbarBorderLayout.cpp
    View/Common/CToolbarBorderLayout.h
    View/Common/VTK/CVTKCustomInteractorStyle2d.cpp
    View/Common/VTK/CVTKCustomInteractorStyle2d.h
    View/Common/VTK/CVTKViewer.cpp
    View/Common/VTK/CVTKViewer.h
    View/Common/VTK/CVTKWidget.cpp
    View/Common/VTK/CVTKWidget.h
    View/DoubleView/3D/C3dAnimationDlg.cpp
    View/DoubleView/3D/C3dAnimationDlg.h
    View/DoubleView/3D/C3dDisplay.cpp
    View/DoubleView/3D/C3dDisplay.h
    View/DoubleView/3D/CPositionDisplay.cpp
    View/DoubleView/3D/CPositionDisplay.h
    View/DoubleView/3D/CScene3dDisplay.cpp
    View/DoubleView/3D/CScene3dDisplay.cpp
    View/DoubleView/3D/CVolumeDisplay.cpp
    View/DoubleView/3D/CVolumeDisplay.h
    View/DoubleView/3D/CVolumeViewSync.cpp
    View/DoubleView/3D/CVolumeViewSync.h
    View/DoubleView/CDataDisplay.cpp
    View/DoubleView/CDataDisplay.h
    View/DoubleView/CDataDisplayLayout.cpp
    View/DoubleView/CDataDisplayLayout.h
    View/DoubleView/CDataListView.cpp
    View/DoubleView/CDataListView.h
    View/DoubleView/CDataListViewDelegate.cpp
    View/DoubleView/CDataListViewDelegate.h
    View/DoubleView/CDataViewer.cpp
    View/DoubleView/CDataViewer.h
    View/DoubleView/CDoubleView.cpp
    View/DoubleView/CDoubleView.h
    View/DoubleView/CPathNavigator.cpp
    View/DoubleView/CPathNavigator.h
    View/DoubleView/CStaticDisplay.cpp
    View/DoubleView/CStaticDisplay.h
    View/DoubleView/CTextDisplay.cpp
    View/DoubleView/CTextDisplay.h
    View/DoubleView/CWidgetDataDisplay.cpp
    View/DoubleView/CWidgetDataDisplay.h
    View/DoubleView/Image/CImageDisplay.cpp
    View/DoubleView/Image/CImageDisplay.h
    View/DoubleView/Image/CImageExportDlg.cpp
    View/DoubleView/Image/CImageExportDlg.h
    View/DoubleView/Image/CImageScene.cpp
    View/DoubleView/Image/CImageScene.h
    View/DoubleView/Image/CImageView.cpp
    View/DoubleView/Image/CImageView.h
    View/DoubleView/Image/CImageViewSync.cpp
    View/DoubleView/Image/CImageViewSync.h
    View/DoubleView/Image/CMultiImageDisplay.cpp
    View/DoubleView/Image/CMultiImageDisplay.h
    View/DoubleView/Plot/CPlotDisplay.cpp
    View/DoubleView/Plot/CPlotDisplay.h
    View/DoubleView/Result/CResultTableDisplay.cpp
    View/DoubleView/Result/CResultTableDisplay.h
    View/DoubleView/Result/CResultTableView.cpp
    View/DoubleView/Result/CResultTableView.h
    View/DoubleView/Result/CResultsViewer.cpp
    View/DoubleView/Result/CResultsViewer.h
    View/DoubleView/Video/CVideoDisplay.cpp
    View/DoubleView/Video/CVideoDisplay.h
    View/DoubleView/Video/CVideoViewSync.cpp
    View/DoubleView/Video/CVideoViewSync.h
    View/Graphics/CGraphicsEllipsePropWidget.cpp
    View/Graphics/CGraphicsEllipsePropWidget.h
    View/Graphics/CGraphicsLayerChoiceDlg.cpp
    View/Graphics/CGraphicsLayerChoiceDlg.h
    View/Graphics/CGraphicsPointPropWidget.cpp
    View/Graphics/CGraphicsPointPropWidget.h
    View/Graphics/CGraphicsPolygonPropWidget.cpp
    View/Graphics/CGraphicsPolygonPropWidget.h
    View/Graphics/CGraphicsPolylinePropWidget.cpp
    View/Graphics/CGraphicsPolylinePropWidget.h
    View/Graphics/CGraphicsPropertiesWidget.cpp
    View/Graphics/CGraphicsPropertiesWidget.h
    View/Graphics/CGraphicsRectPropWidget.cpp
    View/Graphics/CGraphicsRectPropWidget.h
    View/Graphics/CGraphicsTextPropWidget.cpp
    View/Graphics/CGraphicsTextPropWidget.h
    View/Graphics/CGraphicsToolbar.cpp
    View/Graphics/CGraphicsToolbar.h
    View/Main/CCentralViewLayout.cpp
    View/Main/CCentralViewLayout.h
    View/Main/CMainTitleBar.cpp
    View/Main/CMainTitleBar.h
    View/Main/CMainView.ui
    View/Main/CMainView.cpp
    View/Main/CMainView.h
    View/Main/CResponsiveUI.cpp
    View/Main/CResponsiveUI.h
    View/Main/CUserLoginDlg.cpp
    View/Main/CUserLoginDlg.h
    View/Modules/CModuleDockWidget.cpp
    View/Modules/CModuleDockWidget.h
    View/Modules/PluginManager/CCppNewPluginWidget.cpp
    View/Modules/PluginManager/CCppNewPluginWidget.h
    View/Modules/PluginManager/CCppPluginMaker.cpp
    View/Modules/PluginManager/CCppPluginMaker.h
    View/Modules/PluginManager/CPluginManagerWidget.cpp
    View/Modules/PluginManager/CPluginManagerWidget.h
    View/Modules/PluginManager/CPythonNewPluginDlg.cpp
    View/Modules/PluginManager/CPythonNewPluginDlg.h
    View/Modules/PluginManager/CPythonPluginMaker.cpp
    View/Modules/PluginManager/CPythonPluginMaker.h
    View/Modules/PluginManager/CPythonPluginManagerWidget.cpp
    View/Modules/PluginManager/CPythonPluginManagerWidget.h
    View/Modules/PluginManager/PluginManagerDefine.hpp
    View/Modules/Workflow/CGraphicsDeletableButton.cpp
    View/Modules/Workflow/CGraphicsDeletableButton.h
    View/Modules/Workflow/CWorkflowConnection.cpp
    View/Modules/Workflow/CWorkflowConnection.h
    View/Modules/Workflow/CWorkflowDummyItem.cpp
    View/Modules/Workflow/CWorkflowDummyItem.h
    View/Modules/Workflow/CWorkflowIOArea.cpp
    View/Modules/Workflow/CWorkflowIOArea.h
    View/Modules/Workflow/CWorkflowInputTypeDlg.cpp
    View/Modules/Workflow/CWorkflowInputTypeDlg.h
    View/Modules/Workflow/CWorkflowItem.cpp
    View/Modules/Workflow/CWorkflowItem.h
    View/Modules/Workflow/CWorkflowLabelArea.cpp
    View/Modules/Workflow/CWorkflowLabelArea.h
    View/Modules/Workflow/CWorkflowModuleWidget.cpp
    View/Modules/Workflow/CWorkflowModuleWidget.h
    View/Modules/Workflow/CWorkflowNewDlg.cpp
    View/Modules/Workflow/CWorkflowNewDlg.h
    View/Modules/Workflow/CWorkflowPortItem.cpp
    View/Modules/Workflow/CWorkflowPortItem.h
    View/Modules/Workflow/CWorkflowScene.cpp
    View/Modules/Workflow/CWorkflowScene.h
    View/Modules/Workflow/CWorkflowView.cpp
    View/Modules/Workflow/CWorkflowView.h
    View/Modules/Workflow/CWorkflowPublishDlg.h
    View/Modules/Workflow/CWorkflowPublishDlg.cpp
    View/Notifications/CNotificationPane.cpp
    View/Notifications/CNotificationPane.h
    View/Notifications/CNotificationPopup.cpp
    View/Notifications/CNotificationPopup.h
    View/Preferences/CGeneralSettingsWidget.cpp
    View/Preferences/CGeneralSettingsWidget.h
    View/Preferences/CNewUserDlg.cpp
    View/Preferences/CNewUserDlg.h
    View/Preferences/CPreferencesDlg.cpp
    View/Preferences/CPreferencesDlg.h
    View/Preferences/CUserManagementWidget.cpp
    View/Preferences/CUserManagementWidget.h
    View/Preferences/CWorkflowSettingsWidget.cpp
    View/Preferences/CWorkflowSettingsWidget.h
    View/Process/CProcessDocContent.cpp
    View/Process/CProcessDocContent.h
    View/Process/CProcessDocDlg.cpp
    View/Process/CProcessDocDlg.h
    View/Process/CProcessDocFrame.cpp
    View/Process/CProcessDocFrame.h
    View/Process/CProcessDocWidget.cpp
    View/Process/CProcessDocWidget.h
    View/Process/CProcessListPopup.cpp
    View/Process/CProcessListPopup.h
    View/Process/CProcessListView.cpp
    View/Process/CProcessListView.h
    View/Process/CProcessListViewDelegate.cpp
    View/Process/CProcessListViewDelegate.h
    View/Process/CProcessPane.cpp
    View/Process/CProcessPane.h
    View/Process/CProcessParameters.cpp
    View/Process/CProcessParameters.h
    View/Process/CProcessPopupDlg.cpp
    View/Process/CProcessPopupDlg.h
    View/Project/CDatasetLoadPolicyDlg.cpp
    View/Project/CDatasetLoadPolicyDlg.h
    View/Project/CProjectPane.cpp
    View/Project/CProjectPane.h
    View/Project/CProjectTreeView.cpp
    View/Project/CProjectTreeView.h
    View/Hub/CHubDlg.cpp
    View/Hub/CHubDlg.h
    View/Hub/CHubPluginListView.cpp
    View/Hub/CHubPluginListView.h
    View/Hub/CHubPluginListViewDelegate.cpp
    View/Hub/CHubPluginListViewDelegate.h
    View/Hub/CWorkspaceChoiceDlg.h
    View/Hub/CWorkspaceChoiceDlg.cpp
    View/Hub/CPublicationFormDlg.h
    View/Hub/CPublicationFormDlg.cpp
    View/Wizard/CWizardDocFrame.cpp
    View/Wizard/CWizardDocFrame.h
    View/Wizard/CWizardDocPage.cpp
    View/Wizard/CWizardDocPage.h
    View/Wizard/CWizardPane.cpp
    View/Wizard/CWizardPane.h
    View/Wizard/CWizardStepListView.cpp
    View/Wizard/CWizardStepListView.h
    View/Wizard/CWizardStepListViewDelegate.cpp
    View/Wizard/CWizardStepListViewDelegate.h
    View/Wizard/CWizardStepPage.cpp
    View/Wizard/CWizardStepPage.h
    View/Wizard/CWizardTutoListView.cpp
    View/Wizard/CWizardTutoListView.h
    View/Wizard/CWizardTutoListViewDelegate.cpp
    View/Wizard/CWizardTutoListViewDelegate.h
    View/Workflow/CWorkflowInfoDlg.cpp
    View/Workflow/CWorkflowInfoDlg.h
    View/Workflow/CWorkflowPane.cpp
    View/Workflow/CWorkflowPane.h
    app.rc
)

# Resources
set(RESOURCES "")
qt5_add_resources(RESOURCES res.qrc)
target_sources(IkomiaStudio PRIVATE ${RESOURCES})

target_precompile_headers(IkomiaStudio PRIVATE
    Main/pch.hpp
)

if(MSVC)
    target_compile_options(IkomiaStudio PRIVATE
        /arch:AVX2
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

target_compile_features(IkomiaStudio PRIVATE cxx_std_14)

target_compile_definitions(IkomiaStudio PRIVATE
    BOOST_ALL_NO_LIB
    QT_DEPRECATED_WARNINGS
    QT_QTPROPERTYBROWSER_IMPORT
    SOCORRO
    __STDC_FORMAT_MACROS
)

target_include_directories(IkomiaStudio PRIVATE
    # Ikomia libs
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/Core
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/DataIO
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/DataManagement
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/DataPlot
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/DataProcess
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/Utils
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/VolumeRender
    # gmic
    ${CMAKE_CURRENT_LIST_DIR}/../gmic/src
    # Python
    ${Python3_INCLUDE_DIRS}
    # Numpy
    ${Python3_NumPy_INCLUDE_DIRS}
    # OpenCV
    ${OpenCV_INCLUDE_DIRS}
    # Breakpad
    ${CMAKE_CURRENT_LIST_DIR}/../breakpad/src
    # Quazip
    ${CMAKE_CURRENT_LIST_DIR}/../quazip/quazip
    # Qwt
    ${CMAKE_CURRENT_LIST_DIR}/../qwt/src
)

target_link_directories(IkomiaStudio PRIVATE
    # Ikomia libs
    ${CMAKE_CURRENT_LIST_DIR}/../IkomiaCore/Build/lib
    # GMIC
    ${CMAKE_CURRENT_LIST_DIR}/../gmic/build
    # Qwt
    ${CMAKE_CURRENT_LIST_DIR}/../qwt/build/lib
    # Quazip
    ${CMAKE_CURRENT_LIST_DIR}/../quazip/build/lib
)

target_link_libraries(IkomiaStudio PRIVATE
    # Qt
    Qt::Concurrent
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::OpenGL
    Qt::Sql
    Qt::Svg
    # Python
    Python3::Python
    # Boost
    Boost::system
    Boost::filesystem
    ${BOOST_PYTHON_TARGET}
    # OpenMP
    OpenMP::OpenMP_CXX
    # OpenGL
    OpenGL::GL
    # VTK
    ${VTK_LIBRARIES}
    # OpenCV
    opencv_core
    opencv_imgcodecs
    opencv_imgproc
    opencv_alphamat
    opencv_videoio
    opencv_highgui
    opencv_video
    opencv_calib3d
    opencv_features2d
    opencv_objdetect
    opencv_dnn
    opencv_ml
    opencv_flann
    opencv_photo
    opencv_stitching
    opencv_gapi
    opencv_wechat_qrcode
    opencv_aruco
    opencv_bgsegm
    opencv_bioinspired
    opencv_ccalib
    opencv_cudaarithm
    opencv_cudabgsegm
    opencv_cudafeatures2d
    opencv_cudafilters
    opencv_cudaimgproc
    opencv_cudalegacy
    opencv_cudaobjdetect
    opencv_cudaoptflow
    opencv_cudastereo
    opencv_cudawarping
    opencv_cudev
    opencv_datasets
    opencv_dnn_objdetect
    opencv_dnn_superres
    opencv_dpm
    opencv_face
    opencv_fuzzy
    opencv_hfs
    opencv_img_hash
    opencv_intensity_transform
    opencv_line_descriptor
    opencv_mcc
    opencv_optflow
    opencv_phase_unwrapping
    opencv_plot
    opencv_quality
    opencv_rapid
    opencv_reg
    opencv_rgbd
    opencv_saliency
    opencv_shape
    opencv_stereo
    opencv_structured_light
    opencv_superres
    opencv_surface_matching
    opencv_text
    opencv_tracking
    opencv_videostab
    opencv_xfeatures2d
    opencv_ximgproc
    opencv_xobjdetect
    opencv_xphoto
    # gmic
    gmic
    # Qwt
    qwt
    # Quazip
    quazip1-qt5
    # OpenCL
    OpenCL
    # Ikomia
    ikUtils
    ikCore
    ikDataIO
    ikDataManagement
    ikDataPlot
    ikDataProcess
    ikVolumeRender
)

if(UNIX)
    target_link_directories(IkomiaStudio PRIVATE
        # Breakpad
        ${CMAKE_CURRENT_LIST_DIR}/../breakpad/src/client/linux
    )

    target_link_libraries(IkomiaStudio PRIVATE
        curl
        # Linux only OpenCV libs
        opencv_freetype

        # GDCM
        gdcmMSFF
        gdcmDICT
        gdcmIOD
        gdcmDSED
        gdcmCommon
        gdcmjpeg8
        gdcmjpeg12
        gdcmjpeg16
        gdcmopenjp2
        gdcmcharls
        gdcmuuid
        gdcmexpat

        # Breakpad
        breakpad_client
    )
endif()

if(UNIX AND CENTOS7)
    target_include_directories(IkomiaStudio PRIVATE
        ../build/breakpad/src/breakpad/src
        /work/shared/local/include/QuaZip-Qt5-1.3/quazip
        # OpenCV
        /work/shared/local/include/opencv4
    )

    target_link_directories(IkomiaStudio PRIVATE
        /work/shared/local/lib
        /work/shared/local/lib64
    )
endif()

if(UNIX AND NOT CENTOS7)
    target_include_directories(IkomiaStudio PRIVATE
        # OpenCV
        /usr/local/include/opencv4
    )

    target_link_libraries(IkomiaStudio PRIVATE
        opencv_hdf
    )
endif()

if(WIN32)
    target_include_directories(IkomiaStudio PRIVATE
        # Boost
        ${Boost_INCLUDE_DIRS}/Boost/include/boost-${BOOST_VERSION}
        # OpenCL
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v${CUDA_VERSION}/include"
        # Zlib
        "C:/Program Files/zlib/include"
    )

    target_link_directories(IkomiaStudio PRIVATE
        # GMIC
        $ENV{ProgramW6432}/gmic/bin
        $ENV{ProgramW6432}/gmic/lib
        # Breakpad
        ${CMAKE_CURRENT_LIST_DIR}/../breakpad/src/client/windows/Release/lib
    )

    target_link_libraries(IkomiaStudio PRIVATE
        common
        crash_generation_client
        exception_handler
        OpenGL32
    )
endif()

install(TARGETS IkomiaStudio
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)



###############################################
##                                           ##
##     Generation of the package's files     ##
##                                           ##
###############################################

include(CMakePackageConfigHelpers)

set(IKOMIA_STUDIO_DIR
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE PATH "Location of root directory"
)

set(IkomiaStudio_UNDERSCORE_VERSION
    ${IkomiaStudio_VERSION_MAJOR}_${IkomiaStudio_VERSION_MINOR}_${IkomiaStudio_VERSION_PATCH}
    CACHE STRING "Version of the project where dots are replaced by underscore (X.X.X -> 'X_X_X')"
)

configure_package_config_file(
    IkomiaStudioConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/IkomiaStudioConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/IkomiaStudio-${IKOMIA_VERSION}
    PATH_VARS IKOMIA_STUDIO_DIR CMAKE_INSTALL_PREFIX CMAKE_INSTALL_BINDIR
)

write_basic_package_version_file(
    IkomiaStudioConfigVersion.cmake
    VERSION ${IKOMIA_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/IkomiaStudioConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/IkomiaStudioConfigVersion.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/IkomiaStudio-${IKOMIA_VERSION}
)


#------------------------------------------------------------------------
#- Run on WINDOWS
#- To run with QtCreator directly, following paths must be added to PATH:
# OpenGL: C:/Program Files(x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64"
# GDCM: C:/Program Files/GDCM/bin
# Tesseract: C:/Program Files/tesseract/bin
# ZLib: C:/Program Files/zlib/bin
# Quazip: C:/Developpement/quazip/build/lib
# Qwt: C:/Developpement/qwt/build/lib
# gmic: C:/Program Files/gmic/bin/
# OpenSSL: C:/Program Files/OpenSSL-Win64/bin
# IkomiaCore: C:/Developpement/IkomiaCore/Build/lib
#------------------------------------------------------------------------
