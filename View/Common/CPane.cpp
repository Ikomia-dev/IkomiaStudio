/**
 * @file      CPane.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CPane
 *
 * @details   Details
 */

#include "CPane.h"
#include <QPropertyAnimation>

CPane::CPane(QWidget* parent) : QWidget(parent)
{
    m_pLayout = new QVBoxLayout;
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pLayout);
    hide();
}

void CPane::setAnimation(QByteArray animation, int minValue, int maxValue)
{
    m_animationName = animation;
    m_animationMinValue = minValue;
    m_animationMaxValue = maxValue;
}

void CPane::animate()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, m_animationName);
    animation->setDuration(500);

    if(m_bIsOpened == true)
    {
        animation->setStartValue(m_animationMaxValue);
        animation->setEndValue(m_animationMinValue);
        m_bIsOpened = false;
    }
    else
    {
        show();
        animation->setStartValue(m_animationMinValue);
        animation->setEndValue(m_animationMaxValue);
        m_bIsOpened = true;
    }

    connect(animation, &QPropertyAnimation::finished, [&]
    {
        if(m_bIsOpened == false)
            this->hide();
    });

    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}
