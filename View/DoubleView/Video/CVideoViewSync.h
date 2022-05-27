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

#ifndef CVIDEOVIEWSYNC_H
#define CVIDEOVIEWSYNC_H

#include <QObject>
#include <set>
#include "CVideoDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"

class CVideoViewSync : public QObject
{
    Q_OBJECT

    public:

        CVideoViewSync();

        void    syncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2);

        void    unsyncView(CVideoDisplay* pVideoDisplay);
        void    unsyncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2);

        void    syncSliderLength(CVideoDisplay* pSrcDisplay, size_t length);
        void    syncSliderPos(CVideoDisplay* pSrcDisplay, size_t pos);
        void    syncTotalTime(CVideoDisplay* pSrcDisplay, size_t time);
        void    syncCurrentTime(CVideoDisplay* pSrcDisplay, size_t time);
        void    syncFps(CVideoDisplay* pSrcDisplay, double fps);
        void    syncSourceType(CVideoDisplay* pSrcDisplay, CDataVideoBuffer::Type srcType);

    private:

        bool    isConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2) const;

        void    setConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2, bool bConnected);

    private:

        CImageViewSync  m_imageViewSync;
        std::set<std::pair<CVideoDisplay*, CVideoDisplay*>> m_connections;
};
#endif // CVIDEOVIEWSYNC_H
