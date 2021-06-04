/*
 * Copyright (C) 2021 Ikomia SAS
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

#ifndef CPROGRESSBARMANAGER_H
#define CPROGRESSBARMANAGER_H

#include <QObject>
#include "Main/AppDefine.hpp"
#include "Core/CWorkflow.h"
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
