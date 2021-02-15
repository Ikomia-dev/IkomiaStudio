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

#ifndef CWIDGETDATADISPLAY_H
#define CWIDGETDATADISPLAY_H

#include "CDataDisplay.h"

class CWidgetDataDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        CWidgetDataDisplay(QWidget* pWidget, QWidget* pParent = nullptr, bool bDeleteWidget = true, int flags=CDataDisplay::MAXIMIZE_BUTTON);
        ~CWidgetDataDisplay();

    protected:
        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();

        void            removeWidget();

    public slots:

        virtual void    onClose();

    private:

        QWidget*    m_pWidget = nullptr;
        bool        m_bDeleteWidget = true;
};

#endif // CWIDGETDATADISPLAY_H
