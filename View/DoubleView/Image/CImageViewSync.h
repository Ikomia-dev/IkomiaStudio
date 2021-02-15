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

#ifndef CVIEWSYNC_H
#define CVIEWSYNC_H

#include <QObject>
#include <set>
#include "CImageDisplay.h"

class CImageViewSync : public QObject
{
    Q_OBJECT

    public:

        CImageViewSync();

        void    syncView(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2);

        void    unsyncView(CImageDisplay *pImgDisplay);
        void    unsyncView(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2);

    private:

        bool    isConnected(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2) const;

        void    setConnected(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2, bool bConnected);

    private:

        std::set<std::pair<CImageDisplay*, CImageDisplay*>> m_connections;
};

#endif // CVIEWSYNC_H
