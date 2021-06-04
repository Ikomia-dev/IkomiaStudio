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

#ifndef CPREFERENCESDLG_H
#define CPREFERENCESDLG_H

#include "View/Common/CDialog.h"
#include "CGeneralSettingsWidget.h"

class FancyTabBar;
class CUserManagementWidget;
class QSqlQueryModel;
class CWorkflowSettingsWidget;

// Not use for the moment
class CPreferencesDlg : public CDialog
{
    Q_OBJECT

    public:

        CPreferencesDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        CGeneralSettingsWidget*     getGeneralSettings() const;
        CWorkflowSettingsWidget*    getWorkflowSettings() const;

    private:

        void                        initLayout();
        void                        initConnections();

    private:

        FancyTabBar*                m_pTabBar = nullptr;
        QStackedWidget*             m_pStackWidget = nullptr;
        CUserManagementWidget*      m_pUserManagementWidget = nullptr;
        CGeneralSettingsWidget*     m_pGeneralSettingsWidget = nullptr;
        CWorkflowSettingsWidget*    m_pWorkflowSettingsWidget = nullptr;
};

#endif // CPREFERENCESDLG_H
