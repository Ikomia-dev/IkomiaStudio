#include "CProcessListView.h"
#include "CProcessListViewDelegate.h"
#include <QListView>

CProcessListView::CProcessListView()
{
    setObjectName("CProcessListView");
    m_pLayout = new QHBoxLayout;
    m_pListView = new QListView;
    m_pDelegate = new CProcessListViewDelegate(m_pListView);

    m_pListView->setViewMode(QListView::IconMode);
    m_pListView->setMovement(QListView::Static);
    m_pListView->setEditTriggers(QListView::NoEditTriggers);
    m_pListView->setSpacing(30);
    m_pListView->setIconSize(m_itemSize);
    // Position problem with gridsize (overlap upside)
    //m_pListView->setGridSize(QSize(210,210));
    m_pListView->setResizeMode(QListView::Adjust);
    m_pListView->setMouseTracking(true);
    m_pDelegate->setSizeHint(m_itemSize);
    m_pListView->setItemDelegate(m_pDelegate);
    m_pListView->installEventFilter(this);

    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setSpacing(0);
    m_pLayout->addWidget(m_pListView);
    setLayout(m_pLayout);

    initConnections();
}

void CProcessListView::initConnections()
{
    connect(m_pListView, &QListView::clicked, this, &CProcessListView::onListViewClicked);
    connect(m_pListView, &QListView::doubleClicked, this, &CProcessListView::onListViewDoubleClicked);
    connect(m_pDelegate, &CProcessListViewDelegate::doAddProcess, [&](const QString processName){ emit doAddProcess(processName); });
    connect(m_pDelegate, &CProcessListViewDelegate::doShowInfo, [&](const QModelIndex& index){ emit doShowProcessInfo(index); });
}

void CProcessListView::setModel(QAbstractItemModel* pModel)
{
    m_pListView->setModel(pModel);
    connect(m_pListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CProcessListView::onCurrentChanged);
}

void CProcessListView::setModelColumn(int column)
{
    m_pListView->setModelColumn(column);
}

QListView*CProcessListView::getListView()
{
    return m_pListView;
}

QSize CProcessListView::getPluginSize()
{
    return m_itemSize;
}

void CProcessListView::onUpdateModel(QAbstractItemModel* pModel)
{
    setModel(pModel);
}

void CProcessListView::onListViewDoubleClicked(const QModelIndex& index)
{
    if(index.isValid())
        emit doListViewDoubleCLicked(index.data().toString());
}

void CProcessListView::onListViewClicked(const QModelIndex &index)
{
    if(index.isValid())
        emit doListViewLicked(index);
}

void CProcessListView::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    emit doCurrentChanged(current, previous);
}

void CProcessListView::onTreeViewClicked(const QModelIndex& index)
{
    auto pModel = static_cast<const CProcessProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pItem = static_cast<CProcessModel::TreeItem*>(srcIndex.internalPointer());
    if(pItem->getTypeId() == TreeItemType::PROCESS)
    {
        auto pProxyTable = static_cast<const CProcessTableProxyModel*>(m_pListView->model());
        if(pProxyTable)
        {
            // Fied (column) 1 is process name
            auto currentIndex = pProxyTable->index(index.row(), 1);
            m_pListView->setCurrentIndex(currentIndex);
        }
    }
}

bool CProcessListView::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if(event->type() == QEvent::HoverMove)
    {
        // update listview item when we move mouse
        m_pListView->update();
    }

    return false;
}
