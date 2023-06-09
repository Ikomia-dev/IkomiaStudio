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

#include "CStoreDlg.h"
#include <QtWidgets>
#include "CStorePluginListView.h"
#include "CStorePluginListViewDelegate.h"
#include "View/Process/CProcessDocWidget.h"
#include "CWorkspaceChoiceDlg.h"
#include "CPublicationFormDlg.h"

CStoreDlg::CStoreDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Ikomia HUB"), parent, DEFAULT | MAXIMIZE_BUTTON, f)
{
    //Important note: style CDialog::EFFECT_ENABLES must not be used.
    //CStoreDlg includes a QWebEngineView based on QOpenGLWidget,
    //and graphics effects are not supported for OpenGL-based widgets.
    initLayout();
    initConnections();

    //Resize window
    auto screens = QGuiApplication::screens();
    QRect screenRect = screens[0]->availableGeometry();
    resize(QSize(screenRect.width()*0.7, screenRect.height()*0.7));
}

void CStoreDlg::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
    m_pDocWidget->setCurrentUser(user);

    if (isVisible())
    {
        emit doGetHubModel();
        emit doGetWorkspaceModel();
        emit doGetLocalModel();
    }
}

void CStoreDlg::onSetPluginModel(CPluginModel *pModel)
{
    QLabel* pLabelMsg;
    auto pQueryModel = pModel->getModel();

    switch(pModel->getType())
    {
        case CPluginModel::Type::HUB:
            m_pHubView->setModel(pQueryModel);
            pLabelMsg = m_pLabelMsgHub;
            break;
        case CPluginModel::Type::WORKSPACE:
            m_pWorkspaceView->setModel(pQueryModel);
            pLabelMsg = m_pLabelMsgWorkspace;
            break;
        case CPluginModel::Type::LOCAL:
            m_pLocalView->setModel(pQueryModel);
            pLabelMsg = m_pLabelMsgLocal;
            break;
    }

    if(pQueryModel == nullptr)
        pLabelMsg->setText(tr("You have to be registered to display online algorithms"));
    else
    {
        auto updateLabelMsg = [this, pQueryModel, pLabelMsg]
        {
            if(pQueryModel->rowCount() == 0)
                pLabelMsg->setText(tr("No algorithm found"));
            else
                pLabelMsg->setText(tr("%1 algorithms(s) available").arg(pQueryModel->rowCount()));
        };

        connect(pQueryModel, &CStoreQueryModel::modelReset, updateLabelMsg);
        updateLabelMsg();
    }
}

void CStoreDlg::onShowPluginInfo(const QModelIndex &index)
{
    showProcessInfo(index);
}

void CStoreDlg::onPublishPluginToWorkspace(const QModelIndex& index)
{
    CWorkspaceChoiceDlg workspaceDlg(m_currentUser, this);
    if(workspaceDlg.exec() == QDialog::Accepted)
    {
        QString workspace = workspaceDlg.getWorkspaceName();
        emit doPublishWorkspace(index, workspace);
    }
}

void CStoreDlg::onSetNextPublishInfo(const QModelIndex& index, const QJsonObject& publishInfo)
{
    auto pModel = dynamic_cast<CStoreQueryModel*>(m_pWorkspaceView->model());
    assert(pModel);
    QSqlRecord pluginInfo = pModel->record(index.row());
    CPublicationFormDlg publishFormDlg(pluginInfo, publishInfo, this);

    if(publishFormDlg.exec() == QDialog::Accepted)
    {
        emit doPublishHub(index, publishFormDlg.getPublishInfo());
    }
}

void CStoreDlg::initLayout()
{
    auto pLeftWidget = createLeftWidget();
    auto pRightWidget = createRightWidget();

    QSplitter* pSplitter = new QSplitter;
    pSplitter->addWidget(pLeftWidget);
    pSplitter->addWidget(pRightWidget);
    pSplitter->setStretchFactor(0, 1);
    pSplitter->setStretchFactor(1, 2);

    auto pLayout = getContentLayout();
    pLayout->addWidget(pSplitter);
}

void CStoreDlg::initConnections()
{
    connect(m_pEditHubSearch, &QLineEdit::textChanged, [&](const QString& text){ emit doHubSearchChanged(text); });
    connect(m_pEditWorkspaceSearch, &QLineEdit::textChanged, [&](const QString& text){ emit doWorkspaceSearchChanged(text); });
    connect(m_pEditLocalSearch, &QLineEdit::textChanged, [&](const QString& text){ emit doLocalSearchChanged(text); });

    connect(m_pBtnHubRefresh, &QPushButton::clicked, [&]{ emit doGetHubModel(); });
    connect(m_pBtnWorkspaceRefresh, &QPushButton::clicked, [&]{ emit doGetWorkspaceModel(); });

    connect(m_pBtnHub, &QPushButton::clicked, [&]{ m_pPluginStackWidget->setCurrentIndex(0); });
    connect(m_pBtnWorkspace, &QPushButton::clicked, [&]{ m_pPluginStackWidget->setCurrentIndex(1); });
    connect(m_pBtnLocalPlugins, &QPushButton::clicked, [&]{ m_pPluginStackWidget->setCurrentIndex(2); });

    connect(m_pHubView, &CStorePluginListView::doShowPluginInfo, this, &CStoreDlg::onShowPluginInfo);
    connect(m_pWorkspaceView, &CStorePluginListView::doShowPluginInfo, this, &CStoreDlg::onShowPluginInfo);
    connect(m_pLocalView, &CStorePluginListView::doShowPluginInfo, this, &CStoreDlg::onShowPluginInfo);

    connect(m_pHubView, &CStorePluginListView::doInstallPlugin, [&](const QModelIndex& index){ emit doInstallPlugin(CPluginModel::Type::HUB, index); });
    connect(m_pWorkspaceView, &CStorePluginListView::doInstallPlugin, [&](const QModelIndex& index){ emit doInstallPlugin(CPluginModel::Type::WORKSPACE, index); });

    connect(m_pLocalView, &CStorePluginListView::doPublishPlugin, this, &CStoreDlg::onPublishPluginToWorkspace);
    connect(m_pWorkspaceView, &CStorePluginListView::doPublishPlugin, [&](const QModelIndex& index){ emit doGetNextPublishInfo(index); });

    connect(m_pDocWidget, &CProcessDocWidget::doBack, [&]{ m_pRightStackWidget->setCurrentIndex(0); });
    connect(m_pDocWidget, &CProcessDocWidget::doSave, [&](bool bFullEdit, const CTaskInfo& info)
    {
        emit doUpdatePluginInfo(bFullEdit, info);
    });
}

QWidget *CStoreDlg::createLeftWidget()
{
    m_pBtnHub = new QPushButton(tr("Ikomia HUB"));
    m_pBtnWorkspace = new QPushButton(tr("Private workspaces"));
    m_pBtnLocalPlugins = new QPushButton(tr("Installed algorithms"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(m_pBtnHub);
    pLayout->addWidget(m_pBtnWorkspace);
    pLayout->addWidget(m_pBtnLocalPlugins);
    pLayout->addStretch(1);

    auto pWidget = new QWidget;
    pWidget->setLayout(pLayout);
    return pWidget;
}

QWidget* CStoreDlg::createPluginsView(CPluginModel::Type type)
{
    QLineEdit** ppSearchBar = nullptr;
    QPushButton** ppRefreshBtn = nullptr;
    QLabel** ppLabel = nullptr;
    CStorePluginListView** ppView = nullptr;
    CStorePluginListViewDelegate::PluginSource pluginSource;

    switch(type)
    {
        case CPluginModel::Type::HUB:
            pluginSource = CStorePluginListViewDelegate::HUB;
            ppSearchBar = &m_pEditHubSearch;
            ppRefreshBtn = &m_pBtnHubRefresh;
            ppLabel = &m_pLabelMsgHub;
            ppView = &m_pHubView;
            break;
        case CPluginModel::Type::WORKSPACE:
            pluginSource = CStorePluginListViewDelegate::WORKSPACE;
            ppSearchBar = &m_pEditWorkspaceSearch;
            ppRefreshBtn = &m_pBtnWorkspaceRefresh;
            ppLabel = &m_pLabelMsgWorkspace;
            ppView = &m_pWorkspaceView;
            break;
        case CPluginModel::Type::LOCAL:
            pluginSource = CStorePluginListViewDelegate::LOCAL;
            ppSearchBar = &m_pEditLocalSearch;
            ppLabel = &m_pLabelMsgLocal;
            ppView = &m_pLocalView;
            break;
    }

    auto pTopBarLayout = new QHBoxLayout;

    // Search bar
    auto pSearchBar = new QLineEdit;
    (*ppSearchBar) = pSearchBar;
    pSearchBar->setPlaceholderText(tr("<Search by keywords>"));
    pTopBarLayout->addWidget(pSearchBar);

    // Refresh button
    if (type != CPluginModel::Type::LOCAL)
    {
        auto pRefreshBtn = new QPushButton(QIcon(":/Images/update.png"), "");
        (*ppRefreshBtn) = pRefreshBtn;
        pRefreshBtn->setToolTip(tr("Refresh"));
        pTopBarLayout->addWidget(pRefreshBtn);
    }

    //Plugins list view
    auto pView = new CStorePluginListView(pluginSource);
    (*ppView) = pView;

    //Message label
    auto pLabel = createMessageLabel(tr("You have to be registered to display online plugins"));
    (*ppLabel) = pLabel;

    auto pVLayout = new QVBoxLayout;
    pVLayout->addLayout(pTopBarLayout);
    pVLayout->addWidget(pView);
    pVLayout->addWidget(pLabel);
    auto pMainWidget = new QWidget;
    pMainWidget->setLayout(pVLayout);
    return pMainWidget;
}

QWidget *CStoreDlg::createRightWidget()
{
    //----------------------//
    //- Plugins list views -//
    //----------------------//
    auto pHubWidget = createPluginsView(CPluginModel::Type::HUB);
    auto pWorkspaceWidget = createPluginsView(CPluginModel::Type::WORKSPACE);
    auto pLocalWidget = createPluginsView(CPluginModel::Type::LOCAL);

    //---------------------------------//
    //- Stacked widget for list views -//
    //---------------------------------//
    m_pPluginStackWidget = new QStackedWidget;
    m_pPluginStackWidget->addWidget(pHubWidget);
    m_pPluginStackWidget->addWidget(pWorkspaceWidget);
    m_pPluginStackWidget->addWidget(pLocalWidget);
    m_pPluginStackWidget->setCurrentIndex(0);

    //------------------------//
    //- Documentation widget -//
    //------------------------//
    m_pDocWidget = new CProcessDocWidget(CProcessDocWidget::EDIT|CProcessDocWidget::BACK);

    //-----------------------//
    //- Global stack widget -//
    //-----------------------//
    m_pRightStackWidget = new QStackedWidget;
    m_pRightStackWidget->addWidget(m_pPluginStackWidget);
    m_pRightStackWidget->addWidget(m_pDocWidget);
    m_pRightStackWidget->setCurrentIndex(0);

    return m_pRightStackWidget;
}

QLabel *CStoreDlg::createMessageLabel(const QString& msg)
{
    auto pLabelMsg = new QLabel;
    pLabelMsg->setText(msg);
    pLabelMsg->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; }");
    pLabelMsg->setAlignment(Qt::AlignCenter);
    return pLabelMsg;
}

void CStoreDlg::showProcessInfo(const QModelIndex &index)
{
    auto pModel = static_cast<const CStoreQueryModel*>(index.model());
    auto record = pModel->record(index.row());

    CTaskInfo info;
    info.m_id = record.value("id").toInt();
    info.m_name = record.value("name").toString().toStdString();
    info.m_description = record.value("description").toString().toStdString();
    info.m_docLink = record.value("docLink").toString().toStdString();
    info.m_iconPath = record.value("iconPath").toString().toStdString();
    info.m_keywords = record.value("keywords").toString().toStdString();
    info.m_authors = record.value("authors").toString().toStdString();
    info.m_article = record.value("article").toString().toStdString();
    info.m_journal = record.value("journal").toString().toStdString();
    info.m_version = record.value("version").toString().toStdString();
    info.m_minIkomiaVersion = record.value("ikomiaVersion").toString().toStdString();
    info.m_createdDate = record.value("createdDate").toString().toStdString();
    info.m_modifiedDate = record.value(" modifiedDate").toString().toStdString();
    info.m_license = record.value("license").toString().toStdString();
    info.m_repo = record.value("repository").toString().toStdString();
    info.m_year = record.value("year").toInt();
    info.m_language = record.value("language").toInt() == 0 ? ApiLanguage::CPP : ApiLanguage::PYTHON;
    info.m_bInternal = record.value("isInternal").toInt();
    info.m_userId = record.value("userId").toInt();
    info.m_os = static_cast<OSType>(record.value("os").toInt());

    m_pDocWidget->setProcessInfo(info);
    m_pRightStackWidget->setCurrentIndex(1);
}

void CStoreDlg::showEvent(QShowEvent *event)
{
    emit doGetHubModel();
    emit doGetWorkspaceModel();
    emit doGetLocalModel();
    QDialog::showEvent(event);
}

void CStoreDlg::closeEvent(QCloseEvent* event)
{
    emit doClose();
    QDialog::closeEvent(event);
}

void CStoreDlg::hideEvent(QHideEvent* event)
{
    emit doClose();
    QDialog::hideEvent(event);
}

#include "moc_CStoreDlg.cpp"
