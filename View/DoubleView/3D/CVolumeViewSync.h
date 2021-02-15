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

#ifndef CVOLUMEVIEWSYNC_H
#define CVOLUMEVIEWSYNC_H

#include <QObject>
#include "CVolumeDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"

class CVolumeViewSync : public QObject
{
    Q_OBJECT

    public:

        explicit CVolumeViewSync(QObject *parent = nullptr);

        void    syncView(CVolumeDisplay* pDisplay);
        void    syncView(CVolumeDisplay* pDisplay1, CVolumeDisplay* pDisplay2);
        void    syncView(CVolumeDisplay* pVolumeDisplay, CImageDisplay* pImageDisplay);

        void    unsyncView(CVolumeDisplay* pVolumeDisplay, CImageDisplay* pImageDisplay);

    private:

        bool    isConnected(CDataDisplay* pDisplay1, CDataDisplay* pDisplay2) const;

        void    setConnected(CDataDisplay* pDisplay1, CDataDisplay* pDisplay2, bool bConnected);

    private:

        CImageViewSync  m_imageViewSync;
        std::set<std::pair<CDataDisplay*, CDataDisplay*>>   m_connections;
};

#endif // CVOLUMEVIEWSYNC_H
