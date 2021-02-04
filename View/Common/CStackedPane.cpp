#include "CStackedPane.h"

CStackedPane::CStackedPane(QWidget* parent) : CPane(parent)
{
    m_pWidgets = new QStackedWidget;
    m_pLayout->addWidget(m_pWidgets);
}

int CStackedPane::addPane(QWidget *pane)
{
    return m_pWidgets->addWidget(pane);
}

void CStackedPane::togglePane(QWidget *pane)
{
    if(m_pWidgets->currentWidget() == pane || m_bIsOpened == false)
        animate();

    m_pWidgets->setCurrentWidget(pane);
}

void CStackedPane::showPane(QWidget *pPane)
{
    if(m_bIsOpened == false)
        animate();

    m_pWidgets->setCurrentWidget(pPane);
}
