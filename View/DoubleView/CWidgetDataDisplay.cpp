#include "CWidgetDataDisplay.h"

CWidgetDataDisplay::CWidgetDataDisplay(QWidget* pWidget, QWidget* pParent, bool bDeleteWidget, int flags) : CDataDisplay(pParent, "", flags)
{
    m_pWidget = pWidget;
    m_bDeleteWidget = bDeleteWidget;
    initLayout();
    m_typeId = DisplayType::WIDGET_DISPLAY;
}

CWidgetDataDisplay::~CWidgetDataDisplay()
{
    if(m_bDeleteWidget == false)
        removeWidget();
}

void CWidgetDataDisplay::initLayout()
{
    setObjectName("CWidgetDataDisplay");
    if(m_pWidget)
    {
        m_pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pLayout->addWidget(m_pWidget);
    }
}

void CWidgetDataDisplay::removeWidget()
{
    if(m_pWidget)
    {
        assert(m_pLayout);
        m_pLayout->removeWidget(m_pWidget);
        m_pWidget->setParent(nullptr);
        m_pWidget = nullptr;
    }
}

void CWidgetDataDisplay::onClose()
{
    if(m_bDeleteWidget == false)
        removeWidget();

    CDataDisplay::onClose();
}

bool CWidgetDataDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == this || obj == m_pWidget)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}
