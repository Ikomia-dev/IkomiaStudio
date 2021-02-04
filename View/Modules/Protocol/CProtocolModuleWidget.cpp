#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include "CProtocolModuleWidget.h"
#include "CProtocolView.h"
#include "CProtocolScene.h"
#include "CProtocolNewDlg.h"
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

CProtocolModuleWidget::CProtocolModuleWidget(QWidget *parent) : QWidget(parent)
{
    m_name = tr("Workflow Creator");
    setWindowTitle(m_name);
    initLayout();
}

void CProtocolModuleWidget::setCurrentUser(const CUser &user)
{
    if(m_pProcessDocDlg)
        m_pProcessDocDlg->setCurrentUser(user);
}

CProtocolView *CProtocolModuleWidget::getView() const
{
    return m_pView;
}

void CProtocolModuleWidget::onSetModel(CProtocolManager *pModel)
{
    m_pModel = pModel;
    m_pView->setModel(pModel);
    initConnections();
}

void CProtocolModuleWidget::onAddTask(const ProtocolTaskPtr &pTask, const ProtocolVertex &id, const ProtocolVertex &parentId)
{
    m_pView->addTask(pTask, id, parentId);
}

void CProtocolModuleWidget::onAddCandidateTask(const ProtocolTaskPtr &pTask, const ProtocolVertex &id)
{
    m_pView->addTask(pTask, id);
}

void CProtocolModuleWidget::onUpdateTaskInfo(const ProtocolTaskPtr &pTask, const CProcessInfo& info)
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
        connect(m_widgetPtr.get(), &CProtocolTaskWidget::doApplyProcess, [&](const ProtocolTaskParamPtr& pParam)
        {
            emit doRunFromActiveTask(pParam);
        });
        connect(m_widgetPtr.get(), &CProtocolTaskWidget::doSendProcessAction, [&, pTask](int flags)
        {
            emit doSendProcessAction(pTask, flags);
        });
        connect(m_widgetPtr.get(), &CProtocolTaskWidget::doSetGraphicsTool, [&](GraphicsShape tool)
        {
            emit doSetGraphicsTool(tool);
        });
        connect(m_widgetPtr.get(), &CProtocolTaskWidget::doSetGraphicsCategory, [&](const QString& category)
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

void CProtocolModuleWidget::onUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap)
{
    setProcessInfoMap(infoMap, customInfoMap);
}

void CProtocolModuleWidget::onProtocolCreated()
{
    m_pView->manageProtocolCreated();
    setTitle(m_name + " - " + QString::fromStdString(m_pModel->getProtocolName()));
}

void CProtocolModuleWidget::onSaveProtocol()
{
    if(m_pModel == nullptr)
        return;

     if(m_pModel->isProtocolExists() == false)
         return;

    auto protocolNames = m_pModel->getProtocolNames();
    QString protocolName = QString::fromStdString(m_pModel->getProtocolName());

    if(!protocolNames.contains(protocolName))
    {
        CProtocolNewDlg newProtocolDlg(protocolName, protocolNames, this);
        if(newProtocolDlg.exec() == QDialog::Accepted)
        {
            m_pModel->setProtocolName(newProtocolDlg.getName().toStdString());
            m_pModel->setProtocolKeywords(newProtocolDlg.getKeywords().toStdString());
            m_pModel->setProtocolDescription(newProtocolDlg.getDescription().toStdString());
            setTitle(m_name + " - " + newProtocolDlg.getName());
        }
        else
            return;
    }
    m_pModel->saveProtocol();
}

void CProtocolModuleWidget::onExportProtocol()
{
    if(m_pModel == nullptr)
        return;

     if(m_pModel->isProtocolExists() == false)
         return;

    auto protocolNames = m_pModel->getProtocolNames();
    QString protocolName = QString::fromStdString(m_pModel->getProtocolName());

    if(!protocolNames.contains(protocolName))
    {
        CProtocolNewDlg newProtocolDlg(protocolName, protocolNames, this);
        if(newProtocolDlg.exec() == QDialog::Accepted)
        {
            m_pModel->setProtocolName(newProtocolDlg.getName().toStdString());
            m_pModel->setProtocolKeywords(newProtocolDlg.getKeywords().toStdString());
            m_pModel->setProtocolDescription(newProtocolDlg.getDescription().toStdString());
        }
        else
            return;
    }

    QSettings IkomiaSettings;

    auto fileName = Utils::File::saveFile(this, tr("Export workflow"), IkomiaSettings.value(_defaultDirProtocolExport).toString(), tr("Pcl Files (*.pcl)"), QStringList("pcl"), ".pcl");
    if(fileName.isEmpty())
        return;

    IkomiaSettings.setValue(_defaultDirProtocolExport, QFileInfo(fileName).path());
    m_pModel->saveProtocol(fileName);
}

void CProtocolModuleWidget::onLoadProtocol()
{
    assert(m_pModel);
    QSettings IkomiaSettings;

    QString path = QFileDialog::getOpenFileName(nullptr, tr("Load workflow"), IkomiaSettings.value(_defaultDirProtocol).toString(), tr("Pcl Files (*.pcl)"), nullptr, CSettingsManager::dialogOptions());
    if(path.isNull() == false)
    {
        m_lastPathProtocol = QFileInfo(path).path(); // store path for next time
        IkomiaSettings.setValue(_defaultDirProtocol, m_lastPathProtocol);
        m_pModel->loadProtocol(path);
        setTitle(m_name + " - " + QString::fromStdString(m_pModel->getProtocolName()));
    }
}

void CProtocolModuleWidget::onCloseProtocol()
{
    assert(m_pModel);

    if(m_pModel->isProtocolRunning())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The workflow is running. Please wait before closing it."));
        msgBox.exec();
        return;
    }

    if(m_pModel->isProtocolModified() == true)
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
            m_pModel->saveProtocol();
    }
    m_pView->clear();
    clearTabWidget();
    setTitle(m_name);
    emit doNotifyProtocolClosed();
}

void CProtocolModuleWidget::onNewProtocol()
{
    assert(m_pModel);

    if(m_pModel->isProtocolExists())
        onCloseProtocol();

    CProtocolNewDlg newProtocolDlg("", m_pModel->getProtocolNames(), this);
    if(newProtocolDlg.exec() == QDialog::Accepted)
    {
        m_pModel->createProtocol(newProtocolDlg.getName().toStdString(),
                                 newProtocolDlg.getKeywords().toStdString(),
                                 newProtocolDlg.getDescription().toStdString());
        setTitle(m_name + " - " + newProtocolDlg.getName());
    }
}

void CProtocolModuleWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_pProcessDocDlg->hide();
}

void CProtocolModuleWidget::onShowProcessInfo()
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

void CProtocolModuleWidget::onIOPropertyValueChanged(QtProperty *pProperty, const QVariant &value)
{
    assert(m_pModel);

    auto it = m_ioProperties.find(pProperty);
    if(it == m_ioProperties.end())
        return;

    auto type = it.value().type;
    if(type == IOPropType::AUTO_SAVE)
    {
        auto outputIndex = (size_t)it.value().data.toInt();
        m_pModel->setCurrentTaskAutoSave(outputIndex, value.toBool());
    }
    else if(type == IOPropType::SAVE_FOLDER)
        m_pModel->setCurrentTaskSaveFolder(value.toString().toStdString());
}

void CProtocolModuleWidget::initLayout()
{
    m_pView = new CProtocolView;

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
    connect(pInfoBtn, &QPushButton::clicked, this, &CProtocolModuleWidget::onShowProcessInfo);

    // Create custom tab and get layout for later modifications
    m_pParamLayout = createTab(QIcon(":/Images/properties_white.png"), tr("Parameters"), pInfoBtn);
    m_pInfoLayout = createTab(QIcon(":/Images/info-white.png"), tr("Info"), nullptr);
    m_pIOLayout = createTab(QIcon(":/Images/io.png"), tr("I/O"), nullptr);

    // Object to create property for QtTreePropertyBrowser
    m_pVariantManager = new QtVariantPropertyManager(this);
    connect(m_pVariantManager, &QtVariantPropertyManager::valueChanged, this, &CProtocolModuleWidget::onIOPropertyValueChanged);

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

void CProtocolModuleWidget::initLeftTab()
{
    QSize btnSize = QSize(50, 24);
    auto pLayout = static_cast<CToolbarBorderLayout*>(layout());

    // Add button to create new protocol
    auto pBtnNew = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/add.png"));
    assert(pBtnNew != nullptr);
    //pBtnNew->setFlat(true);
    pBtnNew->setToolTip(tr("New workflow"));
    connect(pBtnNew, &QPushButton::clicked, this, &CProtocolModuleWidget::onNewProtocol);

    // Add button to load protocol from file
    auto pBtnLoad = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/open.png"));
    assert(pBtnLoad != nullptr);
    //pBtnLoad->setFlat(true);
    pBtnLoad->setToolTip(tr("Load workflow"));
    connect(pBtnLoad, &QPushButton::clicked, this, &CProtocolModuleWidget::onLoadProtocol);

    // Add button to save current protocol
    auto pBtnSave = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/save.png"));
    assert(pBtnSave != nullptr);
    //pBtnSave->setFlat(true);
    pBtnSave->setToolTip(tr("Save workflow"));
    connect(pBtnSave, &QPushButton::clicked, this, &CProtocolModuleWidget::onSaveProtocol);

    // Add button to export current protocol
    auto pBtnExport = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/export.png"));
    assert(pBtnExport != nullptr);
    //pBtnExport->setFlat(true);
    pBtnExport->setToolTip(tr("Export workflow"));
    connect(pBtnExport, &QPushButton::clicked, this, &CProtocolModuleWidget::onExportProtocol);

    // Add button to close current protocol
    auto pBtnClose = pLayout->addButtonToLeft("", btnSize, QIcon(":Images/close-protocol.png"));
    assert(pBtnClose != nullptr);
    //pBtnClose->setFlat(true);
    pBtnClose->setToolTip(tr("Close workflow"));
    connect(pBtnClose, &QPushButton::clicked, this, &CProtocolModuleWidget::onCloseProtocol);
}

void CProtocolModuleWidget::initTopTab()
{
    QSize btnSize = QSize(24, 24);
    auto pLayout = static_cast<CToolbarBorderLayout*>(layout());

    //Add buttons to set input visualization mode
    auto pBtnOriginInput = addButtonToTop(btnSize, QIcon(":Images/original-input.png"), tr("Visualize origin inputs"), true);
    pBtnOriginInput->setChecked(true);
    connect(pBtnOriginInput, &QPushButton::clicked, [=]{ m_pModel->setInputViewMode(ProtocolInputViewMode::ORIGIN); });

    auto pBtnCurrentInput = addButtonToTop(btnSize, QIcon(":Images/current-input.png"), tr("Visualize current inputs"), true);
    connect(pBtnCurrentInput, &QPushButton::clicked, [&]{ m_pModel->setInputViewMode(ProtocolInputViewMode::CURRENT); });

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
    connect(pBtnRun, &QPushButton::clicked, [&]{ m_pModel->runProtocol(); });

    //Add button to run protocol from current task
    auto pBtnRunFrom = addButtonToTop(btnSize, QIcon(":/Images/play-from.png"), tr("Run workflow from current task"));
    connect(pBtnRunFrom, &QPushButton::clicked, [&]{ m_pModel->runProtocolFromActiveTask(); });

    //Add button to run protocol from start to current task
    auto pBtnRunTo = addButtonToTop(btnSize, QIcon(":/Images/play-to.png"), tr("Run workflow from start to current task"));
    connect(pBtnRunTo, &QPushButton::clicked, [&]{ m_pModel->runProtocolToActiveTask(); });

    //Add button to stop protocol
    auto pBtnStop = addButtonToTop(btnSize, QIcon(":/Images/stop.png"), tr("Stop workflow"));
    connect(pBtnStop, &QPushButton::clicked, [&]{ m_pModel->stopProtocol(); });

    //Add zoom buttons
    pLayout->addSeparatorToTop();
    auto pBtnZoomOriginal = addButtonToTop(btnSize, QIcon(":/Images/zoom-original.png"), tr("Original size"));
    connect(pBtnZoomOriginal, &QPushButton::clicked, [&]{ m_pView->zoomInit(); });

    auto pBtnZoomIn = addButtonToTop(btnSize, QIcon(":/Images/zoom-in.png"), tr("Zoom in"));
    connect(pBtnZoomIn, &QPushButton::clicked, [&]{ m_pView->zoomIn(); });

    auto pBtnZoomOut = addButtonToTop(btnSize, QIcon(":/Images/zoom-out.png"), tr("Zoom out"));
    connect(pBtnZoomOut, &QPushButton::clicked, [&]{ m_pView->zoomOut(); });
}

void CProtocolModuleWidget::initConnections()
{
    //Model -> module
    connect(m_pModel, &CProtocolManager::doAddCandidateTask, this, &CProtocolModuleWidget::onAddCandidateTask);
    connect(m_pModel, &CProtocolManager::doAddTask, this, &CProtocolModuleWidget::onAddTask);
    connect(m_pModel, &CProtocolManager::doProtocolCreated, this, &CProtocolModuleWidget::onProtocolCreated);
    connect(m_pModel, &CProtocolManager::doUpdateTaskInfo, this, &CProtocolModuleWidget::onUpdateTaskInfo);
    connect(m_pModel, &CProtocolManager::doUpdateTaskIOInfo, [&](const ProtocolTaskPtr& taskPtr)
    {
        fillIOProperties(taskPtr);
    });
    connect(m_pModel, &CProtocolManager::doUpdateTaskStateInfo, this, &CProtocolModuleWidget::onUpdateTaskStateInfo);
    connect(m_pModel, &CProtocolManager::doCloseProtocol, this, &CProtocolModuleWidget::onCloseProtocol);

    //Model -> view
    connect(m_pModel, &CProtocolManager::doInputChanged, m_pView, &CProtocolView::onInputChanged);
    connect(m_pModel, &CProtocolManager::doInputAssigned, m_pView, &CProtocolView::onInputAssigned);
    connect(m_pModel, &CProtocolManager::doInputRemoved, m_pView, &CProtocolView::onInputRemoved);
    connect(m_pModel, &CProtocolManager::doInputsCleared, m_pView, &CProtocolView::onInputsCleared);
    connect(m_pModel, &CProtocolManager::doAddConnection, m_pView, &CProtocolView::onAddConnection);
    connect(m_pModel, &CProtocolManager::doUpdateCandidateTask, m_pView, &CProtocolView::onUpdateCandidateTask);
    connect(m_pModel, &CProtocolManager::doUpdateTaskItemView, m_pView, &CProtocolView::onUpdateTaskItem);
    connect(m_pModel, &CProtocolManager::doSetActiveTask, m_pView, &CProtocolView::onSetActiveTask);
    connect(m_pModel, &CProtocolManager::doSetTaskState, m_pView, &CProtocolView::onSetTaskState);
    connect(m_pModel, &CProtocolManager::doDeleteConnection, m_pView, &CProtocolView::onDeleteConnectionFromModel);
    connect(m_pModel, &CProtocolManager::doSetIOInfo, m_pView, &CProtocolView::onSetIOInfo);

    //Model -> scene
    connect(m_pModel, &CProtocolManager::doSetGraphicsProxyModel, m_pView->getScene(), &CProtocolScene::onSetGraphicsProxyModel);
    connect(m_pModel, &CProtocolManager::doSetProjectDataProxyModel, m_pView->getScene(), &CProtocolScene::onSetProjectDataProxyModel);

    //Module -> model
    connect(this, &CProtocolModuleWidget::doRunFromActiveTask, m_pModel, &CProtocolManager::onRunFromActiveTask);
    connect(this, &CProtocolModuleWidget::doSetGraphicsTool, m_pModel, &CProtocolManager::onSetGraphicsTool);
    connect(this, &CProtocolModuleWidget::doSetGraphicsCategory, m_pModel, &CProtocolManager::onSetGraphicsCategory);
    connect(this, &CProtocolModuleWidget::doSendProcessAction, m_pModel, &CProtocolManager::onSendProcessAction);

    //Scene -> model
    auto pScene = static_cast<CProtocolScene*>(m_pView->scene());
    connect(pScene, &CProtocolScene::doSelectionChange, m_pModel, &CProtocolManager::onActiveTaskChanged);
    connect(pScene, &CProtocolScene::doItemConnection, m_pModel, &CProtocolManager::onConnectTask);
    connect(pScene, &CProtocolScene::doSetTaskActionFlag, m_pModel, &CProtocolManager::onSetTaskActionFlag);
    connect(pScene, &CProtocolScene::doSetGraphicsLayerInput, m_pModel, &CProtocolManager::onSetGraphicsLayerInput);
    connect(pScene, &CProtocolScene::doSetFolderPathInput, m_pModel, &CProtocolManager::onSetFolderPathInput);
    connect(pScene, &CProtocolScene::doSetInput, m_pModel, &CProtocolManager::onSetInput);
    connect(pScene, &CProtocolScene::doQueryGraphicsProxyModel, m_pModel, &CProtocolManager::onQueryGraphicsProxyModel);
    connect(pScene, &CProtocolScene::doQueryProjectDataProxyModel, m_pModel, &CProtocolManager::onQueryProjectDataProxyModel);
    connect(pScene, &CProtocolScene::doQueryIOInfo, m_pModel, &CProtocolManager::onQueryIOInfo);
    connect(pScene, &CProtocolScene::doAddInput, m_pModel, &CProtocolManager::onAddInput);
    connect(pScene, &CProtocolScene::doDeleteInput, m_pModel, &CProtocolManager::onDeleteInput);

    //Scene -> main view
    connect(pScene, &CProtocolScene::doShowProcessPopup, [&]{ emit doShowProcessPopup(); });

    //Doc window -> model
    connect(m_pProcessDocDlg->getDocWidget(), &CProcessDocWidget::doSave, m_pModel, &CProtocolManager::onUpdateProcessInfo);
}

QVBoxLayout* CProtocolModuleWidget::createTab(QIcon icon, QString title, QWidget* pBtn)
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

QToolButton *CProtocolModuleWidget::addButtonToTop(const QSize& size, const QIcon &icon, const QString& tooltip, bool bCheckable)
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

void CProtocolModuleWidget::setTitle(const QString &title)
{
    setWindowTitle(title);
    emit doUpdateTitle();
}

void CProtocolModuleWidget::remove(QLayout* pLayout)
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

void CProtocolModuleWidget::clearParamLayout()
{
    m_widgetPtr.reset();

    while(!m_pParamLayout->isEmpty())
    {
        QWidget* pWidget = m_pParamLayout->takeAt(0)->widget();
        m_pParamLayout->removeWidget(pWidget);
        pWidget->hide();
        pWidget->deleteLater();
    }
}

void CProtocolModuleWidget::clearTabWidget()
{
    // Parameters
    clearParamLayout();
    m_pParamLayout->addWidget(new QWidget);
    // Execution time information
    m_pInfoPropertyList->clear();
    // I/O information
    m_pIOPropertyList->clear();
}

void CProtocolModuleWidget::fillIOProperties(const ProtocolTaskPtr &taskPtr)
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
        auto inputName = QString("#%1 - ").arg(i+1) + Utils::Protocol::getIODataName(inputPtr->getDataType());
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
        auto outputName = QString("#%1 - ").arg(i+1) + Utils::Protocol::getIODataName(outputPtr->getDataType());

        QtProperty* pOutputGroup = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), outputName);
        pOutputsGroup->addSubProperty(pOutputGroup);

        QtVariantProperty* pDescProp = m_pVariantManager->addProperty(QVariant::String, tr("Description"));
        pDescProp->setValue(QString::fromStdString(outputPtr->getDescription()));
        pDescProp->setEnabled(false);
        pOutputGroup->addSubProperty(pDescProp);

        QtVariantProperty* pAutoSaveProp = m_pVariantManager->addProperty(QVariant::Bool, tr("Auto save"));
        pAutoSaveProp->setValue(outputPtr->isAutoSave());
        PropAttribute propAttr;
        propAttr.type = IOPropType::AUTO_SAVE;
        propAttr.data = (int)i;
        m_ioProperties.insert(pAutoSaveProp, propAttr);
        pOutputGroup->addSubProperty(pAutoSaveProp);

        QtVariantProperty* pSaveFormatProp = m_pVariantManager->addProperty(QVariant::String, tr("Save format"));
        pSaveFormatProp->setValue(Utils::Data::getExportFormatName(outputPtr->getSaveFormat()));
        pSaveFormatProp->setEnabled(false);
        pOutputGroup->addSubProperty(pSaveFormatProp);
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

void CProtocolModuleWidget::setProcessInfoMap(const VectorPairString& infoMap, const VectorPairString& customInfoMap)
{
    // Clear process properties for new values
    m_pInfoPropertyList->clear();
    // Add property groups and fill them
    setGroupProperties(m_pInfoPropertyList, tr("Process benchmark"), infoMap);
    setGroupProperties(m_pInfoPropertyList, tr("Custom process information"), customInfoMap);
    // Apply style
    applyPropertyListStyle(m_pInfoPropertyList);
}

void CProtocolModuleWidget::setGroupProperties(QtTreePropertyBrowser* pPropList, const QString& title, const VectorPairString& properties)
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

void CProtocolModuleWidget::fillProperty(QtProperty* pItem, const VectorPairString& properties)
{
    for(auto&& it : properties)
    {
        QtVariantProperty *item = m_pVariantManager->addProperty(QVariant::String, QString::fromStdString(it.first));
        item->setValue(QString::fromStdString(it.second));
        pItem->addSubProperty(item);
    }
}

void CProtocolModuleWidget::applyPropertyListStyle(QtTreePropertyBrowser *pPropList)
{
    // Set browser line color
    auto list = pPropList->topLevelItems();
    QPalette p = qApp->palette();

    for(auto&& it : list)
        pPropList->setBackgroundColor(it, p.base().color());
}

void CProtocolModuleWidget::adjustProcessDocDlgPos()
{
    const QRect screen = QApplication::desktop()->availableGeometry(this);
    const QRect rect = m_pProcessDocDlg->geometry();
    int x = screen.right() - rect.width() + m_pProcessDocDlg->getBorderSize();
    int y = screen.bottom() - rect.height();
    m_pProcessDocDlg->move(x, y);
}

#include "moc_CProtocolModuleWidget.cpp"
