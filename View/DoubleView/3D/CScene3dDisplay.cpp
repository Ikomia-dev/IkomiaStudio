/*
 * Copyright (C) 2023 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CScene3dDisplay.h"


CScene3dDisplay::CScene3dDisplay(QWidget* pParent) :
    CDataDisplay(pParent, "", CDataDisplay::NONE),
    m_pVTKViewer(nullptr)
{
    m_typeId = DisplayType::SCENE_3D_DISPLAY;

    initLayout();
    initConnections();
}

void CScene3dDisplay::initLayout()
{
    setObjectName("CScene3dDisplay");

    m_pVTKViewer = new CVTKViewer(this);
    m_pLayout->addWidget(m_pVTKViewer);
}

void CScene3dDisplay::initConnections()
{ }

bool CScene3dDisplay::eventFilter(QObject* obj, QEvent* event)
{
    return CDataDisplay::eventFilter(obj, event);
}

void CScene3dDisplay::setScene3d(const CScene3d &scene3d)
{
    // The 3d scene is sent to the viewer
    m_pVTKViewer->setScene3d(scene3d);
    m_pVTKViewer->displayScene3d();
}
