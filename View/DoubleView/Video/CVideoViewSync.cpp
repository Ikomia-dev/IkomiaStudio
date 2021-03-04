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

#include "CVideoViewSync.h"

CVideoViewSync::CVideoViewSync()
{
}

void CVideoViewSync::syncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2)
{
    assert(pVideoDisplay1);
    assert(pVideoDisplay2);

    if(isConnected(pVideoDisplay1, pVideoDisplay2))
        return;

    //Synchronize image views
    m_imageViewSync.syncView(pVideoDisplay1->getImageDisplay(), pVideoDisplay2->getImageDisplay());

    //Connect second view to first view
    connect(pVideoDisplay2, &CVideoDisplay::doSyncPlay, pVideoDisplay1, &CVideoDisplay::onSyncPlayVideo);
    connect(pVideoDisplay2, &CVideoDisplay::doSyncStop, pVideoDisplay1, &CVideoDisplay::onSyncStopVideo);
    connect(pVideoDisplay2, &CVideoDisplay::doUpdateVideoPos, pVideoDisplay1, &CVideoDisplay::onUpdateVideoPos);

    //Connect first view to second view
    connect(pVideoDisplay1, &CVideoDisplay::doSetFPS, pVideoDisplay2, &CVideoDisplay::onSetFPS);
    connect(pVideoDisplay1, &CVideoDisplay::doSetSliderLength, pVideoDisplay2, &CVideoDisplay::onSetSliderLength);
    connect(pVideoDisplay1, &CVideoDisplay::doSetSliderPos, pVideoDisplay2, &CVideoDisplay::onSetSliderPosSync);
    connect(pVideoDisplay1, &CVideoDisplay::doSetTotalTime, pVideoDisplay2, &CVideoDisplay::onSetTotalTime);
    connect(pVideoDisplay1, &CVideoDisplay::doSetCurrentTime, pVideoDisplay2, &CVideoDisplay::onSetCurrentTime);
    connect(pVideoDisplay1, &CVideoDisplay::doSyncPlay, pVideoDisplay2, &CVideoDisplay::onSyncPlayVideo);
    connect(pVideoDisplay1, &CVideoDisplay::doSyncStop, pVideoDisplay2, &CVideoDisplay::onSyncStopVideo);
    connect(pVideoDisplay1, &CVideoDisplay::doSetPlayVideo, pVideoDisplay2, &CVideoDisplay::onSyncSetPlayVideo);

    setConnected(pVideoDisplay1, pVideoDisplay2, true);
}

void CVideoViewSync::unsyncView(CVideoDisplay *pVideoDisplay)
{
    m_imageViewSync.unsyncView(pVideoDisplay->getImageDisplay());

    for(auto it=m_connections.begin(); it!=m_connections.end();)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pVideoDisplay || pDisplay2 == pVideoDisplay)
        {
            it++;
            unsyncView(pDisplay1, pDisplay2);
        }
        else
            it++;
    }
}

void CVideoViewSync::unsyncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2)
{
    assert(pVideoDisplay1);
    assert(pVideoDisplay2);

    if(isConnected(pVideoDisplay1, pVideoDisplay2) == false)
        return;

    m_imageViewSync.unsyncView(pVideoDisplay1->getImageDisplay(), pVideoDisplay2->getImageDisplay());

    //Disconnect second view to first view
    disconnect(pVideoDisplay2, &CVideoDisplay::doSyncPlay, pVideoDisplay1, &CVideoDisplay::onSyncPlayVideo);
    disconnect(pVideoDisplay2, &CVideoDisplay::doSyncStop, pVideoDisplay1, &CVideoDisplay::onSyncStopVideo);
    disconnect(pVideoDisplay2, &CVideoDisplay::doUpdateVideoPos, pVideoDisplay1, &CVideoDisplay::onUpdateVideoPos);

    //Disconnect first view to second view
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetFPS, pVideoDisplay2, &CVideoDisplay::onSetFPS);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetSliderLength, pVideoDisplay2, &CVideoDisplay::onSetSliderLength);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetSliderPos, pVideoDisplay2, &CVideoDisplay::onSetSliderPosSync);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetTotalTime, pVideoDisplay2, &CVideoDisplay::onSetTotalTime);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetCurrentTime, pVideoDisplay2, &CVideoDisplay::onSetCurrentTime);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSyncPlay, pVideoDisplay2, &CVideoDisplay::onSyncPlayVideo);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSyncStop, pVideoDisplay2, &CVideoDisplay::onSyncStopVideo);
    disconnect(pVideoDisplay1, &CVideoDisplay::doSetPlayVideo, pVideoDisplay2, &CVideoDisplay::onSyncSetPlayVideo);

    setConnected(pVideoDisplay1, pVideoDisplay2, false);
}

void CVideoViewSync::syncSliderLength(CVideoDisplay *pSrcDisplay, int length)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->onSetSliderLength(length);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->onSetSliderLength(length);
    }
}

void CVideoViewSync::syncSliderPos(CVideoDisplay *pSrcDisplay, int pos)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->onSetSliderPos(pos);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->onSetSliderPos(pos);
    }
}

void CVideoViewSync::syncTotalTime(CVideoDisplay *pSrcDisplay, int time)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->onSetTotalTime(time);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->onSetTotalTime(time);
    }
}

void CVideoViewSync::syncCurrentTime(CVideoDisplay *pSrcDisplay, int time)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->onSetCurrentTime(time);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->onSetCurrentTime(time);
    }
}

void CVideoViewSync::syncFps(CVideoDisplay *pSrcDisplay, double fps)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->onSetFPS(fps);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->onSetFPS(fps);
    }
}

void CVideoViewSync::syncSourceType(CVideoDisplay *pSrcDisplay, CDataVideoBuffer::Type srcType)
{
    for(auto it=m_connections.begin(); it!=m_connections.end(); ++it)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pSrcDisplay)
            pDisplay2->setSourceType(srcType);
        else if(pDisplay2 == pSrcDisplay)
            pDisplay1->setSourceType(srcType);
    }
}

bool CVideoViewSync::isConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2) const
{
    auto it = m_connections.find(std::make_pair(pVideoDisplay1, pVideoDisplay2));
    if(it != m_connections.end())
        return true;
    else
    {
        it = m_connections.find(std::make_pair(pVideoDisplay2, pVideoDisplay1));
        return (it != m_connections.end());
    }
}

void CVideoViewSync::setConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2, bool bConnected)
{
    if(bConnected == true)
        m_connections.insert(std::make_pair(pVideoDisplay1, pVideoDisplay2));
    else
    {
        auto it = m_connections.find(std::make_pair(pVideoDisplay1, pVideoDisplay2));
        if(it != m_connections.end())
            m_connections.erase(it);
        else
        {
            it = m_connections.find(std::make_pair(pVideoDisplay2, pVideoDisplay1));
            if(it != m_connections.end())
                m_connections.erase(it);
        }
    }
}
