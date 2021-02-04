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
