/**
 * @file      CDataListView.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDataListView
 *
 * @details   Details
 */

#include "CDataListView.h"
#include "CDataListViewDelegate.h"
#include "Model/CMainModel.h"
#include "Model/Project/CProjectViewProxyModel.h"
#include <QListView>

CDataListView::CDataListView()
{
    setObjectName("CDataListView");
    m_pLayout = new QHBoxLayout;
    m_pListView = new QListView;
    m_pProxyModel = new CProjectViewProxyModel(this);

    m_pListView->setModel(m_pProxyModel);
    m_pListView->setViewMode(QListView::IconMode);
    m_pListView->setMovement(QListView::Static);
    m_pListView->setSpacing(5);
    m_pListView->setIconSize(QSize(200,200));
    // Position problem with gridsize (overlap upside)
    //m_pListView->setGridSize(QSize(210,210));
    m_pListView->setResizeMode(QListView::Adjust);
    m_pListView->setMouseTracking(true);
    m_pListView->setItemDelegate(new CDataListViewDelegate(m_pListView));
    m_pListView->installEventFilter(this);

    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setSpacing(0);
    m_pLayout->addWidget(m_pListView);
    setLayout(m_pLayout);

    initConnections();
}

void CDataListView::initConnections()
{
    connect(m_pListView, &QListView::clicked, this, &CDataListView::onListViewClicked);
    connect(m_pListView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& /*deselected*/)
    {
        if(!selected.indexes().isEmpty())
            onListViewClicked(selected.indexes().first());
    });
    connect(m_pListView, &QListView::doubleClicked, this, &CDataListView::onListViewDoubleClicked);
    // update listview items when small icons/images/thumbnails are created
    connect(m_pProxyModel, &CProjectViewProxyModel::dataChanged, [this]{ m_pListView->update(); });
}

QModelIndex CDataListView::getImageIndex(const QModelIndex &imgChildIndex)
{
    QModelIndex imageIndex = imgChildIndex;
    auto pModel = static_cast<const CMultiProjectModel*>(imgChildIndex.model());
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(imgChildIndex.internalPointer());
    auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, imgChildIndex);
    auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

    while(itemPtr && itemPtr->getTypeId() != TreeItemType::IMAGE)
    {
        imageIndex = imageIndex.parent();
        wrapIndex = wrapIndex.parent();
        itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());
    }
    return imageIndex;
}

void CDataListView::setSourceModel(QAbstractItemModel* pModel)
{
    m_pProxyModel->setSourceModel(pModel);
}

void CDataListView::updateIndex(const QModelIndex& index)
{
    if(index.isValid())
    {
        // Get item type by wrapping multimodel index to projectmodel index
        auto pModel = static_cast<const CMultiProjectModel*>(index.model());
        auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
        auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
        auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

        if(itemPtr->getTypeId() == TreeItemType::IMAGE)
        {
            // Get proxymodel index from multiproject index
            auto mapSrcIndex = m_pProxyModel->mapFromSource(index);
            m_pListView->setRootIndex(mapSrcIndex.parent());
            m_pListView->setCurrentIndex(mapSrcIndex);
        }
        else if(itemPtr->getTypeId() == TreeItemType::VIDEO ||
                itemPtr->getTypeId() == TreeItemType::LIVE_STREAM)
        {
            // Get proxymodel index from multiproject index
            auto mapSrcIndex = m_pProxyModel->mapFromSource(index);
            m_pListView->setRootIndex(mapSrcIndex.parent());
            m_pListView->setCurrentIndex(mapSrcIndex);
        }
        else if(itemPtr->getTypeId() == TreeItemType::GRAPHICS_LAYER ||
                itemPtr->getTypeId() == TreeItemType::RESULT)
        {
            auto mapImageIndex = m_pProxyModel->mapFromSource(getImageIndex(index));
            m_pListView->setRootIndex(mapImageIndex.parent());
            m_pListView->setCurrentIndex(mapImageIndex);
        }
        else if(itemPtr->getTypeId() == TreeItemType::FOLDER)
        {
            auto parentItemPtr = static_cast<ProjectTreeItem*>(wrapIndex.parent().internalPointer());
            if(parentItemPtr && parentItemPtr->getTypeId() == TreeItemType::IMAGE)
            {
                auto mapImageIndex = m_pProxyModel->mapFromSource(index.parent());
                m_pListView->setRootIndex(mapImageIndex.parent());
                m_pListView->setCurrentIndex(mapImageIndex);
            }
            else
                m_pListView->setRootIndex(m_pProxyModel->mapFromSource(index));
        }
        else
            m_pListView->setRootIndex(m_pProxyModel->mapFromSource(index));
    }
}

QAbstractProxyModel* CDataListView::proxyModel()
{
    return m_pProxyModel;
}

void CDataListView::onUpdateSourceModel(QAbstractItemModel* pModel)
{
    setSourceModel(pModel);
}

void CDataListView::onListViewDoubleClicked(const QModelIndex& index)
{
    if(index.isValid())
    {
        if(index.model()->index(0,0,index).isValid())
            m_pListView->setRootIndex(index);

        QModelIndex srcIndex = m_pProxyModel->mapToSource(index);
        emit doDisplayData(srcIndex);
        emit doUpdateIndex(srcIndex);
    }
}

void CDataListView::onListViewClicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        QModelIndex srcIndex = m_pProxyModel->mapToSource(index);
        emit doUpdateIndex(srcIndex);
    }
}

bool CDataListView::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if(event->type() == QEvent::HoverMove)
    {
        // update listview item (small images) when we move mouse
        m_pListView->update();
    }

    return false;
}
