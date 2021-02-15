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

#include "CImageViewSync.h"
#include <QScrollBar>
#include "Protocol/CViewPropertyIO.h"
#include <QTimer>

CImageViewSync::CImageViewSync()
{
}

void CImageViewSync::syncView(CImageDisplay *pImgDisplay1, CImageDisplay *pImgDisplay2)
{
    assert(pImgDisplay1);
    assert(pImgDisplay2);

    if(pImgDisplay1 == pImgDisplay2)
        return;

    auto pView1 = pImgDisplay1->getView();
    auto pView2 = pImgDisplay2->getView();

    if(pView1 == nullptr || pView2 == nullptr)
        return;

    if(isConnected(pImgDisplay1, pImgDisplay2))
        return;

    //Connect second view to first view
    connect(pView2, &CImageView::doZoomEvent, pView1, &CImageView::onUpdateZoom);
    connect(pView2, &CImageView::doZoomFit, pView1, &CImageView::onZoomFit);
    connect(pView2, &CImageView::doZoomOriginal, pView1, &CImageView::onZoomOriginal);
    connect(pView2, &CImageView::doZoomIn, pView1, &CImageView::onZoomIn);
    connect(pView2, &CImageView::doZoomOut, pView1, &CImageView::onZoomOut);
    connect(pView2, &CImageView::doUpdateCenter, pView1, &CImageView::onUpdateCenter);
    connect(pView2, &CImageView::doUpdateTargetPos, pView1, &CImageView::onUpdateTargetPos);

    //Connect first view to second view
    connect(pView1, &CImageView::doZoomEvent, pView2, &CImageView::onUpdateZoom);
    connect(pView1, &CImageView::doZoomFit, pView2, &CImageView::onZoomFit);
    connect(pView1, &CImageView::doZoomOriginal, pView2, &CImageView::onZoomOriginal);
    connect(pView1, &CImageView::doZoomIn, pView2, &CImageView::onZoomIn);
    connect(pView1, &CImageView::doZoomOut, pView2, &CImageView::onZoomOut);
    connect(pView1, &CImageView::doUpdateCenter, pView2, &CImageView::onUpdateCenter);
    connect(pView1, &CImageView::doUpdateTargetPos, pView2, &CImageView::onUpdateTargetPos);

    setConnected(pImgDisplay1, pImgDisplay2, true);
}

void CImageViewSync::unsyncView(CImageDisplay *pImgDisplay)
{
    for(auto it=m_connections.begin(); it!=m_connections.end();)
    {
        auto pDisplay1 = it->first;
        auto pDisplay2 = it->second;

        if(pDisplay1 == pImgDisplay || pDisplay2 == pImgDisplay)
        {
            it++;
            unsyncView(pDisplay1, pDisplay2);
        }
        else
            it++;
    }
}

void CImageViewSync::unsyncView(CImageDisplay *pImgDisplay1, CImageDisplay *pImgDisplay2)
{
    assert(pImgDisplay1);
    assert(pImgDisplay2);

    if(isConnected(pImgDisplay1, pImgDisplay2) == false)
        return;

    auto pView1 = pImgDisplay1->getView();
    auto pView2 = pImgDisplay2->getView();

    if(pView1 == nullptr || pView2 == nullptr)
        return;

    //Disconnect view2 to view1
    disconnect(pView2, &CImageView::doZoomEvent, pView1, &CImageView::onUpdateZoom); // sync smooth zoom
    disconnect(pView2, &CImageView::doZoomFit, pView1, &CImageView::onZoomFit); // sync fit in view
    disconnect(pView2, &CImageView::doZoomOriginal, pView1, &CImageView::onZoomOriginal); // sync full size
    disconnect(pView2, &CImageView::doZoomIn, pView1, &CImageView::onZoomIn); // sync manual zoom
    disconnect(pView2, &CImageView::doZoomOut, pView1, &CImageView::onZoomOut); // sync manual zoom
    disconnect(pView2, &CImageView::doUpdateCenter, pView1, &CImageView::onUpdateCenter); // sync displacement
    disconnect(pView2, &CImageView::doUpdateTargetPos, pView1, &CImageView::onUpdateTargetPos); // sync zoom position under mouse

    //Disconnect view1 to view2
    disconnect(pView1, &CImageView::doZoomEvent, pView2, &CImageView::onUpdateZoom);
    disconnect(pView1, &CImageView::doZoomFit, pView2, &CImageView::onZoomFit);
    disconnect(pView1, &CImageView::doZoomOriginal, pView2, &CImageView::onZoomOriginal);
    disconnect(pView1, &CImageView::doZoomIn, pView2, &CImageView::onZoomIn);
    disconnect(pView1, &CImageView::doZoomOut, pView2, &CImageView::onZoomOut);
    disconnect(pView1, &CImageView::doUpdateCenter, pView2, &CImageView::onUpdateCenter);
    disconnect(pView1, &CImageView::doUpdateTargetPos, pView2, &CImageView::onUpdateTargetPos);

    setConnected(pImgDisplay1, pImgDisplay2, false);
}

bool CImageViewSync::isConnected(CImageDisplay *pImgDisplay1, CImageDisplay *pImgDisplay2) const
{
    auto it = m_connections.find(std::make_pair(pImgDisplay1, pImgDisplay2));
    if(it != m_connections.end())
        return true;
    else
    {
        it = m_connections.find(std::make_pair(pImgDisplay2, pImgDisplay1));
        if(it != m_connections.end())
            return true;
        else
            return false;
    }
}

void CImageViewSync::setConnected(CImageDisplay *pImgDisplay1, CImageDisplay *pImgDisplay2, bool bConnected)
{
    if(bConnected == true)
        m_connections.insert(std::make_pair(pImgDisplay1, pImgDisplay2));
    else
    {
        auto it = m_connections.find(std::make_pair(pImgDisplay1, pImgDisplay2));
        if(it != m_connections.end())
            m_connections.erase(it);
        else
        {
            it = m_connections.find(std::make_pair(pImgDisplay2, pImgDisplay1));
            if(it != m_connections.end())
                m_connections.erase(it);
        }
    }
}
