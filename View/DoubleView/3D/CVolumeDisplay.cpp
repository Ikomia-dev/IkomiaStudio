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

#include "CVolumeDisplay.h"
#include <QSplitter>
#include "../Image/CImageDisplay.h"
#include "C3dDisplay.h"
#include <QApplication>

CVolumeDisplay::CVolumeDisplay(QWidget* pParent) : CDataDisplay(pParent, "", CDataDisplay::NONE)
{
    initLayout();
    initConnections();
    m_typeId = DisplayType::VOLUME_DISPLAY;
}

void CVolumeDisplay::setImage(CImageScene* pScene, QImage image, const QString& name)
{
    m_pView2D->setImage(pScene, image, true);
    m_pView2D->setName(name);
    m_pView2D->fitInView();
}

void CVolumeDisplay::setViewSpaceShared(bool bShared)
{
    m_pView2D->setViewSpaceShared(bShared);
}

void CVolumeDisplay::setSelected(bool bSelect)
{
    if(bSelect)
    {
        QString css = QString("CVolumeDisplay { border: 2px solid %1; }").arg(qApp->palette().highlight().color().name());
        setStyleSheet(css);
    }
    else
    {
        QString css = QString("CVolumeDisplay { border: 2px solid %1; }").arg(qApp->palette().base().color().name());
        setStyleSheet(css);
    }
}

void CVolumeDisplay::initGL()
{
    if(m_pView3D)
        m_pView3D->initGL();
}

void CVolumeDisplay::changeVolume()
{
    m_pView3D->changeVolume();
}

void CVolumeDisplay::initLayout()
{
    setObjectName("CVolumeDisplay");

    m_pView2D = new CImageDisplay(this, "", CImageDisplay::DEFAULT_3D);
    m_pView2D->setSelectOnClick(false);

    m_pView3D = new C3dDisplay(this);
    m_pView3D->setSelectOnClick(false);

    m_pSplitter = new QSplitter(this);
    m_pSplitter->addWidget(m_pView2D);
    m_pSplitter->addWidget(m_pView3D);
    m_pSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));

    m_pLayout->addWidget(m_pSplitter);
}

void CVolumeDisplay::initConnections()
{
    connect(m_pView3D, &C3dDisplay::doToggleMaximize, [&]{ emit doToggleMaximize(this);});
    connect(m_pView3D, &C3dDisplay::doDoubleClicked, [&]{ emit doToggleMaximize(this);});

    connect(this, &CVolumeDisplay::doSelect, [&](CDataDisplay* pDisplay)
    {
        Q_UNUSED(pDisplay);
        setSelected(true);
    });
}

CImageDisplay* CVolumeDisplay::getImageDisplay()
{
    return m_pView2D;
}

C3dDisplay* CVolumeDisplay::get3dDisplay()
{
    return m_pView3D;
}

bool CVolumeDisplay::eventFilter(QObject* obj, QEvent* event)
{
    return CDataDisplay::eventFilter(obj, event);
}

void CVolumeDisplay::updateRenderVolume()
{
    m_pView3D->updateRenderVolume();
}
