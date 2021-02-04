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
