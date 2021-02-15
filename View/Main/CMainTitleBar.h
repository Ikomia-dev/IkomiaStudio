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

#ifndef CMAINTITLEBAR_H
#define CMAINTITLEBAR_H

/**
 * @file      CMainTitleBar.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CMainTitleBar definition
 *
 * @details   Details
 */

#include <QFrame>

class QHBoxLayout;
class QPushButton;

/**
 * @brief
 *
 */
class CMainTitleBar : public QFrame
{
    Q_OBJECT

    public:

        explicit CMainTitleBar(int size, QWidget* parent = Q_NULLPTR);

        void    setUser(const QString& name);

    signals:

        void    close();

        void    minimize();

        void    expand();

    private:

        void    initTitle();
        void    initButtons();
        void    initConnections();

    private:

        QLabel*         m_pLabelTitle = nullptr;
        QHBoxLayout*    m_pLayout = nullptr;
        QPushButton*    m_pClose = nullptr;
        QPushButton*    m_pMinimize = nullptr;
        QPushButton*    m_pExpand = nullptr;
        int             m_size = 20;
};

#endif // CMAINTITLEBAR_H
