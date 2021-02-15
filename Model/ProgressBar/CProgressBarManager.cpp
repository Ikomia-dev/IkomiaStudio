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

#include "CProgressBarManager.h"

//-------------------------------//
//----- CProgressBarManager -----//
//-------------------------------//
CProgressBarManager::CProgressBarManager()
{
    initProgress();
}

CProgressBarManager::CProgressBarManager(bool bUnitTest)
{
    Q_UNUSED(bUnitTest);
}

void CProgressBarManager::initProgress()
{
    auto pal = qApp->palette();
    auto colorProgress = pal.highlight().color();
    auto colorInner = pal.base().color();
    auto colorOuter = pal.alternateBase().color();
    m_progressCircleMgr.setProgressSize(64,64);
    m_progressCircleMgr.setColorBase(colorProgress);
    m_progressCircleMgr.setColorBg(colorOuter);
    m_progressCircleMgr.setColorInner(colorInner);
}

void CProgressBarManager::onSetMessage(const QString& msg)
{
    emit doSetMessage(msg);
}

void CProgressBarManager::onSetElapsedTime(double elapsedTime)
{
    emit doSetElapsedTime(elapsedTime);
}

void CProgressBarManager::launchProgress(CProgressSignalHandler* pSignal, size_t steps, const QString& msg, bool bMainThread)
{
    assert(pSignal);

    auto pProgress = m_progressCircleMgr.createProgress(pSignal, bMainThread);

    //Connections
    connect(this, &CProgressBarManager::doAbortProgressBar, pProgress, &CProgressCircle::onAbort);

    emit doShowProgressNotification(msg, Notification::INFO, pProgress);

    //Set number of steps after initializing popup window to ensure signal connection validity
    emit pSignal->doSetTotalSteps(steps);
}

void CProgressBarManager::launchProgress(CProgressSignalHandler* pSignal, const QString& msg, bool bMainThread)
{
    assert(pSignal);

    CProgressCircle* pProgress = m_progressCircleMgr.createProgress(pSignal, bMainThread);

    // Manage progress bar when protocol abort process
    connect(this, &CProgressBarManager::doAbortProgressBar, pProgress, &CProgressCircle::onAbort);

    // Disconnect abort signal when abort has been emitted once by protocol in order to avoid reentrant signal on a deleted widget
    connect(pProgress, &CProgressCircle::doDisconnectAbort, [this, pProgress]
    {
        disconnect(this, &CProgressBarManager::doAbortProgressBar, pProgress, &CProgressCircle::onAbort);
    });

    emit doShowProgressNotification(msg, Notification::INFO, pProgress);
}

void CProgressBarManager::launchInfiniteProgress(const QString &msg, bool bMainThread)
{
    if(m_bInfiniteStarted == false)
    {
        CProgressCircle* pProgress = m_progressCircleMgr.createInfiniteProgress(bMainThread);
        // Manage when infinite progress reached the end
        connect(this, &CProgressBarManager::doEndInfiniteProgressNotification, pProgress, &CProgressCircle::onFinishInfinite);

        emit doShowInfiniteProgressNotification(msg, pProgress);
        m_bInfiniteStarted = true;
    }
}

void CProgressBarManager::endInfiniteProgress()
{
    if(m_bInfiniteStarted == true)
    {
        emit doEndInfiniteProgressNotification();
        m_bInfiniteStarted = false;
    }
}

void CProgressBarManager::onAbortProgressBar()
{
    emit doAbortProgressBar();
}

#include "moc_CProgressBarManager.cpp"

