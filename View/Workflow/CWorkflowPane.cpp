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

#include "CWorkflowPane.h"
#include <QListView>
#include "CWorkflowInfoDlg.h"

CWorkflowPane::CWorkflowPane(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initContextMenu();
    initConnections();
    m_pListView->installEventFilter(this);
}

void CWorkflowPane::onSetModel(QStringListModel *pModel)
{
    m_pListView->setModel(pModel);
}

void CWorkflowPane::onSetWorkflowInfo(const QString &text, const QString& keywords)
{
    if(m_pInfoDlg == nullptr)
    {
        m_pInfoDlg = new CWorkflowInfoDlg(this);
        connect(m_pInfoDlg, &CWorkflowInfoDlg::doLoadWorkflow, [&]{ onLoadSelectedWorkflow(); m_pInfoDlg->hide(); });
    }

    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() == 0)
        return;

    // Manage if text is empty or not
    // Update popup dlg and description widget
    QString description;
    if(text.isEmpty())
        description = tr("No description available");
    else
        description = text;

    auto protocolName = indexes[0].data(Qt::DisplayRole).toString();
    m_pInfoDlg->setName(protocolName);
    m_pInfoDlg->setDescription(description);
    m_pInfoDlg->show();

    m_pWorkflowDesc->setMarkdown(description);
    m_pWorkflowKeywords->setText(keywords);

    QRect rcItem = m_pListView->visualRect(indexes[0]);
    QPoint position((rcItem.left()+rcItem.right()) / 2, (rcItem.top()+rcItem.bottom()) / 2);
    updateWorkflowInfoPosition(position);
}

void CWorkflowPane::onSearchWorkflow(const QString &text)
{
    emit doSearchWorkflow(text);
}

void CWorkflowPane::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = m_pListView->indexAt(pos);
    if(!index.isValid())
        return;

    m_contextMenu.displayMenu(0, m_pListView->mapToGlobal(pos), index);
}

void CWorkflowPane::onShowWorkflowInfo(const QModelIndex &index)
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() == 0)
        return;

    auto workflowName = indexes[0].data(Qt::DisplayRole).toString();
    if(m_pInfoDlg && m_pInfoDlg->isHidden() == false && workflowName == m_pInfoDlg->getName())
        m_pInfoDlg->hide();
    else
        emit doGetWorkflowInfo(index);
}

void CWorkflowPane::onLoadSelectedWorkflow()
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() > 0)
    {
        emit doLoadWorkflow(indexes[0]);
        emit doOpenWorkflowView();
    }
}

void CWorkflowPane::onDeleteSelectedWorkflow()
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() > 0)
        emit doDeleteWorkflow(indexes[0]);
}

bool CWorkflowPane::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
        switch(pKeyEvent->key())
        {
            case Qt::Key_Delete:
                deleteCurrentWorkflow();
                break;
        }
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void CWorkflowPane::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    if(m_pInfoDlg)
        m_pInfoDlg->hide();
}

void CWorkflowPane::initLayout()
{
    //Toolbar
    QAction* pLoadAct = new QAction(QIcon(":/Images/open.png"), tr("Load workflow"), this);
    pLoadAct->setStatusTip(tr("Load selected workflow"));
    connect(pLoadAct, &QAction::triggered, this, &CWorkflowPane::onLoadSelectedWorkflow);

    QAction* pDeleteAct = new QAction(QIcon(":/Images/delete.png"), tr("Delete workflow"), this);
    pDeleteAct->setStatusTip(tr("Delete selected workflow"));
    connect(pDeleteAct, &QAction::triggered, this, &CWorkflowPane::onDeleteSelectedWorkflow);

    QToolBar* pToolbar = new QToolBar;
    pToolbar->addAction(pLoadAct);
    pToolbar->widgetForAction(pLoadAct)->setObjectName("CToolbarBtn");
    pToolbar->addAction(pDeleteAct);
    pToolbar->widgetForAction(pDeleteAct)->setObjectName("CToolbarBtn");

    //Search Box
    m_pSearchBox = new QLineEdit;
    m_pSearchBox->setPlaceholderText(tr("<Search by keywords>"));

    m_pListView = new QListView;
    m_pListView->setSelectionMode(QListView::SingleSelection);
    m_pListView->setEditTriggers(QListView::EditKeyPressed);
    m_pListView->setContextMenuPolicy(Qt::CustomContextMenu);

    QWidget* pContainer = new QWidget;
    QVBoxLayout* pContainerLayout = new QVBoxLayout;
    pContainerLayout->addWidget(pToolbar);
    pContainerLayout->addWidget(m_pSearchBox);
    pContainerLayout->addWidget(m_pListView);
    pContainerLayout->setStretchFactor(m_pListView, 2);
    pContainer->setLayout(pContainerLayout);

    // Workflow description
    auto pWorkflowDescription =  new QLabel(tr("Description"));
    pWorkflowDescription->setAlignment(Qt::AlignLeft);
    pContainerLayout->addWidget(pWorkflowDescription);

    m_pWorkflowDesc = new QTextEdit;
    m_pWorkflowDesc->setObjectName("CDataDisplay");
    m_pWorkflowDesc->setReadOnly(true);
    pContainerLayout->addWidget(m_pWorkflowDesc);
    pContainerLayout->setStretchFactor(m_pWorkflowDesc, 3);

    // Workflow keywords
    auto pKeywordsLabel =  new QLabel(tr("Keywords"));
    pKeywordsLabel->setAlignment(Qt::AlignLeft);
    pContainerLayout->addWidget(pKeywordsLabel);

    m_pWorkflowKeywords = new QLineEdit;
    m_pWorkflowKeywords->setObjectName("CDataDisplay");
    m_pWorkflowKeywords->setReadOnly(true);
    pContainerLayout->addWidget(m_pWorkflowKeywords);

    QToolBox* pToolBox = new QToolBox;
    pToolBox->addItem(pContainer, tr("Workflow"));
    auto pLayout = new QVBoxLayout;
    pLayout->addWidget(pToolBox);

    setLayout(pLayout);
}

void CWorkflowPane::initContextMenu()
{
    m_contextMenu.addAction(0, tr("Load workflow"),
                            [this](QModelIndex& index){ emit doLoadWorkflow(index); emit doOpenWorkflowView(); },
                            QIcon(":/Images/open.png") );
    m_contextMenu.addAction(0, tr("Delete workflow"),
                            [this](QModelIndex& index){ emit doDeleteWorkflow(index); },
                            QIcon(":/Images/delete.png") );
}

void CWorkflowPane::initConnections()
{
    connect(m_pListView, &QListView::clicked, this, &CWorkflowPane::onShowWorkflowInfo);
    connect(m_pListView, &QListView::doubleClicked, [&](const QModelIndex& index){ emit doLoadWorkflow(index); emit doOpenWorkflowView(); });
    connect(m_pListView, &QListView::customContextMenuRequested, this, &CWorkflowPane::onShowContextMenu);
    connect(m_pSearchBox, &QLineEdit::textChanged, this, &CWorkflowPane::onSearchWorkflow);
}

void CWorkflowPane::deleteCurrentWorkflow()
{
    QModelIndex index = m_pListView->currentIndex();
    if(index.isValid())
        emit doDeleteWorkflow(index);
}

void CWorkflowPane::updateWorkflowInfoPosition(QPoint itemPosition)
{
    QRect screen = QApplication::primaryScreen()->availableGeometry();
    auto size = m_pInfoDlg->size();
    auto right = m_pListView->frameGeometry().right();
    auto top = itemPosition.y() - size.height()/2;
    auto globalTopRight = m_pListView->mapToGlobal(QPoint(right, top));

    if(globalTopRight.y() + size.height() > screen.bottom())
        globalTopRight.setY(screen.bottom() - size.height());
    else if(globalTopRight.y() < 0)
        globalTopRight.setY(0);

    m_pInfoDlg->move(globalTopRight);
}

#include "moc_CWorkflowPane.cpp"
