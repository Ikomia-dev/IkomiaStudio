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

#include "CVolumeViewSync.h"

CVolumeViewSync::CVolumeViewSync(QObject *parent) : QObject(parent)
{
}

void CVolumeViewSync::syncView(CVolumeDisplay *pDisplay)
{
    assert(pDisplay);
}

void CVolumeViewSync::syncView(CVolumeDisplay *pDisplay1, CVolumeDisplay *pDisplay2)
{
    assert(pDisplay1);
    assert(pDisplay2);

    if(pDisplay1 == pDisplay2)
        return;

    //Synchronize image views
    m_imageViewSync.syncView(pDisplay1->getImageDisplay(), pDisplay2->getImageDisplay());
    setConnected(pDisplay1, pDisplay2, true);
}

void CVolumeViewSync::syncView(CVolumeDisplay *pVolumeDisplay, CImageDisplay *pImageDisplay)
{
    assert(pVolumeDisplay);
    assert(pImageDisplay);

    auto pVolumeImgDisplay = pVolumeDisplay->getImageDisplay();
    if(pVolumeImgDisplay == pImageDisplay || pVolumeImgDisplay == nullptr)
        return;

    //Synchronize image views
    m_imageViewSync.syncView(pVolumeDisplay->getImageDisplay(), pImageDisplay);
    setConnected(pVolumeDisplay, pImageDisplay, true);
}

void CVolumeViewSync::unsyncView(CVolumeDisplay *pVolumeDisplay, CImageDisplay *pImageDisplay)
{
    assert(pVolumeDisplay);
    assert(pImageDisplay);

    if(isConnected(pVolumeDisplay, pImageDisplay) == false)
        return;

    auto pVolumeImgDisplay = pVolumeDisplay->getImageDisplay();
    if(pVolumeImgDisplay != nullptr)
        m_imageViewSync.unsyncView(pVolumeImgDisplay, pImageDisplay);

    setConnected(pVolumeDisplay, pImageDisplay, false);
}

bool CVolumeViewSync::isConnected(CDataDisplay *pDisplay1, CDataDisplay *pDisplay2) const
{
    auto it = m_connections.find(std::make_pair(pDisplay1, pDisplay2));
    if(it != m_connections.end())
        return true;
    else
    {
        it = m_connections.find(std::make_pair(pDisplay2, pDisplay1));
        return (it != m_connections.end());
    }
}

void CVolumeViewSync::setConnected(CDataDisplay *pDisplay1, CDataDisplay *pDisplay2, bool bConnected)
{
    if(bConnected == true)
        m_connections.insert(std::make_pair(pDisplay1, pDisplay2));
    else
    {
        auto it = m_connections.find(std::make_pair(pDisplay1, pDisplay2));
        if(it != m_connections.end())
            m_connections.erase(it);
        else
        {
            it = m_connections.find(std::make_pair(pDisplay2, pDisplay1));
            if(it != m_connections.end())
                m_connections.erase(it);
        }
    }
}
