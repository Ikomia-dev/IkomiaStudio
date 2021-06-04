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
#include "Model/Store/CStoreQueryModel.h"
#include "View/Process/CProcessDocWidget.h"

CStoreDlg::CStoreDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Ikomia Store"), parent, DEFAULT | MAXIMIZE_BUTTON, f)
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
    emit doGetServerModel();
    emit doGetLocalModel();
    m_pDocWidget->setCurrentUser(user);
}

void CStoreDlg::onSetServerPluginModel(CStoreQueryModel *pModel)
{
    m_pServerPluginsView->setModel(pModel);

    if(pModel == nullptr)
        m_pLabelMsgServer->setText(tr("You have to be registered to display online plugins"));
    else
    {
        auto updateLabelMsg = [this, pModel]
        {
            if(pModel->rowCount() == 0)
                m_pLabelMsgServer->setText(tr("No plugin found"));
            else
                m_pLabelMsgServer->setText(tr("%1 plugin(s) available").arg(pModel->rowCount()));
        };

        connect(pModel, &CStoreQueryModel::modelReset, updateLabelMsg);
        updateLabelMsg();
    }
}

void CStoreDlg::onSetLocalPluginModel(CStoreQueryModel *pModel)
{
    m_pLocalPluginsView->setModel(pModel);

    if(pModel == nullptr)
        m_pLabelMsgLocal->setText(tr("No plugins loaded"));
    else
    {
        auto updateLabelMsg = [this, pModel]
        {
            if(pModel->rowCount() == 0)
                m_pLabelMsgLocal->setText(tr("No plugin found"));
            else
                m_pLabelMsgLocal->setText(tr("%1 plugin(s) available").arg(pModel->rowCount()));
        };

        connect(pModel, &CStoreQueryModel::modelReset, updateLabelMsg);
        updateLabelMsg();
    }
}

void CStoreDlg::onShowLocalPluginInfo(const QModelIndex &index)
{
    showProcessInfo(index);
}

void CStoreDlg::onShowServerPluginInfo(const QModelIndex &index)
{
    showProcessInfo(index);
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
    connect(m_pEditServerSearch, &QLineEdit::textChanged, [&](const QString& text){ emit doServerSearchChanged(text); });
    connect(m_pEditLocalSearch, &QLineEdit::textChanged, [&](const QString& text){ emit doLocalSearchChanged(text); });

    connect(m_pBtnServerRefresh, &QPushButton::clicked, [&]{ emit doGetServerModel(); });

    connect(m_pBtnServerPlugins, &QPushButton::clicked, [&]{ m_pPluginStackWidget->setCurrentIndex(0); });
    connect(m_pBtnLocalPlugins, &QPushButton::clicked, [&]{ m_pPluginStackWidget->setCurrentIndex(1); });

    connect(m_pLocalPluginsView, &CStorePluginListView::doPublishPlugin, [&](const QModelIndex& index){ emit doPublishPlugin(index); });
    connect(m_pLocalPluginsView, &CStorePluginListView::doShowPluginInfo, this, &CStoreDlg::onShowLocalPluginInfo);

    connect(m_pServerPluginsView, &CStorePluginListView::doInstallPlugin, [&](const QModelIndex& index){ emit doInstallPlugin(index); });
    connect(m_pServerPluginsView, &CStorePluginListView::doShowPluginInfo, this, &CStoreDlg::onShowServerPluginInfo);

    connect(m_pDocWidget, &CProcessDocWidget::doBack, [&]{ m_pRightStackWidget->setCurrentIndex(0); });
    connect(m_pDocWidget, &CProcessDocWidget::doSave, [&](bool bFullEdit, const CTaskInfo& info)
    {
        emit doUpdatePluginInfo(bFullEdit, info);
    });
}

QWidget *CStoreDlg::createLeftWidget()
{
    m_pBtnServerPlugins = new QPushButton(tr("Online store"));
    m_pBtnLocalPlugins = new QPushButton(tr("Installed plugins"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(m_pBtnServerPlugins);
    pLayout->addWidget(m_pBtnLocalPlugins);
    pLayout->addStretch(1);

    auto pWidget = new QWidget;
    pWidget->setLayout(pLayout);
    return pWidget;
}

QWidget *CStoreDlg::createRightWidget()
{
    //----------//
    //- Server -//
    //----------//
    //Search bar + refresh button
    m_pEditServerSearch = new QLineEdit;
    m_pEditServerSearch->setPlaceholderText(tr("<Search by keywords>"));
    m_pBtnServerRefresh = new QPushButton(QIcon(":/Images/update.png"), "");
    m_pBtnServerRefresh->setToolTip(tr("Refresh"));
    auto pTopBarServerLayout = new QHBoxLayout;
    pTopBarServerLayout->addWidget(m_pEditServerSearch);
    pTopBarServerLayout->addWidget(m_pBtnServerRefresh);

    //Plugins list view
    m_pServerPluginsView = new CStorePluginListView(CStorePluginListViewDelegate::SERVER);

    //Message label
    m_pLabelMsgServer = createMessageLabel(tr("You have to be registered to display online plugins"));

    auto pServerVLayout = new QVBoxLayout;
    pServerVLayout->addLayout(pTopBarServerLayout);
    pServerVLayout->addWidget(m_pServerPluginsView);
    pServerVLayout->addWidget(m_pLabelMsgServer);
    auto pServerWidget = new QWidget;
    pServerWidget->setLayout(pServerVLayout);

    //---------//
    //- Local -//
    //---------//
    //Search bar
    m_pEditLocalSearch = new QLineEdit;
    m_pEditLocalSearch->setPlaceholderText(tr("<Search by keywords>"));

    //Plugins list view
    m_pLocalPluginsView = new CStorePluginListView(CStorePluginListViewDelegate::LOCAL);

    //Message label
    m_pLabelMsgLocal = createMessageLabel(tr("No plugins loaded"));

    auto pLocalVLayout = new QVBoxLayout;
    pLocalVLayout->addWidget(m_pEditLocalSearch);
    pLocalVLayout->addWidget(m_pLocalPluginsView);
    pLocalVLayout->addWidget(m_pLabelMsgLocal);
    auto pLocalWidget = new QWidget;
    pLocalWidget->setLayout(pLocalVLayout);

    //---------------------------------//
    //- Stacked widget for list views -//
    //---------------------------------//
    m_pPluginStackWidget = new QStackedWidget;
    m_pPluginStackWidget->addWidget(pServerWidget);
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
    info.m_ikomiaVersion = record.value("ikomiaVersion").toString().toStdString();
    info.m_createdDate = record.value("createdDate").toString().toStdString();
    info.m_modifiedDate = record.value(" modifiedDate").toString().toStdString();
    info.m_license = record.value("license").toString().toStdString();
    info.m_repo = record.value("repository").toString().toStdString();
    info.m_year = record.value("year").toInt();
    info.m_language = record.value("language").toInt();
    info.m_bInternal = record.value("isInternal").toInt();
    info.m_userId = record.value("userId").toInt();
    info.m_os = record.value("os").toInt();

    m_pDocWidget->setProcessInfo(info);
    m_pRightStackWidget->setCurrentIndex(1);
}

void CStoreDlg::showEvent(QShowEvent *event)
{
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
