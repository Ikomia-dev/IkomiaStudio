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

/////////////////////////////////////////////////////////////////////////////
// Date of creation: 04.07.2013
// Creator: Alexander Egorov aka mofr
// Authors: mofr
/////////////////////////////////////////////////////////////////////////////

#include "CProgressCircle.h"
#include "CProgressSignalHandler.h"
#include "View/Main/CResponsiveUI.h"
#include <QPainter>
#include <QPixmapCache>
#include <QFontMetrics>
#include <QParallelAnimationGroup>
#include <QTimer>

CProgressCircle::CProgressCircle(CProgressSignalHandler* pSignal, bool bMainThread, QWidget *parent) :
    QWidget(parent),
    mValue(0),
    mMaximum(0),
    mInnerRadius(0.5),
    mOuterRadius(1.0),
    mColor(110,190,235),
    mVisibleValue(0),
    mValueAnimation(this, "visibleValue"),
    mInfiniteAnimation(this, "infiniteAnimationValue"),
    mInfiniteAnimationValue(0.0)
{
    m_bMainThread = bMainThread;
    m_pSignal = pSignal;

    if(m_pSignal)
    {
        connect(m_pSignal, &CProgressSignalHandler::doSetTotalSteps, this, &CProgressCircle::onSetTotalSteps);
        connect(m_pSignal, &CProgressSignalHandler::doAddSubTotalSteps, this, &CProgressCircle::onAddSubTotalSteps);
        connect(m_pSignal, &CProgressSignalHandler::doProgress, this, &CProgressCircle::onProgress);
        connect(m_pSignal, &CProgressSignalHandler::doSetValue, this, &CProgressCircle::setValue);
        connect(m_pSignal, &CProgressSignalHandler::doSetMessage, this, &CProgressCircle::onSetMessage);
        connect(m_pSignal, &CProgressSignalHandler::doFinish, this, &CProgressCircle::onFinish);
    }
}

CProgressCircle::~CProgressCircle()
{
}

QSize CProgressCircle::sizeHint() const
{
    return QSize(64,64);
}

int CProgressCircle::value() const
{
    return mValue;
}

int CProgressCircle::maximum() const
{
    return mMaximum;
}

qreal CProgressCircle::innerRadius() const
{
    return mInnerRadius;
}

qreal CProgressCircle::outerRadius() const
{
    return mOuterRadius;
}

QColor CProgressCircle::color() const
{
    return mColor;
}

void CProgressCircle::reset()
{
    mValue = 0;
    mMaximum = 0;
    mInnerRadius = 0.6;
    mOuterRadius = 1.0;
    mVisibleValue = 0;
    mColor = m_colorBase;
    m_percentStr = "0";
    update();
}

void CProgressCircle::start()
{
    QPropertyAnimation* progressCircleAnimation = new QPropertyAnimation(this, "outerRadius");
    progressCircleAnimation->setDuration(100);
    progressCircleAnimation->setEasingCurve(QEasingCurve::OutQuad);
    progressCircleAnimation->setStartValue(0.0);
    progressCircleAnimation->setEndValue(1.0);

    QTimer::singleShot(0, progressCircleAnimation, SLOT(start(QAbstractAnimation::DeleteWhenStopped)));
}

void CProgressCircle::finish(bool bSuccess)
{
    QParallelAnimationGroup *pGroup = new QParallelAnimationGroup(this);

    QPropertyAnimation* pAnimOuterRadius = new QPropertyAnimation(this, "outerRadius");
    pAnimOuterRadius->setDuration(1500);
    pAnimOuterRadius->setEasingCurve(QEasingCurve::OutQuad);
    pAnimOuterRadius->setEndValue(0.5);

    QPropertyAnimation* pAnimInnerRadius = new QPropertyAnimation(this, "innerRadius");
    pAnimInnerRadius->setDuration(750);
    pAnimInnerRadius->setEasingCurve(QEasingCurve::OutQuad);
    pAnimInnerRadius->setEndValue(0.0);

    QColor color = bSuccess ? QColor(155,219,58,100) : QColor(255,100,100,100);

    QPropertyAnimation* pAnimColor = new QPropertyAnimation(this, "color");
    pAnimColor->setDuration(750);
    pAnimColor->setEasingCurve(QEasingCurve::OutQuad);
    pAnimColor->setEndValue(color);

    pGroup->addAnimation(pAnimOuterRadius);
    pGroup->addAnimation(pAnimInnerRadius);
    pGroup->addAnimation(pAnimColor);

    connect(pGroup, &QParallelAnimationGroup::finished, [this, bSuccess]
    {
        if(bSuccess)
            emit doFinish();
        else
            emit doAbort();
    });

    if(m_pSignal)
    {
        disconnect(m_pSignal, &CProgressSignalHandler::doSetTotalSteps, this, &CProgressCircle::onSetTotalSteps);
        disconnect(m_pSignal, &CProgressSignalHandler::doAddSubTotalSteps, this, &CProgressCircle::onAddSubTotalSteps);
        disconnect(m_pSignal, &CProgressSignalHandler::doProgress, this, &CProgressCircle::onProgress);
        disconnect(m_pSignal, &CProgressSignalHandler::doSetValue, this, &CProgressCircle::setValue);
        disconnect(m_pSignal, &CProgressSignalHandler::doSetMessage, this, &CProgressCircle::onSetMessage);
        disconnect(m_pSignal, &CProgressSignalHandler::doFinish, this, &CProgressCircle::onFinish);
    }

    QTimer::singleShot(0, pGroup, SLOT(start(QAbstractAnimation::DeleteWhenStopped)));
}

void CProgressCircle::finishInfinite(bool bSuccess)
{
    stop();
    finish(bSuccess);
}

void CProgressCircle::startInfinite()
{
    mMaximum = 0;
    m_percentStr.clear();
    mInfiniteAnimationValue = 0.0;
    mInfiniteAnimation.setLoopCount(-1);//infinite
    mInfiniteAnimation.setDuration(1000);
    mInfiniteAnimation.setStartValue(0.0);
    mInfiniteAnimation.setEndValue(1.0);

    QTimer::singleShot(0, &mInfiniteAnimation, SLOT(start()));
}

void CProgressCircle::stop()
{
    mMaximum = 1;
    mVisibleValue = 1;
    mInfiniteAnimation.stop();
    update();
}

CProgressSignalHandler *CProgressCircle::getSignal()
{
    return m_pSignal;
}

void CProgressCircle::onProgress()
{
    if(mMaximum == 0)
        return;

    mValue++;

    int duration = 1000 / mMaximum;
    setVisibleValue(mValue);
    mValueAnimation.stop();
    mValueAnimation.setEndValue(mValue);
    mValueAnimation.setDuration(duration);
    mValueAnimation.start();

    setPercent(mValue*100/mMaximum);

    emit valueChanged(mValue);

    if(mValue == mMaximum)
        finish(true);

    if(m_bMainThread)
        CResponsiveUI::processEvents();
}

void CProgressCircle::onAbort()
{
    emit doDisconnectAbort();
    finish(false);
}

void CProgressCircle::onSetTotalSteps(size_t maxSteps)
{
    setMaximum((int)maxSteps);
    start();
}

void CProgressCircle::onAddSubTotalSteps(int count)
{
    int newMax = mMaximum + count;
    setMaximum(newMax);
}

void CProgressCircle::onFinishInfinite()
{
    finishInfinite(true);
}

void CProgressCircle::setValue(int value)
{
    if(mMaximum == 0)
        return;

    if(value < 0)
        value = 0;

    if(mValue != value)
    {
        int duration = 1000 / mMaximum;
        setVisibleValue(value);
        mValueAnimation.stop();
        mValueAnimation.setEndValue(value);
        mValueAnimation.setDuration(duration);
        mValueAnimation.start();

        setPercent(value*100/mMaximum);
        mValue = value;
        emit valueChanged(value);

        if(mValue == mMaximum)
            finish(true);
    }
}

void CProgressCircle::setMaximum(int maximum)
{
    if(maximum < 0)
        maximum = 0;

    if(mMaximum != maximum)
    {
        mMaximum = maximum;
        update();
        emit maximumChanged(maximum);

        if(mMaximum == 0)
            mInfiniteAnimation.start();
        else
            mInfiniteAnimation.stop();
    }
}

void CProgressCircle::setInnerRadius(qreal innerRadius)
{
    if(innerRadius > 1.0) innerRadius = 1.0;
    if(innerRadius < 0.0) innerRadius = 0.0;

    if(mInnerRadius != innerRadius)
    {
        mInnerRadius = innerRadius;
        update();
    }
}

void CProgressCircle::setOuterRadius(qreal outerRadius)
{
    if(outerRadius > 1.0) outerRadius = 1.0;
    if(outerRadius < 0.0) outerRadius = 0.0;

    if(mOuterRadius != outerRadius)
    {
        mOuterRadius = outerRadius;
        update();
    }
}

void CProgressCircle::setColor(QColor color)
{
    if(color != mColor)
    {
        mColor = color;
        update();
    }
}

void CProgressCircle::setColorBase(QColor color)
{
    m_colorBase = color;
    mColor = color;
    update();
}

void CProgressCircle::setColorBg(QColor color)
{
    if(color != m_colorBg)
    {
        m_colorBg = color;
        update();
    }
}

void CProgressCircle::setColorInner(QColor color)
{
    if(color != m_colorInner)
    {
        m_colorInner = color;
        update();
    }
}

void CProgressCircle::setPercent(int percent)
{
    m_percentStr = QString::number(percent);
}

QRectF squared(QRectF rect)
{
    if(rect.width() > rect.height())
    {
        qreal diff = rect.width() - rect.height();
        return rect.adjusted(diff/2, 0, -diff/2, 0);
    }
    else
    {
        qreal diff = rect.height() - rect.width();
        return rect.adjusted(0, diff/2, 0, -diff/2);
    }
}

void CProgressCircle::paintEvent(QPaintEvent *)
{
    QPixmap pixmap;
    if (!QPixmapCache::find(key(), &pixmap))
    {
        pixmap = generatePixmap();
        QPixmapCache::insert(key(), pixmap);
    }

    // Draw pixmap at center of item
    QPainter painter(this);
    painter.drawPixmap( 0.5 * ( width() - pixmap.width() ), 0.5 * ( height() - pixmap.height() ), pixmap );
}

void CProgressCircle::setInfiniteAnimationValue(qreal value)
{
    mInfiniteAnimationValue = value;
    update();
}

void CProgressCircle::setVisibleValue(int value)
{
    if(mVisibleValue != value)
    {
        mVisibleValue = value;
        update();
    }
}

void CProgressCircle::onSetMessage(const QString &msg)
{
    emit doSetMessage(msg);
}

void CProgressCircle::onFinish()
{
    finish(true);
}

QString CProgressCircle::key() const
{
    return QString("%1,%2,%3,%4,%5,%6,%7,%8")
            .arg(mInfiniteAnimationValue)
            .arg(mVisibleValue)
            .arg(mMaximum)
            .arg(mInnerRadius)
            .arg(mOuterRadius)
            .arg(width())
            .arg(height())
            .arg(mColor.rgb())
            ;
}

QPixmap CProgressCircle::generatePixmap() const
{
    QPixmap pixmap(squared(rect()).size().toSize());
    pixmap.fill(QColor(0,0,0,0));
    QPainter painter(&pixmap);

    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF rect = pixmap.rect().adjusted(1,1,-1,-1);
    qreal margin = rect.width()*(1.0 - mOuterRadius)/2.0;
    rect.adjust(margin,margin,-margin,-margin);
    qreal innerRadius = mInnerRadius*rect.width()/2.0;

    //background grey circle
    painter.setBrush(m_colorBg);
    painter.setPen(Qt::transparent);
    painter.drawPie(rect, 0, 360*16);

    painter.setBrush(mColor);
    painter.setPen(Qt::transparent);

    if(mMaximum == 0)
    {
        //draw as infinite process
        int startAngle = -mInfiniteAnimationValue * 360 * 16;
        int spanAngle = 0.15 * 360 * 16;
        painter.drawPie(rect, startAngle, spanAngle);
    }
    else
    {
        int value = qMin(mVisibleValue, mMaximum);
        int startAngle = 90 * 16;
        int spanAngle = -qreal(value) * 360 * 16 / mMaximum;
        painter.drawPie(rect, startAngle, spanAngle);
    }

    //inner circle and frame
    painter.setBrush(m_colorInner);
    painter.setPen(Qt::transparent);
    painter.drawEllipse(rect.center(), innerRadius, innerRadius);

    //outer frame
    painter.drawArc(rect, 0, 360*16);

    painter.setBrush(m_colorBase);
    painter.setPen(m_colorBase);
    auto ft = painter.font();
    ft.setBold(true);    
    auto r = rect.center();
    QRect rectInner = QRect(r.x()-innerRadius, r.y()-innerRadius, innerRadius*2, innerRadius*2);

    float factor = (rectInner.height() - 5) / painter.fontMetrics().height();
    if ((factor < 1 && factor > 0) || (factor > 1.25))
    {
        ft.setPointSizeF(ft.pointSizeF()*factor);
    }

    painter.setFont(ft);
    painter.drawText(rectInner, Qt::AlignCenter, m_percentStr);

    return pixmap;
}

qreal CProgressCircle::infiniteAnimationValue() const
{
    return mInfiniteAnimationValue;
}

int CProgressCircle::visibleValue() const
{
    return mVisibleValue;
}
