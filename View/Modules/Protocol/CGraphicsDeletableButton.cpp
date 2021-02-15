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

#include "CGraphicsDeletableButton.h"
#include "View/Common/CSvgButton.h"

CGraphicsDeletableButton::CGraphicsDeletableButton(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsProxyWidget(parent, wFlags)
{
}

void CGraphicsDeletableButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    showDeleteButton();
    assert(widget());
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.highlight().color();
    auto styleStr = QString("border: 2px solid %1; border-radius: 5px; background: transparent; padding-left: 0px; padding-right: 0px; padding-top: 0px; padding-bottom: 0px;").arg(hoverBorderColor.name());
    widget()->setStyleSheet(styleStr);
}

void CGraphicsDeletableButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    hideDeleteButton();
    assert(widget());
    widget()->setStyleSheet("border: none; background: transparent; padding-left: 0px; padding-right: 0px; padding-top: 0px; padding-bottom: 0px;");
}

void CGraphicsDeletableButton::showDeleteButton()
{
    if(m_pProxyDeleteBtn == nullptr)
    {
        CSvgButton* pBtn = new CSvgButton(":/Images/close-connect.svg", true);
        pBtn->setToolTip(tr("Delete"));
        connect(pBtn, &CSvgButton::clicked, this, [&]{ emit doDelete(); });
        m_pProxyDeleteBtn = new QGraphicsProxyWidget(this);
        m_pProxyDeleteBtn->setWidget(pBtn);
    }

    QRectF rect = boundingRect();
    m_pProxyDeleteBtn->setPos(rect.right()-m_pProxyDeleteBtn->size().width()/2, rect.top()-m_pProxyDeleteBtn->size().height()/2);
    m_pProxyDeleteBtn->show();
}

void CGraphicsDeletableButton::hideDeleteButton()
{
    m_pProxyDeleteBtn->hide();
}
