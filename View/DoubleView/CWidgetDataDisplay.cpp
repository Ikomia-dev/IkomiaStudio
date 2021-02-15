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
