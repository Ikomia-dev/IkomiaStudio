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
