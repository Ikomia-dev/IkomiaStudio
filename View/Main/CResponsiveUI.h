#ifndef CRESPONSIVEUI_H
#define CRESPONSIVEUI_H

class CResponsiveUI
{
    public:

        CResponsiveUI();

        static void setMinTimeBetweenProcessEventCall(double time);

        static void processEvents();

    private:

        //Time in ms
        static int  m_minTimeBetweenProcessEventCall;
        static int  m_lastCallTime;
};

#endif // CRESPONSIVEUI_H
