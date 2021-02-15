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

#include "CResponsiveUI.h"
#include <QTime>

int CResponsiveUI::m_minTimeBetweenProcessEventCall = 200;
int CResponsiveUI::m_lastCallTime = 0;

CResponsiveUI::CResponsiveUI()
{
}

void CResponsiveUI::setMinTimeBetweenProcessEventCall(double time)
{
    m_minTimeBetweenProcessEventCall = time;
}

void CResponsiveUI::processEvents()
{
    auto time = QTime::currentTime();
    int current = time.msecsSinceStartOfDay();

    if(current - m_lastCallTime > m_minTimeBetweenProcessEventCall)
    {
        QCoreApplication::processEvents();
        m_lastCallTime = current;
    }
}
