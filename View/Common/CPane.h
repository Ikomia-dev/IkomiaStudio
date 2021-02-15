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

#ifndef CPANE_H
#define CPANE_H

/**
 * @file      CPane.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CPane definition
 *
 * @details   Details
 */

#include <QWidget>
#include <QVBoxLayout>
#include "Main/forwards.hpp"

/**
 * @brief
 *
 */
class CPane : public QWidget
{
    public:
        /**
         * @brief
         *
         * @param parent
         */
        CPane(QWidget* parent = Q_NULLPTR);

        void        setAnimation(QByteArray animation, int minValue, int maxValue);

        void        animate();

    protected:

        QVBoxLayout*        m_pLayout = nullptr;
        bool                m_bIsOpened = false;
        QByteArray          m_animationName = "";
        int                 m_animationMinValue = 0;
        int                 m_animationMaxValue = 0;
};

#endif // CPANE_H
