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

#include "CProgressCircleManager.h"
#include <QApplication>
#include <QParallelAnimationGroup>

CProgressCircleManager::CProgressCircleManager()
{
}

CProgressCircle* CProgressCircleManager::createProgress(CProgressSignalHandler* pSignal, bool bMainThread)
{
    CProgressCircle* pProgress = new CProgressCircle(pSignal, bMainThread);
    pProgress->setFixedSize(m_width, m_height);
    pProgress->setColorBase(m_colorBase);
    pProgress->setColorBg(m_colorBg);
    pProgress->setColorInner(m_colorInner);
    return pProgress;
}

CProgressCircle *CProgressCircleManager::createInfiniteProgress(bool bMainThread)
{
    m_pInfiniteProgress = new CProgressCircle(nullptr, bMainThread);
    m_pInfiniteProgress->setFixedSize(m_width, m_height);
    m_pInfiniteProgress->setColorBase(m_colorBase);
    m_pInfiniteProgress->setColorBg(m_colorBg);
    m_pInfiniteProgress->setColorInner(m_colorInner);

    return m_pInfiniteProgress;
}

void CProgressCircleManager::setProgressSize(int w, int h)
{
    m_width = w;
    m_height = h;
}

void CProgressCircleManager::setColorBase(const QColor& color)
{
    m_colorBase = color;
}

void CProgressCircleManager::setColorBg(const QColor& color)
{
    m_colorBg = color;
}

void CProgressCircleManager::setColorInner(const QColor& color)
{
    m_colorInner = color;
}
