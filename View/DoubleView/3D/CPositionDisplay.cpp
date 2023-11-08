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

#include "CPositionDisplay.h"
#include "../Image/CImageDisplay.h"
#include <QApplication>


CPositionDisplay::CPositionDisplay(QWidget* pParent) : CDataDisplay(pParent, "", CDataDisplay::NONE)
{
    initLayout();
    initConnections();
    m_typeId = DisplayType::POSITION_DISPLAY;
}

void CPositionDisplay::setImage(CImageScene* pScene, QImage image, const QString& name)
{
    m_pView2D->setImage(pScene, image, true);
    m_pView2D->setName(name);
    m_pView2D->fitInView();
}

void CPositionDisplay::setViewSpaceShared(bool bShared)
{
    m_pView2D->setViewSpaceShared(bShared);
}

void CPositionDisplay::setSelected(bool bSelect)
{
    if(bSelect)
    {
        QString css = QString("CPositionDisplay { border: 2px solid %1; }").arg(qApp->palette().highlight().color().name());
        setStyleSheet(css);
    }
    else
    {
        QString css = QString("CPositionDisplay { border: 2px solid %1; }").arg(qApp->palette().base().color().name());
        setStyleSheet(css);
    }
}

void CPositionDisplay::initLayout()
{
    setObjectName("CVolumeDisplay");

    m_pView2D = new CImageDisplay(this, "", CImageDisplay::DEFAULT_3D | CImageDisplay::GRAPHICS_BUTTON);
    m_pView2D->setSelectOnClick(false);

    m_pLayout->addWidget(m_pView2D);
}

void CPositionDisplay::initConnections()
{
    connect(this, &CPositionDisplay::doSelect, [&](CDataDisplay* pDisplay)
    {
        Q_UNUSED(pDisplay);
        setSelected(true);
    });
}

CImageDisplay* CPositionDisplay::getImageDisplay()
{
    return m_pView2D;
}

bool CPositionDisplay::eventFilter(QObject* obj, QEvent* event)
{
    return CDataDisplay::eventFilter(obj, event);
}
