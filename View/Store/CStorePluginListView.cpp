#include "CStorePluginListView.h"
#include "CStorePluginListViewDelegate.h"

CStorePluginListView::CStorePluginListView(int pluginSource, QWidget *parent) : QListView(parent)
{
    m_pDelegate = new CStorePluginListViewDelegate(pluginSource, this);

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QListView::NoEditTriggers);
    setMouseTracking(true);
    setSpacing(20);
    setItemDelegate(m_pDelegate);

    initConnections();
}

void CStorePluginListView::mouseMoveEvent(QMouseEvent *event)
{
    // update each listview item delegate look when we move
    update();
    return QListView::mouseMoveEvent(event);
}

void CStorePluginListView::initConnections()
{
    connect(m_pDelegate, &CStorePluginListViewDelegate::doShowInfo, [&](const QModelIndex& index){ emit doShowPluginInfo(index); });
    connect(m_pDelegate, &CStorePluginListViewDelegate::doInstallPlugin, [&](const QModelIndex& index){ emit doInstallPlugin(index); });
    connect(m_pDelegate, &CStorePluginListViewDelegate::doPublishPlugin, [&](const QModelIndex& index){ emit doPublishPlugin(index); });
}
