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

#include "CMainCtrl.h"
#include "View/Common/CInfoPane.h"
#include "View/Project/CProjectPane.h"
#include "View/DoubleView/CDoubleView.h"
#include "View/DoubleView/CDataListView.h"
#include "View/DoubleView/3D/C3dDisplay.h"
#include "View/Process/CProcessPane.h"
#include "View/Process/CProcessPopupDlg.h"
#include "View/Modules/Workflow/CWorkflowModuleWidget.h"
#include "View/Modules/Workflow/CWorkflowScene.h"
#include "View/Workflow/CWorkflowPane.h"
#include "View/Graphics/CGraphicsToolbar.h"
#include "View/Hub/CHubDlg.h"
#include "View/Wizard/CWizardPane.h"
#include "View/Preferences/CWorkflowSettingsWidget.h"
#include "Model/Data/CFeaturesTableModel.h"
#include "Model/Data/CMultiImageModel.h"
#include <QApplication>

CMainCtrl::CMainCtrl(CMainModel* pModel, CMainView* pView, QSplashScreen *pSplash)
{
    assert(pModel);
    assert(pView);

    m_pModel = pModel;
    m_pView = pView;
    m_pSplash = pSplash;

    initSplash();
    m_pModel->init();

    // Tutorials
    m_tutorialMgr.setModelView(pModel, pView);
}

void CMainCtrl::initSplash()
{
    connect(m_pModel, &CMainModel::doSetSplashMessage, m_pSplash, &QSplashScreen::showMessage);
}

void CMainCtrl::initConnections()
{
    assert(m_pView != nullptr && m_pModel != nullptr);

    // Project connections
    initProjectConnections();

    // Data connections
    initDataConnections();

    // Process connections
    initProcessConnections();

    // ApplyProcess connections
    initWorkflowConnections();

    // Image information connections
    initInfoConnections();

    // Render connections
    initRenderConnections();

    // Progress bar connections
    initProgressConnections();

    // Graphics connections
    initGraphicsConnections();

    // Results connections
    initResultsConnections();

    // Video Connections
    initVideoConnections();

    // User connections
    initUserConnections();

    // Plugins hub connections
    initHubConnections();

    // Wizard connections
    initWizardConnections();

    // Settings connections
    initSettingsConnections();

    // Plugin connections
    initPluginConnections();

    initActionConnections();
}

void CMainCtrl::initProjectConnections()
{
    //Main view -> project manager
    connect(m_pView, &CMainView::doNewProject, m_pModel->getProjectManager(), &CProjectManager::onNewProject);
    connect(m_pView, &CMainView::doLoadProject, m_pModel->getProjectManager(), &CProjectManager::onLoadProject);
    connect(m_pView, &CMainView::doLoadImages, m_pModel->getProjectManager(), &CProjectManager::onLoadImages);
    connect(m_pView, &CMainView::doLoadVideos, m_pModel->getProjectManager(), &CProjectManager::onLoadVideos);
    connect(m_pView, &CMainView::doLoadStream, m_pModel->getProjectManager(), &CProjectManager::onLoadStream);
    connect(m_pView, &CMainView::doLoadFolder, m_pModel->getProjectManager(), &CProjectManager::onLoadFolder);

    //Project pane -> project manager
    connect(m_pView->getProjectPane(), &CProjectPane::doAddEmptyFolder, m_pModel->getProjectManager(), &CProjectManager::onAddFolder);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddFolder, m_pModel->getProjectManager(), &CProjectManager::onLoadFolder);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddDataset, m_pModel->getProjectManager(), &CProjectManager::onAddDataset);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddDimension, m_pModel->getProjectManager(), &CProjectManager::onAddDimension);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddImages, m_pModel->getProjectManager(), &CProjectManager::onAddImage);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddImagesToDimension, m_pModel->getProjectManager(), &CProjectManager::onAddImageToDimension);
    connect(m_pView->getProjectPane(), &CProjectPane::doAddDicomFolder, m_pModel->getProjectManager(), &CProjectManager::onAddDicomFolder);
    connect(m_pView->getProjectPane(), &CProjectPane::doDisplayData, m_pModel->getProjectManager(), &CProjectManager::onDisplayData);
    connect(m_pView->getProjectPane(), &CProjectPane::doSaveProjectAs, m_pModel->getProjectManager(), &CProjectManager::onSaveProjectAs);
    connect(m_pView->getProjectPane(), &CProjectPane::doSaveProject, m_pModel->getProjectManager(), &CProjectManager::onSaveProject);
    connect(m_pView->getProjectPane(), &CProjectPane::doCloseProject, m_pModel->getProjectManager(), &CProjectManager::onCloseProject);
    connect(m_pView->getProjectPane(), &CProjectPane::doInsertProjectDimension, m_pModel->getProjectManager(), &CProjectManager::onInsertDimension);
    connect(m_pView->getProjectPane()->getTreeView(), &CProjectTreeView::doDeleteItem, m_pModel->getProjectManager(), &CProjectManager::onDeleteItem);
    connect(m_pView->getProjectPane(), &CProjectPane::doRemoveItem, m_pModel->getProjectManager(), &CProjectManager::onDeleteItem);
    connect(m_pView->getProjectPane(), &CProjectPane::doUpdateProjectModelIndex, m_pModel->getProjectManager(), &CProjectManager::onUpdateIndex);
    connect(m_pView->getProjectPane(), &CProjectPane::doExportProject, m_pModel->getProjectManager(), &CProjectManager::onExportProject);
    connect(m_pView->getProjectPane(), &CProjectPane::doShowLocation, m_pModel->getProjectManager(), &CProjectManager::onShowLocation);

    //Double view -> project manager
    connect(m_pView->getDoubleView()->getDataViewer()->getDataListView(), &CDataListView::doDisplayData, m_pModel->getProjectManager(), &CProjectManager::onDisplayData);

    //Image display -> project manager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateImageIndex, m_pModel->getProjectManager(), &CProjectManager::onCurrentImgChangedInZ);

    //Project manager -> project view
    connect(m_pModel->getProjectManager(), &CProjectManager::doSetModel, m_pView->getProjectPane(), &CProjectPane::onUpdateProject);
    connect(m_pModel->getProjectManager(), &CProjectManager::doUpdateImgZ, m_pView->getProjectPane(), &CProjectPane::onTreeViewClicked);
    connect(m_pModel->getProjectManager(), &CProjectManager::doUpdateIndex, m_pView->getProjectPane(), &CProjectPane::onUpdateIndex);
    connect(m_pModel->getProjectManager(), &CProjectManager::doUpdateSelectedIndex, m_pView->getProjectPane(), &CProjectPane::onUpdateSelectedIndex);

    //Project manager -> main view
    connect(m_pModel->getProjectManager(), &CProjectManager::doCloseProject, m_pView, &CMainView::onCloseProject);

    //Project manager -> double view
    connect(m_pModel->getProjectManager(), &CProjectManager::doSetModel, m_pView->getDoubleView(), &CDoubleView::onUpdateDataListviewModel);   
    connect(m_pModel->getProjectManager(), &CProjectManager::doSwitchView, m_pView->getDoubleView(), &CDoubleView::onSwitchView);
    connect(m_pModel->getProjectManager(), &CProjectManager::doAfterCurrentImageDeleted, m_pView->getDoubleView(), &CDoubleView::onImageDeleted);
    connect(m_pModel->getProjectManager(), &CProjectManager::doDisplayVideo, m_pView->getDoubleView(), &CDoubleView::onDisplayVideo);
    connect(m_pModel->getProjectManager(), &CProjectManager::doUpdateIndex, m_pView->getDoubleView(), &CDoubleView::onUpdateDataViewerIndex);
}

void CMainCtrl::initProcessConnections()
{
    //Main view -> main model
    connect(m_pView, &CMainView::doLoadPlugin, m_pModel->getProcessManager(), &CProcessManager::onReloadAllPlugins);

    //Process manager -> process pane
    connect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessModel, m_pView->getProcessPane(), &CProcessPane::onSetModel);
    connect(m_pModel->getProcessManager(), &CProcessManager::doSetWidgetInstance, m_pView->getProcessPane(), &CProcessPane::onSetWidgetInstance);
    connect(m_pModel->getProcessManager(), &CProcessManager::doSetTableModel, m_pView->getProcessPane(), &CProcessPane::onSetTableModel);
    connect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessInfo, m_pView->getProcessPane(), &CProcessPane::onSetProcessInfo);
    connect(m_pModel->getProcessManager(), &CProcessManager::doOnAllProcessReloaded, m_pView->getProcessPane(), &CProcessPane::onAllProcessReloaded);
    connect(m_pModel->getProcessManager(), &CProcessManager::doOnProcessReloaded, m_pView->getProcessPane(), &CProcessPane::onProcessReloaded);

    //Process pane -> process manager
    connect(m_pView->getProcessPane(), &CProcessPane::doSearchProcess, m_pModel->getProcessManager(), &CProcessManager::onSearchProcess);
    connect(m_pView->getProcessPane(), &CProcessPane::doSearchProcess, m_pModel->getProcessManager(), &CProcessManager::onSearchTableProcess);
    connect(m_pView->getProcessPane(), &CProcessPane::doQueryWidgetInstance, m_pModel->getProcessManager(), &CProcessManager::onQueryWidgetInstance);
    connect(m_pView->getProcessPane(), &CProcessPane::doQueryProcessInfo, m_pModel->getProcessManager(), &CProcessManager::onQueryProcessInfo);
    connect(m_pView->getProcessPane(), &CProcessPane::doTreeViewClicked, m_pModel->getProcessManager(), &CProcessManager::onUpdateTableModel);
    connect(m_pView->getProcessPane()->getProcessParameters(), &CProcessParameters::doUpdateProcessInfo, m_pModel->getProcessManager(), &CProcessManager::onUpdateProcessInfo);

    //Process manager -> process popup dialog
    connect(m_pModel->getProcessManager(), &CProcessManager::doSetWidgetInstance, m_pView->getProcessPopup(), &CProcessPopupDlg::onSetWidgetInstance);

    //Process popup dialog -> process manager
    connect(m_pView->getProcessPopup(), &CProcessPopupDlg::doTreeViewClicked, m_pModel->getProcessManager(), &CProcessManager::onUpdateTableModel);
    connect(m_pView->getProcessPopup(), &CProcessPopupDlg::doTextChanged, m_pModel->getProcessManager(), &CProcessManager::onSearchTableProcess);
    connect(m_pView->getProcessPopup(), &CProcessPopupDlg::doQueryWidgetInstance, m_pModel->getProcessManager(), &CProcessManager::onQueryWidgetInstance);
    connect(m_pView->getProcessPopup(), &CProcessPopupDlg::doUpdateProcessInfo, m_pModel->getWorkflowManager(), &CWorkflowManager::onUpdateProcessInfo);

    //Process popup dialog -> protocol manager
    connect(m_pView->getProcessPopup(), &CProcessPopupDlg::doAddProcess, m_pModel->getWorkflowManager(), &CWorkflowManager::onAddProcess);

    // When we open/close the process popup dialog, we must switch process model AND table process model in process manager in order to get the right behaviour
    // To achieve this, we connect/disconnect the model update mechanism
    // (ProcessPaneTreeView/ProcessPaneListView vs ProcessPopupDlgTreeView/ProcessPopupDlgListView)
    connect(m_pView, &CMainView::doBeforeProcessPopupOpen, [&]{
        disconnect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessModel, m_pView->getProcessPane(), &CProcessPane::onSetModel);
        disconnect(m_pModel->getProcessManager(), &CProcessManager::doSetTableModel, m_pView->getProcessPane(), &CProcessPane::onSetTableModel);
        connect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessModel, m_pView->getProcessPopup(), &CProcessPopupDlg::onSetProcessTreeModel);
        connect(m_pModel->getProcessManager(), &CProcessManager::doSetTableModel, m_pView->getProcessPopup(), &CProcessPopupDlg::onSetProcessListModel);
        m_pModel->getProcessManager()->onSetCurrentProcessModel(CProcessManager::ID::PROCESS_POPUP);
    });
    connect(m_pView, &CMainView::doAfterProcessPopupClose, [&]{
        disconnect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessModel, m_pView->getProcessPopup(), &CProcessPopupDlg::onSetProcessTreeModel);
        disconnect(m_pModel->getProcessManager(), &CProcessManager::doSetTableModel, m_pView->getProcessPopup(), &CProcessPopupDlg::onSetProcessListModel);
        connect(m_pModel->getProcessManager(), &CProcessManager::doSetProcessModel, m_pView->getProcessPane(), &CProcessPane::onSetModel);
        connect(m_pModel->getProcessManager(), &CProcessManager::doSetTableModel, m_pView->getProcessPane(), &CProcessPane::onSetTableModel);
        m_pModel->getProcessManager()->onSetCurrentProcessModel(CProcessManager::ID::PROCESS_TREE);
    });
}

void CMainCtrl::initWorkflowConnections()
{
    //Process view -> project view
    connect(m_pView->getProcessPane(), &CProcessPane::doApplyProcess, m_pView->getProjectPane(), &CProjectPane::onApplyProcess);

    //Project view -> workflow manager
    connect(m_pView->getProjectPane(), &CProjectPane::doApplyProcess, m_pModel->getWorkflowManager(), &CWorkflowManager::onApplyProcess);

    //Workflow pane -> workflow manager
    connect(m_pView->getWorkflowPane(), &CWorkflowPane::doLoadWorkflow, m_pModel->getWorkflowManager(), &CWorkflowManager::onLoadWorkflow);
    connect(m_pView->getWorkflowPane(), &CWorkflowPane::doDeleteWorkflow, m_pModel->getWorkflowManager(), &CWorkflowManager::onDeleteWorkflow);
    connect(m_pView->getWorkflowPane(), &CWorkflowPane::doSearchWorkflow, m_pModel->getWorkflowManager(), &CWorkflowManager::onSearchWorkflow);
    connect(m_pView->getWorkflowPane(), &CWorkflowPane::doGetWorkflowInfo, m_pModel->getWorkflowManager(), &CWorkflowManager::onGetWorkflowInfo);

    //Workflow module -> manager
    connect(m_pView->getWorkflowModule(), &CWorkflowModuleWidget::doNotifyWorkflowClosed, m_pModel->getWorkflowManager(), &CWorkflowManager::onWorkflowClosed);

    //Workflow manager -> workflow module
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doSetManager, m_pView->getWorkflowModule(), &CWorkflowModuleWidget::onSetModel);

    //Workflow manager -> workflow pane
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doSetNamesModel, m_pView->getWorkflowPane(), &CWorkflowPane::onSetModel);
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doSetNamesFromImageModel, m_pView->getWorkflowPane(), &CWorkflowPane::onSetFromImageModel);
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doSetDescription, m_pView->getWorkflowPane(), &CWorkflowPane::onSetDescription);

    //Workflow manager -> main view
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doNewWorkflowNotification, m_pView, &CMainView::onNewNotification);
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doSetWorkflowChangedIcon, m_pView, &CMainView::onSetWorkflowChangedIcon);
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doStopVideo, m_pView->getDoubleView(), &CDoubleView::onStopVideo);

    //Workflow input view manager -> main view
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doInitDisplay, m_pView->getDoubleView(), &CDoubleView::onInitDisplay);
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doDisplayImage, m_pView->getDoubleView(), &CDoubleView::onDisplayImage);
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doDisplayVideo, m_pView->getDoubleView(), &CDoubleView::onDisplayVideo);
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doUpdateVolumeImage, m_pView->getDoubleView(), &CDoubleView::onUpdateVolumeImage);
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doApplyViewProperty, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onApplyViewProperty);

    //Workflow input view manager -> video manager
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doInitVideoInfo, m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::onInitInfo);
    connect(m_pModel->getWorkflowManager()->getInputViewManager(), &CWorkflowInputViewManager::doDisplayVideoImage, m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::onDisplayCurrentVideoImage);

    //Double view -> workflow manager
    connect(m_pView->getDoubleView(), &CDoubleView::doSetSelectedDisplay, m_pModel->getWorkflowManager(), &CWorkflowManager::onIODisplaysSelected);

    //Main view -> workflow manager
    connect(m_pView, &CMainView::doCloseApp, m_pModel->getWorkflowManager(), &CWorkflowManager::onCloseApp);
}

void CMainCtrl::initInfoConnections()
{
    //Image manager -> Info pane
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doDisplayImageInfo, m_pView->getInfoPane(), &CInfoPane::onDisplayImageInfo);

    //Video manager -> Info pane
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doDisplayVideoInfo, m_pView->getInfoPane(), &CInfoPane::onDisplayVideoInfo);

    //Workflow manager -> Info pane
    connect(m_pModel->getWorkflowManager(), &CWorkflowManager::doUpdateImageInfo, m_pView->getInfoPane(), &CInfoPane::onDisplayImageInfo);

    //Info pane -> Data manager
    connect(m_pView->getInfoPane(), &CInfoPane::doEnableInfo, m_pModel->getDataManager(), &CMainDataManager::onEnableInfo);
}

void CMainCtrl::initRenderConnections()
{
    //Data viewer -> render manager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doInitCL, m_pModel->getRenderManager(), &CRenderManager::onInitCL);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doInitPBO, m_pModel->getRenderManager(), &CRenderManager::onInitPBO);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doRenderVolume, m_pModel->getRenderManager(), &CRenderManager::onRenderVolume);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateWindowSize, m_pModel->getRenderManager(), &CRenderManager::onUpdateWindowSize);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateRenderMode, m_pModel->getRenderManager(), &CRenderManager::onUpdateRenderMode);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateColormap, m_pModel->getRenderManager(), &CRenderManager::onUpdateColormap);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateParam, m_pModel->getRenderManager(), &CRenderManager::onUpdateRenderParam);

    // Data viewer -> main view
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doSendErrorMessage,m_pView, &CMainView::onShowErrorMessage);

    //Render manager -> data viewer
    connect(m_pModel->getRenderManager(), &CRenderManager::doUpdateVolumeRender, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onUpdateRenderVolume);
    connect(m_pModel->getRenderManager(), &CRenderManager::doEnableRender, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onEnableRender);
}

void CMainCtrl::initProgressConnections()
{
    connect(m_pModel->getProgressManager(), &CProgressBarManager::doShowProgressNotification, m_pView, &CMainView::onNewNotification);
    connect(m_pModel->getProgressManager(), &CProgressBarManager::doShowInfiniteProgressNotification, m_pView, &CMainView::onNewInfiniteProgress);
}

void CMainCtrl::initGraphicsConnections()
{
    //Toolbar -> manager
    connect(m_pView->getGraphicsToolbar(), &CGraphicsToolbar::doSetGraphicsTool, m_pModel->getGraphicsManager(), &CGraphicsManager::onSetGraphicsTool);

    //Toolbar -> data viewer
    connect(m_pView->getGraphicsToolbar(), &CGraphicsToolbar::doActivateGraphics, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onChangeGraphicsActivationState);

    //Project pane -> manager
    connect(m_pView->getProjectPane(), &CProjectPane::doAddGraphicsLayer, m_pModel->getGraphicsManager(), &CGraphicsManager::onAddLayer);
    connect(m_pView->getProjectPane(), &CProjectPane::doSetCurrentGraphicsLayer, m_pModel->getGraphicsManager(), &CGraphicsManager::onSetCurrentLayer);

    //Manager -> data viewer
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doSetCurrentLayer, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetCurrentGraphicsLayer);
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doAddGraphicsItem, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onAddGraphicsItem);
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doRemoveGraphicsLayerToSource, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onRemoveGraphicsLayer);
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doAddTemporaryLayerToSource, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onAddGraphicsLayer);

    //Manager -> result viewer
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doAddTemporaryLayerToResult, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onAddGraphicsLayer);
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doRemoveGraphicsLayerToResult, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onRemoveGraphicsLayer);

    //Manager -> toolbar
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doSetGraphicsContext, m_pView, &CMainView::onSetGraphicsContext);
    connect(m_pModel->getGraphicsManager(), &CGraphicsManager::doGraphicsContextChanged, m_pView, &CMainView::onGraphicsContextChanged);

    //Views -> toolbar
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doChangeGraphicsActivationState, m_pView->getGraphicsToolbar(), &CGraphicsToolbar::onChangeActivationState);

    //Views -> manager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doAddGraphicsLayer, m_pModel->getGraphicsManager(), &CGraphicsManager::onAddLayerFromView);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doGraphicsChanged, m_pModel->getGraphicsManager(), &CGraphicsManager::onGraphicsChanged);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doGraphicsRemoved, m_pModel->getGraphicsManager(), &CGraphicsManager::onGraphicsRemoved);
}

void CMainCtrl::initResultsConnections()
{
    //Results manager -> double view
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayImage, m_pView->getDoubleView(), &CDoubleView::onDisplayResultImage);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayMeasuresTable, m_pView->getDoubleView(), &CDoubleView::onDisplayResultMeasuresTable);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayFeaturesTable, m_pView->getDoubleView(), &CDoubleView::onDisplayResultFeaturesTable);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayPlot, m_pView->getDoubleView(), &CDoubleView::onDisplayResultPlot);
    connect(m_pModel->getResultManager(), &CResultManager::doClearResultsView, m_pView->getDoubleView(), &CDoubleView::onClearResults);
    connect(m_pModel->getResultManager(), &CResultManager::doInitDisplay, m_pView->getDoubleView(), &CDoubleView::onInitResultDisplay);
    connect(m_pModel->getResultManager(), &CResultManager::doHideResultsView, m_pView->getDoubleView(), &CDoubleView::onHideResultsView);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayVideo, m_pView->getDoubleView(), &CDoubleView::onDisplayResultVideo);
    connect(m_pModel->getResultManager(), &CResultManager::doAddResultWidget, m_pView->getDoubleView(), &CDoubleView::onAddResultWidget);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayDnnDataset, m_pView->getDoubleView(), &CDoubleView::onDisplayMultiImage);
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayText, m_pView->getDoubleView(), &CDoubleView::onDisplayResultText);

    //Result manager -> result viewer
    connect(m_pModel->getResultManager(), &CResultManager::doApplyViewProperty, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onApplyViewProperty); 
    connect(m_pModel->getResultManager(), &CResultManager::doDisplayOverlay, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onDisplayOverlay);
    connect(m_pModel->getResultManager(), &CResultManager::doClearOverlay, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onClearOverlay);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoSourceType, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoSourceType);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoFPS, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoFPS);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoLength, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoLength);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoPos, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoPos);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoTotalTime, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoTotalTime);
    connect(m_pModel->getResultManager(), &CResultManager::doSetVideoCurrentTime, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoCurrentTime);

    //Result viewer -> manager
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doSaveCurrentResultImage, m_pModel->getResultManager(), &CResultManager::onSaveCurrentImage);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doExportCurrentResultImage, m_pModel->getResultManager(), &CResultManager::onExportCurrentImage);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doSaveTableData, m_pModel->getResultManager(), &CResultManager::onSaveTableData);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doExportTableData, m_pModel->getResultManager(), &CResultManager::onExportTableData);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doSaveCurrentResultVideo, m_pModel->getResultManager(), &CResultManager::onSaveCurrentVideo);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doExportCurrentResultVideo, m_pModel->getResultManager(), &CResultManager::onExportCurrentVideo);
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doExportDatasetImage, m_pModel->getResultManager(), &CResultManager::onExportDatasetImage);

    //Workflow module -> manager
    connect(m_pView->getWorkflowModule(), &CWorkflowModuleWidget::doNotifyWorkflowClosed, m_pModel->getResultManager(), &CResultManager::onWorkflowClosed);

    //Results manager -> main view
    connect(m_pModel->getResultManager(), &CResultManager::doNewResultNotification, m_pView, &CMainView::onNewNotification);
}

void CMainCtrl::initVideoConnections()
{
    // Video Manager -> data viewer
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetVideoLength, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoSliderLength);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doUpdateVideoPos, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoSliderPos);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetFPS, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoFPS);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetTotalTime, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoTotalTime);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetCurrentTime, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoCurrentTime);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetSourceType, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onSetVideoSourceType);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doStopRecording, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onStopRecordingVideo);

    // Video Manager -> result viewer
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetVideoLength, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoLength);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doUpdateVideoPos, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoPos);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetFPS, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoFPS);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetTotalTime, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoTotalTime);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetCurrentTime, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoCurrentTime);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doSetSourceType, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onSetVideoSourceType);

    // Video Manager -> doubleview
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doCurrentDataChanged, m_pView->getDoubleView(), &CDoubleView::onInputDataChanged);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doStopVideoPlayerView, m_pView->getDoubleView(), &CDoubleView::onStopVideoPlayer);

    // VideoManager -> ProjectPane
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doUpdateImgSequenceIndex, m_pView->getProjectPane(), &CProjectPane::onTreeViewClicked);

    // VideoManager -> ProjectManager
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doAddRecordVideo, m_pModel->getProjectManager(), &CProjectManager::onAddRecordVideo);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doDisplayVideoImage, m_pModel->getProjectManager(), &CProjectManager::onDisplayVideoImage);

    // Video manager -> protocol manager
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doVideoDataChanged, m_pModel->getWorkflowManager(), &CWorkflowManager::onInputDataChanged);
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doStopWorkflowThread, m_pModel->getWorkflowManager(), &CWorkflowManager::onStopThread);

    // ResultManager -> ProjectManager
    connect(m_pModel->getResultManager(), &CResultManager::doAddRecordVideo, m_pModel->getProjectManager(), &CProjectManager::onAddRecordVideo);

    // ResultManager -> ResultViewer
    connect(m_pModel->getResultManager(), &CResultManager::doStopRecording, m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::onStopRecordingVideo);

    //Workflow manager -> protocol manager
    connect(m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::doNotifyVideoStart, m_pModel->getWorkflowManager(), &CWorkflowManager::onNotifyVideoStart);

    // VideoDisplay -> ProjectManager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doUpdateVideoPos, m_pModel->getProjectManager(), &CProjectManager::onUpdateVideoPos);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doRecordVideo, m_pModel->getProjectManager(), &CProjectManager::onRecordVideo);

    // Data Viewer -> Data Manager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doPlayVideo, m_pModel->getDataManager(), &CMainDataManager::onPlayVideo);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doStopVideo, m_pModel->getDataManager(), &CMainDataManager::onStopVideo);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doNotifyVideoStart, m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::onNotifyVideoStart);

    // Sync result view with result manager
    connect(m_pView->getDoubleView()->getResultsViewer(), &CResultsViewer::doRecordVideo, m_pModel->getResultManager(), &CResultManager::onRecordResultVideo);

    //Workflow module -> manager
    connect(m_pView->getWorkflowModule(), &CWorkflowModuleWidget::doNotifyWorkflowClosed, m_pModel->getDataManager()->getVideoMgr(), &CVideoManager::onCloseWorkflow);
    connect(m_pView->getWorkflowModule(), &CWorkflowModuleWidget::doNotifyWorkflowClosed, m_pModel->getDataManager()->getImgMgr(), &CImgManager::onCloseWorkflow);
}

void CMainCtrl::initUserConnections()
{
    //Main view -> manager
    connect(m_pView, &CMainView::doConnectUser, m_pModel->getUserManager(), &CUserManager::onConnectUser);
    connect(m_pView, &CMainView::doDisconnectUser, m_pModel->getUserManager(), &CUserManager::onDisconnectUser);

    //Manager -> main view
    connect(m_pModel->getUserManager(), &CUserManager::doSetCurrentUser, m_pView, &CMainView::onSetCurrentUser);
    connect(m_pModel->getUserManager(), &CUserManager::doShowNotification, m_pView, &CMainView::onNewNotification);
}

void CMainCtrl::initHubConnections()
{
    //View -> model
    connect(m_pView->getHubView(), &CHubDlg::doGetHubModel, m_pModel->getHubManager(), &CHubManager::onRequestHubModel);
    connect(m_pView->getHubView(), &CHubDlg::doGetWorkspaceModel, m_pModel->getHubManager(), &CHubManager::onRequestWorkspaceModel);
    connect(m_pView->getHubView(), &CHubDlg::doGetLocalModel, m_pModel->getHubManager(), &CHubManager::onRequestLocalModel);
    connect(m_pView->getHubView(), &CHubDlg::doPublishHub, m_pModel->getHubManager(), &CHubManager::onPublishHub);
    connect(m_pView->getHubView(), &CHubDlg::doPublishWorkspace, m_pModel->getHubManager(), &CHubManager::onPublishWorkspace);
    connect(m_pView->getHubView(), &CHubDlg::doInstallPlugin, m_pModel->getHubManager(), &CHubManager::onInstallPlugin);
    connect(m_pView->getHubView(), &CHubDlg::doUpdatePluginInfo, m_pModel->getHubManager(), &CHubManager::onUpdatePluginInfo);
    connect(m_pView->getHubView(), &CHubDlg::doHubSearchChanged, m_pModel->getHubManager(), &CHubManager::onHubSearchChanged);
    connect(m_pView->getHubView(), &CHubDlg::doWorkspaceSearchChanged, m_pModel->getHubManager(), &CHubManager::onWorkspaceSearchChanged);
    connect(m_pView->getHubView(), &CHubDlg::doLocalSearchChanged, m_pModel->getHubManager(), &CHubManager::onLocalSearchChanged);
    connect(m_pView->getHubView(), &CHubDlg::doGetNextPublishInfo, m_pModel->getHubManager(), &CHubManager::onRequestNextPublishInfo);

    //Model -> view
    connect(m_pModel->getHubManager(), &CHubManager::doSetPluginModel, m_pView->getHubView(), &CHubDlg::onSetPluginModel);
    connect(m_pModel->getHubManager(), &CHubManager::doNotifyModelError, m_pView->getHubView(), &CHubDlg::onModelError);
    connect(m_pModel->getHubManager(), &CHubManager::doSetNextPublishInfo, m_pView->getHubView(), &CHubDlg::onSetNextPublishInfo);

    connect(m_pModel->getHubManager(), &CHubManager::doRestartIkomia, m_pView, &CMainView::onRestartIkomia, Qt::QueuedConnection);
}

void CMainCtrl::initWizardConnections()
{
    if(m_pView->getWizardPane())
    {
        // Tutorials standby
        connect(&m_tutorialMgr, &CWizardManager::doSetTutorialModel, m_pView->getWizardPane(), &CWizardPane::onSetTutorialModel);
        connect(&m_tutorialMgr, &CWizardManager::doSetStepModel, m_pView->getWizardPane(), &CWizardPane::onSetStepModel);
        //connect(m_pView->getWizardPane()->getTutoListView(), &CWizardTutoListView::doPlayTuto, &m_wizardMgr, &CWizardManager::onPlayTuto);
        connect(&m_tutorialMgr, &CWizardManager::doSetTotalSteps, m_pView->getWizardPane(), &CWizardPane::onSetTotalSteps);
        connect(&m_tutorialMgr, &CWizardManager::doUpdateStepPage, m_pView->getWizardPane(), &CWizardPane::onShowStepPage);
        connect(&m_tutorialMgr, &CWizardManager::doFinish, m_pView->getWizardPane(), &CWizardPane::onFinish);
        //connect(m_pView->getWizardPane()->getWizardStepPage(), &CWizardStepPage::doCancelTuto, &m_wizardMgr, &CWizardManager::onCancel);
    }
}

void CMainCtrl::initSettingsConnections()
{
    // Preferences widget -> manager
    connect(m_pView->getPreferenceDlg()->getGeneralSettings(), &CGeneralSettingsWidget::doEnableTutorialHelper, m_pModel->getSettingsManager(), &CSettingsManager::onEnableTutorialHelper);
    connect(m_pView->getPreferenceDlg()->getGeneralSettings(), &CGeneralSettingsWidget::doEnableNativeDialog, m_pModel->getSettingsManager(), &CSettingsManager::onUseNativeDlg);
    connect(m_pView->getPreferenceDlg()->getWorkflowSettings(), &CWorkflowSettingsWidget::doSetSaveFolder, m_pModel->getSettingsManager(), &CSettingsManager::onSetWorkflowSaveFolder);

    // Manager -> preferences widget
    connect(m_pModel->getSettingsManager(), &CSettingsManager::doEnableTutorialHelper, m_pView->getPreferenceDlg()->getGeneralSettings(), &CGeneralSettingsWidget::onEnableTutorialHelper);
    connect(m_pModel->getSettingsManager(), &CSettingsManager::doEnableNativeDialog, m_pView->getPreferenceDlg()->getGeneralSettings(), &CGeneralSettingsWidget::onEnableNativeDialog);
    connect(m_pModel->getSettingsManager(), &CSettingsManager::doSetWorkflowSaveFolder, m_pView->getPreferenceDlg()->getWorkflowSettings(), &CWorkflowSettingsWidget::onSetSaveFolder);
}

void CMainCtrl::initPluginConnections()
{
   //View -> process model
    connect(m_pView, &CMainView::doGetPythonQueryModel, m_pModel->getPluginManager(), &CPluginManager::onRequestPythonModel);
    connect(m_pView, &CMainView::doReloadAllPlugins, m_pModel->getProcessManager(), &CProcessManager::onReloadAllPlugins);
    connect(m_pView, &CMainView::doReloadPlugin, m_pModel->getProcessManager(), &CProcessManager::onReloadPlugin);

    //View -> plugin model
    connect(m_pView, &CMainView::doEditPythonPlugin, m_pModel->getPluginManager(), &CPluginManager::onEditPythonPlugin);
    connect(m_pView, &CMainView::doShowPluginLocation, m_pModel->getPluginManager(), &CPluginManager::onShowLocation);
    connect(m_pView, &CMainView::doGetPythonDependencyModel, m_pModel->getPluginManager(), &CPluginManager::onRequestPythonDependencyModel);
    connect(m_pView, &CMainView::doInstallPythonPkg, m_pModel->getPluginManager(), &CPluginManager::onInstallPythonPkg);
    connect(m_pView, &CMainView::doUpdatePythonPkg, m_pModel->getPluginManager(), &CPluginManager::onUpdatePythonPkg);

    //Model -> view
    connect(m_pModel->getPluginManager(), &CPluginManager::doSetPythonQueryModel, m_pView, &CMainView::onSetPythonQueryModel);
    connect(m_pModel->getPluginManager(), &CPluginManager::doSetPythonDependencyModel, m_pView, &CMainView::onSetPythonDependencyModel);

    //Model -> main view
    connect(m_pModel->getPluginManager(), &CPluginManager::doShowNotification, m_pView, &CMainView::onNewNotification);
}

void CMainCtrl::initActionConnections()
{
    connect(m_pView, &CMainView::doStartJupyterLab, m_pModel, &CMainModel::onStartJupyterLab);
}

void CMainCtrl::initDataConnections()
{
    //Image manager -> Data viewer
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doUpdateNbImg, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onUpdateNbImage);
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doUpdateCurrentImgIndex, m_pView->getDoubleView()->getDataViewer(), &CDataViewer::onUpdateCurrentImageIndex);

    //Image manager -> Double view
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doDisplayVolume, m_pView->getDoubleView(), &CDoubleView::onDisplayVolume);
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doDisplayImage, m_pView->getDoubleView(), &CDoubleView::onDisplayImage);    
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doCurrentDataChanged, m_pView->getDoubleView(), &CDoubleView::onInputDataChanged);

    //Image manager -> Workflow manager
    connect(m_pModel->getDataManager()->getImgMgr(), &CImgManager::doInputDataChanged, m_pModel->getWorkflowManager(), &CWorkflowManager::onInputDataChanged);

    //Double view -> Main data manager
    connect(m_pView->getDoubleView(), &CDoubleView::doSetSelectedDisplay, m_pModel->getDataManager(), &CMainDataManager::onSetSelectedDisplay);

    //Data viewer -> Main data manager
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doSaveCurrentVideoFrame, m_pModel->getDataManager(), &CMainDataManager::onSaveCurrentVideoFrame);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doExportCurrentImage, m_pModel->getDataManager(), &CMainDataManager::onExportCurrentImage);
    connect(m_pView->getDoubleView()->getDataViewer(), &CDataViewer::doExportCurrentVideoFrame, m_pModel->getDataManager(), &CMainDataManager::onExportCurrentVideoFrame);
}

void CMainCtrl::show()
{
    assert(m_pView && m_pModel);

    initConnections();
    QPoint globalCursorPos = QCursor::pos();
    auto currentScreen = QGuiApplication::screenAt(globalCursorPos);
    QRect screenRect = currentScreen->availableGeometry();
    m_pView->move(QPoint(screenRect.x(), screenRect.y()));
    m_pView->setCustomGeometry(QRect(screenRect.topLeft(), QSize(screenRect.width()/2, screenRect.height()/2)));
    m_pView->showMaximized();
    m_pModel->notifyViewShow();

    // Tutorials wizard
    launchWizard();
}

void CMainCtrl::launchWizard()
{
    // Tutorials standby
    // Init tutorial database
    m_tutorialMgr.initTutorials();

    //m_wizardMgr.launchTutorialHelper();
    m_tutorialMgr.launchFirstStepHelper();
}
