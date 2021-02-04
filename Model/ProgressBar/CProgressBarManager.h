#ifndef CPROGRESSBARMANAGER_H
#define CPROGRESSBARMANAGER_H

#include <QObject>
#include "Main/AppDefine.hpp"
#include "Core/CProtocol.h"
#include "View/Common/CProgressCircleManager.h"

//-------------------------------//
//----- CProgressBarManager -----//
//-------------------------------//
class CProgressBarManager : public QObject
{
    Q_OBJECT

    public:

        CProgressBarManager();
        CProgressBarManager(bool bUnitTest);

        void    initProgress();
        void    launchProgress(CProgressSignalHandler* pSignal, size_t steps, const QString& msg, bool bMainThread);
        void    launchProgress(CProgressSignalHandler* pSignal, const QString& msg, bool bMainThread);
        void    launchInfiniteProgress(const QString& msg, bool bMainThread);

        void    endInfiniteProgress();

    signals:

        void    doSetValue(uint idProgress, int value);
        void    doSetMessage(const QString& msg);
        void    doSetElapsedTime(double elapsedTime);

        void    doAbortProgressBar();

        void    doShowProgressNotification(const QString& msg, Notification type, CProgressCircle* pProgress, int duration=Ikomia::_NotifDefaultDuration);
        void    doShowInfiniteProgressNotification(const QString& msg, CProgressCircle* pProgress);

        void    doEndInfiniteProgressNotification();

    public slots:

        void    onSetMessage(const QString& msg);
        void    onSetElapsedTime(double elapsedTime);
        void    onAbortProgressBar();

    private:

        CProgressCircleManager  m_progressCircleMgr;
        bool                    m_bInfiniteStarted = false;
};

#endif // CPROGRESSBARMANAGER_H
