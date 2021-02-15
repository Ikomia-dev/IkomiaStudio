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

#ifndef CPROGRESSBAR_H
#define CPROGRESSBAR_H

#include "CProgressCircle.h"
#include <QLabel>
#include <QBoxLayout>

class CProgressSignalHandler;

class CProgressCircleManager : public QObject
{
    Q_OBJECT

    public:

        explicit CProgressCircleManager();

        CProgressCircle*    createProgress(CProgressSignalHandler* pSignal, bool bMainThread);
        CProgressCircle*    createInfiniteProgress(bool bMainThread);

        void                setProgressSize(int w, int h);
        void                setColorBase(const QColor& color);
        void                setColorBg(const QColor& color);
        void                setColorInner(const QColor& color);

    private:

        QColor                  m_colorBg;
        QColor                  m_colorInner;
        QColor                  m_colorBase;
        int                     m_width = 0;
        int                     m_height = 0;
        CProgressCircle*        m_pInfiniteProgress = nullptr; //Weak pointer
};

#endif // CPROGRESSBAR_H
