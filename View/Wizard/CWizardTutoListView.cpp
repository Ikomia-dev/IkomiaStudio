#include "CWizardTutoListView.h"
#include "CWizardTutoListViewDelegate.h"

CWizardTutoListView::CWizardTutoListView(QWidget* parent) : QListView(parent)
{
    m_pDelegate = new CWizardTutoListViewDelegate(this);
    m_pDelegate->setSizeHint(m_itemSize);

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QListView::NoEditTriggers);
    setMouseTracking(true);
    setSpacing(5);
    setItemDelegate(m_pDelegate);

    initConnections();
}

void CWizardTutoListView::initConnections()
{
    connect(m_pDelegate, &CWizardTutoListViewDelegate::doShowInfo, [&](const QModelIndex& index){ emit doShowTutoInfo(index); });
    connect(m_pDelegate, &CWizardTutoListViewDelegate::doPlayTuto, [&](const QModelIndex& index){ emit doPlayTuto(index); });
}

void CWizardTutoListView::mouseMoveEvent(QMouseEvent *event)
{
    // update each listview item delegate look when we move
    update();

    return QListView::mouseMoveEvent(event);
}
