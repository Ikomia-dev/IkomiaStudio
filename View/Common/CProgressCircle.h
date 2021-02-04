/////////////////////////////////////////////////////////////////////////////
// Date of creation: 04.07.2013
// Creator: Alexander Egorov aka mofr
// Authors: mofr
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include <QPropertyAnimation>

class CProgressSignalHandler;

class CProgressCircle : public QWidget
{
        Q_OBJECT
        Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
        Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
        Q_PROPERTY(qreal innerRadius READ innerRadius WRITE setInnerRadius)
        Q_PROPERTY(qreal outerRadius READ outerRadius WRITE setOuterRadius)
        Q_PROPERTY(QColor color READ color WRITE setColor)

        //private
        Q_PROPERTY(qreal infiniteAnimationValue READ infiniteAnimationValue WRITE setInfiniteAnimationValue)
        Q_PROPERTY(int visibleValue READ visibleValue WRITE setVisibleValue)

    public:

        explicit CProgressCircle(CProgressSignalHandler* pSignal, bool bMainThread, QWidget *parent = 0);
        ~CProgressCircle();

        virtual QSize sizeHint() const;

        int     value() const;

        /**
         * @brief maximum
         * If maximum <= 0, widget shows infinite process;
         * @return
         */
        int     maximum() const;

        /**
         * @brief innerRadius = [0.0 .. 1.0]
         * @return
         */
        qreal   innerRadius() const;

        /**
         * @brief outerRadius = [0.0 .. 1.0]
         * @return
         */
        qreal   outerRadius() const;

        QColor  color() const;

        void    reset();

        void    start();
        void    finish(bool bSuccess);
        void    finishInfinite(bool bSuccess);

        void    startInfinite();
        void    stop();

        CProgressSignalHandler* getSignal();

    public slots:

        void    onProgress();
        void    onAbort();
        void    onSetTotalSteps(size_t maxSteps);
        void    onFinishInfinite();

        void    setValue(int value);
        void    setMaximum(int maximum);

        void    setInnerRadius(qreal innerRadius);
        void    setOuterRadius(qreal outerRadius);

        void    setColor(QColor color);
        void    setColorBase(QColor color);
        void    setColorBg(QColor color);
        void    setColorInner(QColor color);

        void    setPercent(int percent);

    signals:

        void    valueChanged(int);
        void    maximumChanged(int);
        void    doFinish();
        void    doAbort();
        void    doDisconnectAbort();
        void    doSetMessage(const QString& msg);

    protected:

        void    paintEvent(QPaintEvent *);

    private slots:

        void    setInfiniteAnimationValue(qreal value);
        void    setVisibleValue(int value);
        void    onSetMessage(const QString& msg);
        void    onFinish();

    private:

        QString key() const;
        QPixmap generatePixmap() const;
        qreal   infiniteAnimationValue() const;
        int     visibleValue() const;

    private:

        //Keep GUI responsive if progress is run from main thread
        bool                    m_bMainThread = false;

        //actual current value
        CProgressSignalHandler* m_pSignal = nullptr;
        int                     mValue;
        int                     mMaximum;
        qreal                   mInnerRadius;
        qreal                   mOuterRadius;
        QColor                  mColor;
        QColor                  m_colorBase;
        QColor                  m_colorBg = QColor(225,225,225);
        QColor                  m_colorInner = QColor(255,255,255);
        QString                 m_percentStr = "0";

        //value visible to user
        int                     mVisibleValue;
        QPropertyAnimation      mValueAnimation;

        QPropertyAnimation      mInfiniteAnimation;
        qreal                   mInfiniteAnimationValue;
};
