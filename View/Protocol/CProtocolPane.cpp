#include "CProtocolPane.h"
#include <QListView>
#include "CProtocolInfoDlg.h"

CProtocolPane::CProtocolPane(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initContextMenu();
    initConnections();
    m_pListView->installEventFilter(this);
}

void CProtocolPane::onSetModel(QStringListModel *pModel)
{
    m_pListView->setModel(pModel);
}

void CProtocolPane::onSetFromImageModel(QStringListModel *pModel)
{
    // Current img standby
    //m_pFromImageListView->setModel(pModel);
}

void CProtocolPane::onSetDescription(const QString &text)
{
    if(m_pInfoDlg == nullptr)
    {
        m_pInfoDlg = new CProtocolInfoDlg(this);
        connect(m_pInfoDlg, &CProtocolInfoDlg::doLoadProtocol, [&]{ onLoadSelectedProtocol(); m_pInfoDlg->hide(); });
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

    m_pProtocolDesc->setHtml(description);

    QRect rcItem = m_pListView->visualRect(indexes[0]);
    QPoint position((rcItem.left()+rcItem.right()) / 2, (rcItem.top()+rcItem.bottom()) / 2);
    updateProtocolInfoPosition(position);
}

void CProtocolPane::onSearchProtocol(const QString &text)
{
    emit doSearchProtocol(text);
}

void CProtocolPane::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = m_pListView->indexAt(pos);
    if(!index.isValid())
        return;

    m_contextMenu.displayMenu(0, m_pListView->mapToGlobal(pos), index);
}

void CProtocolPane::onShowProtocolInfo(const QModelIndex &index)
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() == 0)
        return;

    auto protocolName = indexes[0].data(Qt::DisplayRole).toString();
    if(m_pInfoDlg && m_pInfoDlg->isHidden() == false && protocolName == m_pInfoDlg->getName())
        m_pInfoDlg->hide();
    else
        emit doGetProtocolInfo(index);
}

void CProtocolPane::onLoadSelectedProtocol()
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() > 0)
    {
        emit doLoadProtocol(indexes[0]);
        emit doOpenProtocolView();
    }
}

void CProtocolPane::onDeleteSelectedProtocol()
{
    auto indexes = m_pListView->selectionModel()->selectedIndexes();
    if(indexes.size() > 0)
        emit doDeleteProtocol(indexes[0]);
}

bool CProtocolPane::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
        switch(pKeyEvent->key())
        {
            case Qt::Key_Delete:
                deleteCurrentProtocol();
                break;
        }
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void CProtocolPane::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    if(m_pInfoDlg)
        m_pInfoDlg->hide();
}

void CProtocolPane::initLayout()
{
    //Toolbar
    QAction* pLoadAct = new QAction(QIcon(":/Images/open.png"), tr("Load workflow"), this);
    pLoadAct->setStatusTip(tr("Load selected workflow"));
    connect(pLoadAct, &QAction::triggered, this, &CProtocolPane::onLoadSelectedProtocol);

    QAction* pDeleteAct = new QAction(QIcon(":/Images/delete.png"), tr("Delete workflow"), this);
    pDeleteAct->setStatusTip(tr("Delete selected workflow"));
    connect(pDeleteAct, &QAction::triggered, this, &CProtocolPane::onDeleteSelectedProtocol);

    QToolBar* pToolbar = new QToolBar;
    pToolbar->addAction(pLoadAct);
    pToolbar->widgetForAction(pLoadAct)->setObjectName("CToolbarBtn");
    pToolbar->addAction(pDeleteAct);
    pToolbar->widgetForAction(pDeleteAct)->setObjectName("CToolbarBtn");

    //Search Box
    m_pSearchBox = new QLineEdit;
    m_pSearchBox->setPlaceholderText(tr("<Search by keywords>"));

    m_pListView = new QListView(this);
    m_pListView->setSelectionMode(QListView::SingleSelection);
    m_pListView->setEditTriggers(QListView::EditKeyPressed);
    m_pListView->setContextMenuPolicy(Qt::CustomContextMenu);

    QWidget* pContainer = new QWidget;
    QVBoxLayout* pContainerLayout = new QVBoxLayout;
    pContainerLayout->addWidget(pToolbar);
    pContainerLayout->addWidget(m_pSearchBox);
    pContainerLayout->addWidget(m_pListView);
    pContainerLayout->setStretchFactor(m_pListView, 3);
    pContainer->setLayout(pContainerLayout);

    // Protocol description
    auto pProtocolDescription =  new QLabel(tr("Workflow description"));
    pProtocolDescription->setAlignment(Qt::AlignLeft);

    pContainerLayout->addWidget(pProtocolDescription);

    m_pProtocolDesc = new QTextEdit(this);
    m_pProtocolDesc->setObjectName("CDataDisplay");
    pContainerLayout->addWidget(m_pProtocolDesc);
    pContainerLayout->setStretchFactor(m_pProtocolDesc, 1);

    // Current img standby
    /*m_pFromImageListView = new QListView(this);
    m_pFromImageListView->setSelectionMode(QListView::SingleSelection);
    m_pFromImageListView->setEditTriggers(QListView::EditKeyPressed);

    auto pFromImageListLabel =  new QLabel(tr("Used with current image"));
    pFromImageListLabel->setAlignment(Qt::AlignLeft);

    pContainerLayout->addWidget(pFromImageListLabel);

    pContainerLayout->addWidget(m_pFromImageListView);
    pContainerLayout->setStretchFactor(m_pFromImageListView, 1);*/

    QToolBox* pToolBox = new QToolBox;
    pToolBox->addItem(pContainer, tr("Workflow"));
    auto pLayout = new QVBoxLayout;
    pLayout->addWidget(pToolBox);

    setLayout(pLayout);
}

void CProtocolPane::initContextMenu()
{
    m_contextMenu.addAction(0, tr("Load workflow"),
                            [this](QModelIndex& index){ emit doLoadProtocol(index); emit doOpenProtocolView(); },
                            QIcon(":/Images/open.png") );
    m_contextMenu.addAction(0, tr("Delete workflow"),
                            [this](QModelIndex& index){ emit doDeleteProtocol(index); },
                            QIcon(":/Images/delete.png") );
}

void CProtocolPane::initConnections()
{
    connect(m_pListView, &QListView::clicked, this, &CProtocolPane::onShowProtocolInfo);
    connect(m_pListView, &QListView::doubleClicked, [&](const QModelIndex& index){ emit doLoadProtocol(index); emit doOpenProtocolView(); });
    connect(m_pListView, &QListView::customContextMenuRequested, this, &CProtocolPane::onShowContextMenu);
    connect(m_pSearchBox, &QLineEdit::textChanged, this, &CProtocolPane::onSearchProtocol);
}

void CProtocolPane::deleteCurrentProtocol()
{
    QModelIndex index = m_pListView->currentIndex();
    if(index.isValid())
        emit doDeleteProtocol(index);
}

void CProtocolPane::updateProtocolInfoPosition(QPoint itemPosition)
{
    QRect screen = QApplication::desktop()->availableGeometry(this);
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

#include "moc_CProtocolPane.cpp"
