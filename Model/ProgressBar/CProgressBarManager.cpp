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

