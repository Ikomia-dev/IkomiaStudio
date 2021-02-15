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

#ifndef CGENERALSETTINGSWIDGET_H
#define CGENERALSETTINGSWIDGET_H

#include <QWidget>

class QCheckBox;

class CGeneralSettingsWidget : public QWidget
{
       Q_OBJECT

    public:

        explicit    CGeneralSettingsWidget(QWidget* parent = nullptr);

    signals:

        void        doEnableTutorialHelper(bool bEnable);
        void        doEnableNativeDialog(bool bEnable);

    public slots:

        void        onEnableTutorialHelper(bool bEnable);
        void        onEnableNativeDialog(bool bEnable);

    private:

        void        initLayout();
        void        initConnections();

    private:

        // Tutorials standby
        QCheckBox*  m_pCheckTuto = nullptr;
        QCheckBox*  m_pCheckNative = nullptr;
};

#endif // CGENERALSETTINGSWIDGET_H
