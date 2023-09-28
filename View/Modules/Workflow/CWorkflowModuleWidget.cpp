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

#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include "CWorkflowModuleWidget.h"
#include "CWorkflowView.h"
#include "CWorkflowScene.h"
#include "CWorkflowNewDlg.h"
#include "CWorkflowPublishDlg.h"
#include "View/Common/CToolbarBorderLayout.h"
#include "View/Common/CRollupWidget.h"
#include "View/Process/CProcessDocDlg.h"
#include "View/Process/CProcessDocWidget.h"
#include "Model/Project/CProjectGraphicsProxyModel.h"
#include "Model/Project/CProjectDataProxyModel.h"
#include <Widgets/QtPropertyBrowser/qttreepropertybrowser.h>
#include <Widgets/QtPropertyBrowser/qtbuttonpropertybrowser.h>
#include "Widgets/QtPropertyBrowser/qtvariantproperty.h"
#include "Main/AppTools.hpp"

CWorkflowModuleWidget::CWorkflowModuleWidget(QWidget *parent) : QWidget(parent)
{
    m_name = tr("Workflow Creator");
    setWindowTitle(m_name);
    initLayout();
}

CWorkflowModuleWidget::~CWorkflowModuleWidget()
{
    CPyEnsureGIL gil;
    m_widgetPtr.reset();
}

void CWorkflowModuleWidget::setCurrentUser(const CUser &user)
{
    if(m_pProcessDocDlg)
        m_pProcessDocDlg->setCurrentUser(user);
}

CWorkflowView *CWorkflowModuleWidget::getView() const
{
    return m_pView;
}

void CWorkflowModuleWidget::onSetModel(CWorkflowManager *pModel)
{
    m_pModel = pModel;
    m_pView->setModel(pModel);
    initConnections();
}

void CWorkflowModuleWidget::onAddTask(const WorkflowTaskPtr &pTask, const WorkflowVertex &id, const WorkflowVertex &parentId)
{
    m_pView->addTask(pTask, id, parentId);
}

void CWorkflowModuleWidget::onAddCandidateTask(const WorkflowTaskPtr &pTask, const WorkflowVertex &id)
{
    m_pView->addTask(pTask, id);
}

void CWorkflowModuleWidget::onSetScaleProjects(const QJsonArray &projects, const CUser& user)
{
    QString wfName = QString::fromStdString(m_pModel->getWorkflowName());
    QString wfDescription = QString::fromStdString(m_pModel->getWorkflowDescription());
    CWorkflowPublishDlg publishFormDlg(wfName, wfDescription, projects, user, this);

    if (publishFormDlg.exec() == QDialog::Accepted)
    {
        wfName = publishFormDlg.getWorkflowName();
        wfDescription = publishFormDlg.getWorkflowDescription();
        bool bNewProject = publishFormDlg.isNewProject();
        QString projectName = publishFormDlg.getProjectName();
        QString projectDescription = publishFormDlg.getProjectDescription();
        QString namespacePath = publishFormDlg.getNamespacePath();
        m_pModel->publishWorkflow(wfName, wfDescription, bNewProject, projectName, projectDescription, namespacePath);
    }
}

void CWorkflowModuleWidget::onUpdateTaskInfo(const WorkflowTaskPtr &pTask, const CTaskInfo& info)
{
    // Clear parameters layout before creating widget because of shared_ptr
    clearParamLayout();

    // Create new widget shared pointer
    m_widgetPtr = m_pModel->createTaskWidget(pTask);
    if(m_widgetPtr != nullptr)
    {
        // Add it to layout
        m_pParamLayout->addWidget(m_widgetPtr.get());
        // Make somme connections
        connect(m_widgetPtr.get(), &CWorkflowTaskWidget::doApplyProcess, [&](const WorkflowTaskParamPtr& pParam)
        {
            emit doRunFromActiveTask(pParam);
        });
        connect(m_widgetPtr.get(), &CWorkflowTaskWidget::doSendProcessAction, [&, pTask](int flags)
        {
            emit doSendProcessAction(pTask, flags);
        });
        connect(m_widgetPtr.get(), &CWorkflowTaskWidget::doSetGraphicsTool, [&](GraphicsShape tool)
        {
            emit doSetGraphicsTool(tool);
        });
        connect(m_widgetPtr.get(), &CWorkflowTaskWidget::doSetGraphicsCategory, [&](const QString& category)
        {
            emit doSetGraphicsCategory(category);
        });
    }
    else
        m_pParamLayout->addWidget(new QWidget);

    // Fill I/O information
    fillIOProperties(pTask);

    //Update process info
    m_currentProcessInfo = info;
    if(m_pProcessDocDlg->isVisible())
        m_pProcessDocDlg->setProcessInfo(m_currentProcessInfo);
}

void CWorkflowModuleWidget::onUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap)
{
    setProcessInfoMap(infoMap, customInfoMap);
}

void CWorkflowModuleWidget::onWorkflowCreated()
{
    m_pView->manageWorkflowCreated();
    setTitle(m_name + " - " + QString::fromStdString(m_pModel->getWorkflowName()));
}

void CWorkflowModuleWidget::onSaveWorkflow()
{
    if(m_pModel == nullptr)
        return;

     if(m_pModel->isWorkflowExists() == false)
         return;

    auto wfNames = m_pModel->getWorkflowNames();
    QString wfName = QString::fromStdString(m_pModel->getWorkflowName());

    if(!wfNames.contains(wfName))
    {
        CWorkflowNewDlg newWorkflowDlg(wfName, wfNames, this);
        if(newWorkflowDlg.exec() == QDialog::Accepted)
        {
            m_pModel->setWorkflowName(newWorkflowDlg.getName().toStdString());
            m_pModel->setWorkflowKeywords(newWorkflowDlg.getKeywords().toStdString());
            m_pModel->setWorkflowDescription(newWorkflowDlg.getDescription().toStdString());
            setTitle(m_name + " - " + newWorkflowDlg.getName());
        }
        else
            return;
    }
    m_pModel->saveWorkflow();
}

void CWorkflowModuleWidget::onExportWorkflow()
{
    if(m_pModel == nullptr)
        return;

     if(m_pModel->isWorkflowExists() == false)
         return;

    auto wfNames = m_pModel->getWorkflowNames();
    QString wfName = QString::fromStdString(m_pModel->getWorkflowName());

    if(!wfNames.contains(wfName))
    {
        CWorkflowNewDlg newWorkflowDlg(wfName, wfNames, this);
        if(newWorkflowDlg.exec() == QDialog::Accepted)
        {
            m_pModel->setWorkflowName(newWorkflowDlg.getName().toStdString());
            m_pModel->setWorkflowKeywords(newWorkflowDlg.getKeywords().toStdString());
            m_pModel->setWorkflowDescription(newWorkflowDlg.getDescription().toStdString());
        }
        else
            return;
    }

    QSettings IkomiaSettings;
    QStringList extensions = {"json", "pcl"};

    auto fileName = Utils::File::saveFile(this, tr("Export workflow"), IkomiaSettings.value(_defaultDirWorkflowExport).toString(), tr("Workflow (*.json *.pcl)"), extensions, ".json");
    if(fileName.isEmpty())
        return;

    IkomiaSettings.setValue(_defaultDirWorkflowExport, QFileInfo(fileName).path());
    m_pModel->saveWorkflow(fileName);
}

void CWorkflowModuleWidget::onPublishWorkflow()
{
    if (m_pModel == nullptr)
        return;

    if (m_pModel->isWorkflowExists() == false)
        return;

    m_pModel->requestScaleProjects();
}

void CWorkflowModuleWidget::onLoadWorkflow()
{
    assert(m_pModel);
    QSettings IkomiaSettings;

    QString path = QFileDialog::getOpenFileName(nullptr, tr("Load workflow"), IkomiaSettings.value(_defaultDirWorkflow).toString(), tr("Workflow (*.json *.pcl)"), nullptr, CSettingsManager::dialogOptions());
    if(path.isNull() == false)
    {
        m_lastPathWorkflow = QFileInfo(path).path(); // store path for next time
        IkomiaSettings.setValue(_defaultDirWorkflow, m_lastPathWorkflow);
        m_pModel->loadWorkflow(path);
        setTitle(m_name + " - " + QString::fromStdString(m_pModel->getWorkflowName()));
    }
}

void CWorkflowModuleWidget::onCloseWorkflow()
{
    assert(m_pModel);

    if(m_pModel->isWorkflowRunning())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The workflow is running. Please wait before closing it."));
        msgBox.exec();
        return;
    }

    if(m_pModel->isWorkflowModified() == true)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The workflow has been modified."));
        msgBox.setInformativeText(tr("Do you want to save the changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int ret = msgBox.exec();
        if(ret == QMessageBox::Cancel)
            return;
        else if(ret == QMessageBox::Save)
            m_pModel->saveWorkflow();
    }
    m_pView->clear();
    clearTabWidget();
    setTitle(m_name);
    emit doNotifyWorkflowClosed();
}

void CWorkflowModuleWidget::onNewWorkflow()
{
    assert(m_pModel);

    if(m_pModel->isWorkflowExists())
        onCloseWorkflow();

    CWorkflowNewDlg newWorkflowDlg("", m_pModel->getWorkflowNames(), this);
    if(newWorkflowDlg.exec() == QDialog::Accepted)
    {
        m_pModel->createWorkflow(newWorkflowDlg.getName().toStdString(),
                                 newWorkflowDlg.getKeywords().toStdString(),
                                 newWorkflowDlg.getDescription().toStdString());
        setTitle(m_name + " - " + newWorkflowDlg.getName());
    }
}

void CWorkflowModuleWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_pProcessDocDlg->hide();
}

void CWorkflowModuleWidget::onShowProcessInfo()
{
    if(m_pProcessDocDlg->isHidden())
    {
        m_pProcessDocDlg->setProcessInfo(m_currentProcessInfo);
        m_pProcessDocDlg->show();
        adjustProcessDocDlgPos();
    }
    else
        m_pProcessDocDlg->hide();
}

void CWorkflowModuleWidget::onIOPropertyValueChanged(QtProperty *pProperty, const QVariant &value)
{
    assert(m_pModel);

    auto it = m_ioProperties.find(pProperty);
    if(it == m_ioProperties.end())
        return;

    auto type = it.value().type;
    switch(type)
    {
        case IOPropType::AUTO_SAVE:
        {
            auto outputIndex = (size_t)it.value().data.toInt();
            m_pModel->setCurrentTaskAutoSave(outputIndex, value.toBool());
            break;
        }
        case IOPropType::SAVE_FOLDER:
        {
            m_pModel->setCurrentTaskSaveFolder(value.toString().toStdString());
            break;
        }
        case IOPropType::SAVE_FORMAT:
        {
            auto outputIndex = (size_t)it.value().data.toInt();
            m_pModel->setCurrentTaskSaveFormat(outputIndex, value.toInt());
            break;
        }
    }
}

void CWorkflowModuleWidget::initLayout()
{
    m_pView = new CWorkflowView;

    //Popup process info - non modal
    m_pProcessDocDlg = new CProcessDocDlg(CDialog::DEFAULT_FIXED, this);

    m_pTab = new QTabWidget;
    m_pTab->setMinimumWidth(300);
    m_pTab->setMinimumHeight(250);

    QWidget* pContainerTab = new QWidget;
    QVBoxLayout* pContainerTabLayout = new QVBoxLayout;
    pContainerTabLayout->addWidget(m_pTab);
    pContainerTab->setLayout(pContainerTabLayout);

    QWidget* pContainerView = new QWidget;
    QVBoxLayout* pContainerViewLayout = new QVBoxLayout;
    pContainerViewLayout->addWidget(m_pView);
    pContainerView->setLayout(pContainerViewLayout);

    //Process information button
    QPushButton* pInfoBtn = new QPushButton(QIcon(":/Images/info-color2.png"), "");
    pInfoBtn->setFixedSize(22, 22);
    pInfoBtn->setToolTip(tr("Documentation"));
    pInfoBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}")
                                       .arg(qApp->palette().highlight().color().name()));
    connect(pInfoBtn, &QPushButton::clicked, this, &CWorkflowModuleWidget::onShowProcessInfo);

    // Create custom tab and get layout for later modifications
    m_pParamLayout = createTab(QIcon(":/Images/properties_white.png"), tr("Parameters"), pInfoBtn);
    m_pInfoLayout = createTab(QIcon(":/Images/info-white.png"), tr("Info"), nullptr);
    m_pIOLayout = createTab(QIcon(":/Images/io.png"), tr("I/O"), nullptr);

    // Object to create property for QtTreePropertyBrowser
    m_pVariantManager = new QtVariantPropertyManager(this);
    connect(m_pVariantManager, &QtVariantPropertyManager::valueChanged, this, &CWorkflowModuleWidget::onIOPropertyValueChanged);

    // Init info layout
    m_pInfoPropertyList = new QtTreePropertyBrowser(this);
    m_pInfoPropertyList->setHeaderVisible(false);
    m_pInfoPropertyList->setPropertiesWithoutValueMarked(true);
    m_pInfoLayout->addWidget(m_pInfoPropertyList);

    // Init IO layout
    m_pIOPropertyList = new QtTreePropertyBrowser(this);
    QtVariantEditorFactory* pEditorFactory = new QtVariantEditorFactory();
    m_pIOPropertyList->setFactoryForManager(m_pVariantManager, pEditorFactory);
    m_pIOPropertyList->setHeaderVisible(false);
    m_pIOPropertyList->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
    m_pIOPropertyList->setPropertiesWithoutValueMarked(true);
    m_pIOLayout->addWidget(m_pIOPropertyList);

    QSplitter* pSplitter = new QSplitter(this);
    pSplitter->setOrientation(Qt::Horizontal);
    pSplitter->addWidget(pContainerView);
    pSplitter->addWidget(pContainerTab);
    pSplitter->setStretchFactor(0, 5);
    pSplitter->setStretchFactor(1, 1);

    auto pLayout = new CToolbarBorderLayout;
    pLayout->addLeftBar(QSize(50, 24), 5);
    pLayout->addTopBar(QSize(24, 24), 5);
    pLayout->addWidget(pSplitter, CBorderLayout::Center);
    setLayout(pLayout);

    initLeftTab();
    initTopTab();
}

void CWorkflowModuleWidget::initLeftTab()
{
    QSize btnSize = QSize(50, 24);
    auto pLayout = static_cast<CToolbarBorderLayout*>(layout());

    // Add button to create new protocol
    auto pBtnNew = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/add.png"));
    assert(pBtnNew != nullptr);
    pBtnNew->setToolTip(tr("New workflow"));
    connect(pBtnNew, &QPushButton::clicked, this, &CWorkflowModuleWidget::onNewWorkflow);

    // Add button to load protocol from file
    auto pBtnLoad = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/open.png"));
    assert(pBtnLoad != nullptr);
    pBtnLoad->setToolTip(tr("Load workflow"));
    connect(pBtnLoad, &QPushButton::clicked, this, &CWorkflowModuleWidget::onLoadWorkflow);

    // Add button to save current protocol
    auto pBtnSave = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/save.png"));
    assert(pBtnSave != nullptr);
    pBtnSave->setToolTip(tr("Save workflow"));
    connect(pBtnSave, &QPushButton::clicked, this, &CWorkflowModuleWidget::onSaveWorkflow);

    // Add button to export current protocol
    auto pBtnExport = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/export.png"));
    assert(pBtnExport != nullptr);
    pBtnExport->setToolTip(tr("Export workflow"));
    connect(pBtnExport, &QPushButton::clicked, this, &CWorkflowModuleWidget::onExportWorkflow);

    // Add button to publish workflow to Scale
    auto pBtnPublish = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/share.png"));
    assert(pBtnPublish != nullptr);
    pBtnPublish->setToolTip(tr("Publish workflow to Ikomia Scale"));
    connect(pBtnPublish, &QPushButton::clicked, this, &CWorkflowModuleWidget::onPublishWorkflow);

    // Add button to close current protocol
    auto pBtnClose = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/close-workflow.png"));
    assert(pBtnClose != nullptr);
    pBtnClose->setToolTip(tr("Close workflow"));
    connect(pBtnClose, &QPushButton::clicked, this, &CWorkflowModuleWidget::onCloseWorkflow);
}

void CWorkflowModuleWidget::initTopTab()
{
    QSize btnSize = QSize(24, 24);
    auto pLayout = static_cast<CToolbarBorderLayout*>(layout());

    //Add buttons to set input visualization mode
    auto pBtnOriginInput = addButtonToTop(btnSize, QIcon(":Images/original-input.png"), tr("Visualize origin inputs"), true);
    pBtnOriginInput->setChecked(true);
    connect(pBtnOriginInput, &QPushButton::clicked, [=]{ m_pModel->setInputViewMode(WorkflowInputViewMode::ORIGIN); });

    auto pBtnCurrentInput = addButtonToTop(btnSize, QIcon(":Images/current-input.png"), tr("Visualize current inputs"), true);
    connect(pBtnCurrentInput, &QPushButton::clicked, [&]{ m_pModel->setInputViewMode(WorkflowInputViewMode::CURRENT); });

    //Add button to update plugin task
    pLayout->addSeparatorToTop();
    auto pBtnUpdatePlugins = addButtonToTop(btnSize, QIcon(":Images/update.png"), tr("Reload plugins of current workflow"), false);
    connect(pBtnUpdatePlugins, &QPushButton::clicked, [&]{ m_pModel->reloadCurrentPlugins(); });

    QButtonGroup* pInputViewModeGroup = new QButtonGroup(this);
    pInputViewModeGroup->addButton(pBtnOriginInput);
    pInputViewModeGroup->addButton(pBtnCurrentInput);

    //Add button to run protocol
    pLayout->addSeparatorToTop();
    auto pBtnRun = addButtonToTop(btnSize, QIcon(":/Images/play.png"), tr("Run workflow"));
    connect(pBtnRun, &QPushButton::clicked, [&]{ m_pModel->runWorkflow(); });

    //Add button to run protocol from current task
    auto pBtnRunFrom = addButtonToTop(btnSize, QIcon(":/Images/play-from.png"), tr("Run workflow from current task"));
    connect(pBtnRunFrom, &QPushButton::clicked, [&]{ m_pModel->runWorkflowFromActiveTask(); });

    //Add button to run protocol from start to current task
    auto pBtnRunTo = addButtonToTop(btnSize, QIcon(":/Images/play-to.png"), tr("Run workflow from start to current task"));
    connect(pBtnRunTo, &QPushButton::clicked, [&]{ m_pModel->runWorkflowToActiveTask(); });

    //Add button to stop protocol
    auto pBtnStop = addButtonToTop(btnSize, QIcon(":/Images/stop.png"), tr("Stop workflow"));
    connect(pBtnStop, &QPushButton::clicked, [&]{ m_pModel->stopWorkflow(); });

    //Add zoom buttons
    pLayout->addSeparatorToTop();
    auto pBtnZoomOriginal = addButtonToTop(btnSize, QIcon(":/Images/zoom-original.png"), tr("Original size"));
    connect(pBtnZoomOriginal, &QPushButton::clicked, [&]{ m_pView->zoomInit(); });

    auto pBtnZoomIn = addButtonToTop(btnSize, QIcon(":/Images/zoom-in.png"), tr("Zoom in"));
    connect(pBtnZoomIn, &QPushButton::clicked, [&]{ m_pView->zoomIn(); });

    auto pBtnZoomOut = addButtonToTop(btnSize, QIcon(":/Images/zoom-out.png"), tr("Zoom out"));
    connect(pBtnZoomOut, &QPushButton::clicked, [&]{ m_pView->zoomOut(); });
}

void CWorkflowModuleWidget::initConnections()
{
    //Model -> module
    connect(m_pModel, &CWorkflowManager::doAddCandidateTask, this, &CWorkflowModuleWidget::onAddCandidateTask);
    connect(m_pModel, &CWorkflowManager::doAddTask, this, &CWorkflowModuleWidget::onAddTask);
    connect(m_pModel, &CWorkflowManager::doWorkflowCreated, this, &CWorkflowModuleWidget::onWorkflowCreated);
    connect(m_pModel, &CWorkflowManager::doUpdateTaskInfo, this, &CWorkflowModuleWidget::onUpdateTaskInfo);
    connect(m_pModel, &CWorkflowManager::doUpdateTaskIOInfo, [&](const WorkflowTaskPtr& taskPtr)
    {
        fillIOProperties(taskPtr);
    });
    connect(m_pModel, &CWorkflowManager::doUpdateTaskStateInfo, this, &CWorkflowModuleWidget::onUpdateTaskStateInfo);
    connect(m_pModel, &CWorkflowManager::doCloseWorkflow, this, &CWorkflowModuleWidget::onCloseWorkflow);
    connect(m_pModel, &CWorkflowManager::doSetScaleProjects, this, &CWorkflowModuleWidget::onSetScaleProjects);

    //Model -> view
    connect(m_pModel, &CWorkflowManager::doInputChanged, m_pView, &CWorkflowView::onInputChanged);
    connect(m_pModel, &CWorkflowManager::doInputAssigned, m_pView, &CWorkflowView::onInputAssigned);
    connect(m_pModel, &CWorkflowManager::doInputRemoved, m_pView, &CWorkflowView::onInputRemoved);
    connect(m_pModel, &CWorkflowManager::doInputsCleared, m_pView, &CWorkflowView::onInputsCleared);
    connect(m_pModel, &CWorkflowManager::doAddConnection, m_pView, &CWorkflowView::onAddConnection);
    connect(m_pModel, &CWorkflowManager::doUpdateCandidateTask, m_pView, &CWorkflowView::onUpdateCandidateTask);
    connect(m_pModel, &CWorkflowManager::doUpdateTaskItemView, m_pView, &CWorkflowView::onUpdateTaskItem);
    connect(m_pModel, &CWorkflowManager::doSetActiveTask, m_pView, &CWorkflowView::onSetActiveTask);
    connect(m_pModel, &CWorkflowManager::doSetTaskState, m_pView, &CWorkflowView::onSetTaskState);
    connect(m_pModel, &CWorkflowManager::doDeleteConnection, m_pView, &CWorkflowView::onDeleteConnectionFromModel);
    connect(m_pModel, &CWorkflowManager::doSetIOInfo, m_pView, &CWorkflowView::onSetIOInfo);

    //Model -> scene
    connect(m_pModel, &CWorkflowManager::doSetGraphicsProxyModel, m_pView->getScene(), &CWorkflowScene::onSetGraphicsProxyModel);
    connect(m_pModel, &CWorkflowManager::doSetProjectDataProxyModel, m_pView->getScene(), &CWorkflowScene::onSetProjectDataProxyModel);

    //Module -> model
    connect(this, &CWorkflowModuleWidget::doRunFromActiveTask, m_pModel, &CWorkflowManager::onRunFromActiveTask);
    connect(this, &CWorkflowModuleWidget::doSetGraphicsTool, m_pModel, &CWorkflowManager::onSetGraphicsTool);
    connect(this, &CWorkflowModuleWidget::doSetGraphicsCategory, m_pModel, &CWorkflowManager::onSetGraphicsCategory);
    connect(this, &CWorkflowModuleWidget::doSendProcessAction, m_pModel, &CWorkflowManager::onSendProcessAction);

    //Scene -> model
    auto pScene = static_cast<CWorkflowScene*>(m_pView->scene());
    connect(pScene, &CWorkflowScene::doSelectionChange, m_pModel, &CWorkflowManager::onActiveTaskChanged);
    connect(pScene, &CWorkflowScene::doItemConnection, m_pModel, &CWorkflowManager::onConnectTask);
    connect(pScene, &CWorkflowScene::doSetTaskActionFlag, m_pModel, &CWorkflowManager::onSetTaskActionFlag);
    connect(pScene, &CWorkflowScene::doSetGraphicsLayerInput, m_pModel, &CWorkflowManager::onSetGraphicsLayerInput);
    connect(pScene, &CWorkflowScene::doSetFolderPathInput, m_pModel, &CWorkflowManager::onSetFolderPathInput);
    connect(pScene, &CWorkflowScene::doSetInput, m_pModel, &CWorkflowManager::onSetInput);
    connect(pScene, &CWorkflowScene::doQueryGraphicsProxyModel, m_pModel, &CWorkflowManager::onQueryGraphicsProxyModel);
    connect(pScene, &CWorkflowScene::doQueryProjectDataProxyModel, m_pModel, &CWorkflowManager::onQueryProjectDataProxyModel);
    connect(pScene, &CWorkflowScene::doQueryIOInfo, m_pModel, &CWorkflowManager::onQueryIOInfo);
    connect(pScene, &CWorkflowScene::doAddInput, m_pModel, &CWorkflowManager::onAddInput);
    connect(pScene, &CWorkflowScene::doDeleteInput, m_pModel, &CWorkflowManager::onDeleteInput);

    //Scene -> main view
    connect(pScene, &CWorkflowScene::doShowProcessPopup, [&]{ emit doShowProcessPopup(); });
}

QVBoxLayout* CWorkflowModuleWidget::createTab(QIcon icon, QString title, QWidget* pBtn)
{
    assert(m_pTab);

    QWidget* pWidget = new QWidget;
    QVBoxLayout* pLayout = new QVBoxLayout;

    pLayout->setAlignment(Qt::AlignTop);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);
    pLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    QWidget* pContainer = new QWidget;
    pContainer->setLayout(pLayout);

    QScrollArea* pScrollArea = new QScrollArea;
    pScrollArea->setFrameShape(QFrame::NoFrame);
    pScrollArea->setWidget(pContainer);
    pScrollArea->setWidgetResizable(true);

    QVBoxLayout* pOuterLayout = new QVBoxLayout;
    pOuterLayout->setContentsMargins(5, 5, 5, 5);
    pOuterLayout->setSpacing(0);
    pOuterLayout->setAlignment(Qt::AlignTop);
    pOuterLayout->addWidget(pScrollArea);

    pWidget->setLayout(pOuterLayout);

    m_pTab->addTab(pWidget, icon, title);

    if(pBtn != nullptr)
    {
        auto pTabBar = m_pTab->tabBar();
        pTabBar->setTabButton(pTabBar->count() - 1, QTabBar::RightSide, pBtn);
    }
    return pLayout;
}

QToolButton *CWorkflowModuleWidget::addButtonToTop(const QSize& size, const QIcon &icon, const QString& tooltip, bool bCheckable)
{
    auto pLayout = static_cast<CToolbarBorderLayout*>(layout());
    assert(pLayout);
    auto pBtn = pLayout->addButtonToTop("", size, icon);
    assert(pBtn != nullptr);
    //pBtn->setFlat(true);
    pBtn->setToolTip(tooltip);

    if(bCheckable)
    {
        auto highLightColor = qApp->palette().highlight().color();
        QColor checkedColor(highLightColor.red(), highLightColor.green(), highLightColor.blue(), 128);
        pBtn->setStyleSheet(QString("QPushButton:checked { border-radius: 5px; background-color: rgba(%2,%3,%4,%5); }")
                            .arg(checkedColor.red()).arg(checkedColor.green()).arg(checkedColor.blue()).arg(checkedColor.alpha()));
        pBtn->setCheckable(true);
    }
    return pBtn;
}

void CWorkflowModuleWidget::setTitle(const QString &title)
{
    setWindowTitle(title);
    emit doUpdateTitle();
}

void CWorkflowModuleWidget::remove(QLayout* pLayout)
{
    QLayoutItem* child;
    while(pLayout->count()!=0)
    {
        child = pLayout->takeAt(0);
        if(child->layout() != 0)
            remove(child->layout());
        else if(child->widget() != 0)
            delete child->widget();

        delete child;
    }
}

void CWorkflowModuleWidget::clearParamLayout()
{
    CPyEnsureGIL gil;
    m_widgetPtr.reset();

    while(!m_pParamLayout->isEmpty())
    {
        QWidget* pWidget = m_pParamLayout->takeAt(0)->widget();
        m_pParamLayout->removeWidget(pWidget);
        pWidget->hide();
        pWidget->deleteLater();
    }
}

void CWorkflowModuleWidget::clearTabWidget()
{
    // Parameters
    clearParamLayout();
    m_pParamLayout->addWidget(new QWidget);
    // Execution time information
    m_pInfoPropertyList->clear();
    // I/O information
    m_pIOPropertyList->clear();
}

void CWorkflowModuleWidget::fillIOProperties(const WorkflowTaskPtr &taskPtr)
{
    // Clear process properties for new values
    m_pIOPropertyList->clear();
    m_ioProperties.clear();

    // Inputs information
    QtProperty* pInputsGroup = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Inputs"));
    QtBrowserItem* pInputsItem = m_pIOPropertyList->addProperty(pInputsGroup);
    m_pIOPropertyList->setBackgroundColor(pInputsItem, qApp->palette().base().color());

    for(size_t i=0; i<taskPtr->getInputCount(); ++i)
    {
        auto inputPtr = taskPtr->getInput(i);
        if(!inputPtr)
            continue;

        auto inputName = QString("#%1 - ").arg(i+1) + Utils::Workflow::getIODataName(inputPtr->getDataType());
        QtVariantProperty* pInputProp = m_pVariantManager->addProperty(QVariant::String, inputName);
        pInputProp->setValue(QString::fromStdString(inputPtr->getDescription()));
        pInputProp->setEnabled(false);
        pInputsGroup->addSubProperty(pInputProp);
    }

    // Outputs information
    QtProperty* pOutputsGroup = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Outputs"));
    QtBrowserItem* pOutputsProp = m_pIOPropertyList->addProperty(pOutputsGroup);
    m_pIOPropertyList->setBackgroundColor(pOutputsProp, qApp->palette().base().color());

    for(size_t i=0; i<taskPtr->getOutputCount(); ++i)
    {
        auto outputPtr = taskPtr->getOutput(i);
        if(!outputPtr)
            continue;

        auto outputName = QString("#%1 - ").arg(i+1) + Utils::Workflow::getIODataName(outputPtr->getDataType());
        QtProperty* pOutputGroup = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), outputName);
        pOutputsGroup->addSubProperty(pOutputGroup);

        // Description
        QtVariantProperty* pDescProp = m_pVariantManager->addProperty(QVariant::String, tr("Description"));
        pDescProp->setValue(QString::fromStdString(outputPtr->getDescription()));
        pDescProp->setEnabled(false);
        pOutputGroup->addSubProperty(pDescProp);

        // Auto-save property
        QtVariantProperty* pAutoSaveProp = m_pVariantManager->addProperty(QVariant::Bool, tr("Auto save"));
        pAutoSaveProp->setValue(outputPtr->isAutoSave());
        PropAttribute propAttr;
        propAttr.type = IOPropType::AUTO_SAVE;
        propAttr.data = (int)i;
        m_ioProperties.insert(pAutoSaveProp, propAttr);
        pOutputGroup->addSubProperty(pAutoSaveProp);

        // Save format
        auto saveFormats = outputPtr->getPossibleSaveFormats();
        if(saveFormats.size() > 0)
        {
            QStringList formatNames;
            int currentFormatIndex = 0;
            auto currentFormat = outputPtr->getSaveFormat();

            for(size_t i=0; i<saveFormats.size(); ++i)
            {
                formatNames.append(Utils::Data::getExportFormatName(saveFormats[i]));
                if(saveFormats[i] == currentFormat)
                    currentFormatIndex = i;
            }

            QtVariantProperty* pSaveFormatProp = m_pVariantManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Save format"));
            pSaveFormatProp->setAttribute("enumNames", formatNames);
            pSaveFormatProp->setValue(currentFormatIndex);
            pOutputGroup->addSubProperty(pSaveFormatProp);

            PropAttribute propAttr;
            propAttr.type = IOPropType::SAVE_FORMAT;
            propAttr.data = (int)i;
            m_ioProperties.insert(pSaveFormatProp, propAttr);
        }
    }

    // Export options
    QtProperty* pExportGroup = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Save options"));
    pOutputsGroup->addSubProperty(pExportGroup);

    QtVariantProperty* pSaveFolderProp = m_pVariantManager->addProperty(QtVariantPropertyManager::filePathTypeId(), tr("Folder"));
    pExportGroup->addSubProperty(pSaveFolderProp);
    pSaveFolderProp->setValue(QString::fromStdString(taskPtr->getOutputFolder()));
    pSaveFolderProp->setAttribute("pathMode", static_cast<int>(QFileDialog::Directory));
    PropAttribute propAttr;
    propAttr.type = IOPropType::SAVE_FOLDER;
    m_ioProperties.insert(pSaveFolderProp, propAttr);
}

void CWorkflowModuleWidget::setProcessInfoMap(const VectorPairString& infoMap, const VectorPairString& customInfoMap)
{
    // Clear process properties for new values
    m_pInfoPropertyList->clear();
    // Add property groups and fill them
    setGroupProperties(m_pInfoPropertyList, tr("Process benchmark"), infoMap);
    setGroupProperties(m_pInfoPropertyList, tr("Custom process information"), customInfoMap);
    // Apply style
    applyPropertyListStyle(m_pInfoPropertyList);
}

void CWorkflowModuleWidget::setGroupProperties(QtTreePropertyBrowser* pPropList, const QString& title, const VectorPairString& properties)
{
    if(properties.empty())
        return;

    // Top item for process benchmark
    QtProperty* pCommonItem = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), title);

    // Add top item to browser
    pPropList->addProperty(pCommonItem);

    // Fill all values
    fillProperty(pCommonItem, properties);
}

void CWorkflowModuleWidget::fillProperty(QtProperty* pItem, const VectorPairString& properties)
{
    for(auto&& it : properties)
    {
        QtVariantProperty *item = m_pVariantManager->addProperty(QVariant::String, QString::fromStdString(it.first));
        item->setValue(QString::fromStdString(it.second));
        pItem->addSubProperty(item);
    }
}

void CWorkflowModuleWidget::applyPropertyListStyle(QtTreePropertyBrowser *pPropList)
{
    // Set browser line color
    auto list = pPropList->topLevelItems();
    QPalette p = qApp->palette();

    for(auto&& it : list)
        pPropList->setBackgroundColor(it, p.base().color());
}

void CWorkflowModuleWidget::adjustProcessDocDlgPos()
{
    const QRect screen = QApplication::desktop()->availableGeometry(this);
    const QRect rect = m_pProcessDocDlg->geometry();
    int x = screen.right() - rect.width() + m_pProcessDocDlg->getBorderSize();
    int y = screen.bottom() - rect.height();
    m_pProcessDocDlg->move(x, y);
}

#include "moc_CWorkflowModuleWidget.cpp"
