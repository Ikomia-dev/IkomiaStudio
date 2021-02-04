#include "CWizardStepListView.h"
#include "CWizardStepListViewDelegate.h"

CWizardStepListView::CWizardStepListView(QWidget* parent) : QListView(parent)
{
    m_pDelegate = new CWizardStepListViewDelegate(this);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QListView::NoEditTriggers);
    setSelectionMode(QListView::NoSelection);
    setMouseTracking(true);
    setSpacing(5);
    setFlow(QListView::TopToBottom);
    setItemDelegate(m_pDelegate);
}
