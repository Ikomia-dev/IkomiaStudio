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

#ifndef CGRAPHICSTEXTPROPWIDGET_H
#define CGRAPHICSTEXTPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsTextProperty;

class CGraphicsTextPropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsTextPropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsTextProperty* pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createToolButton(const QString& title, const QIcon& icon, bool bCheckable);

        void            updateUI();

    private:

        CColorPushButton*       m_pColorBtn = nullptr;
        QPushButton*            m_pBoldBtn = nullptr;
        QPushButton*            m_pItalicBtn = nullptr;
        QPushButton*            m_pUnderlineBtn = nullptr;
        QPushButton*            m_pStrikeOutBtn = nullptr;
        QFontComboBox*          m_pComboFont = nullptr;
        QSpinBox*               m_pSpinFontSize = nullptr;
        GraphicsTextProperty*   m_pProperty = nullptr;
};

#endif // CGRAPHICSTEXTPROPWIDGET_H
