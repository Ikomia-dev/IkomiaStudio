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

#ifndef CSTACKEDPANE_H
#define CSTACKEDPANE_H

#include "Main/forwards.hpp"
#include "CPane.h"

class CStackedPane : public CPane
{
    public:

        CStackedPane(QWidget* parent = Q_NULLPTR);

        int     addPane(QWidget* pane);

        void    togglePane(QWidget* pane);

        void    showPane(QWidget* pPane);

    private:

        QStackedWidget* m_pWidgets;
};

#endif // CSTACKEDPANE_H
