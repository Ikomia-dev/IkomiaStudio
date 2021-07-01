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

#ifndef CGRAPHICSPOLYLINEPROPWIDGET_H
#define CGRAPHICSPOLYLINEPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct CGraphicsPolylineProperty;

class CGraphicsPolylinePropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsPolylinePropWidget(QWidget *parent = nullptr);

        void            setProperties(CGraphicsPolylineProperty* pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        void            updateUI();

    private:

        CColorPushButton*           m_pPenColorBtn = nullptr;
        QSpinBox*                   m_pSpinSize = nullptr;
        QSpinBox*                   m_pSpinOpacity = nullptr;
        QLineEdit*                  m_pEditCategory = nullptr;
        CGraphicsPolylineProperty*   m_pProperty = nullptr;
};

#endif // CGRAPHICSPOLYLINEPROPWIDGET_H
