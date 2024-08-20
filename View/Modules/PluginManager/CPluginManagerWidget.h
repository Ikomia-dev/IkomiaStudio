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

#ifndef CPLUGINMAKERWIDGET_H
#define CPLUGINMAKERWIDGET_H

#include <QWidget>
#include "CPythonPluginManagerWidget.h"
#include "CCppNewPluginWidget.h"

class FancyTabBar;
class CProgressCircle;

class CPluginManagerWidget : public QWidget
{
    Q_OBJECT

    public:

        CPluginManagerWidget(QWidget* parent = nullptr);

        void    setPythonQueryModel(CPluginPythonModel* pModel);
        void    setPythonDependencyModel(CPluginPythonDependencyModel* pModel);

    signals:

        void    doNewNotification(const QString text, Notification type, CProgressCircle* pItem, int duration);
        void    doGetPythonQueryModel();
        void    doGetPythonDependencyModel(const QString& pluginName);
        void    doReload(const QString& pluginName, int language);
        void    doReloadAll();
        void    doEditPythonPlugin(const QString& pluginName);
        void    doShowLocation(const QString& pluginName, int language);
        void    doInstallPythonPkg(const QString& moduleName);
        void    doUpdatePythonPkg(const QString& moduleName);
        void    doInstallRequirements(const QString& pluginName);

    private:

        void    initLayout();
        void    initConnections();

    private:

        FancyTabBar*                m_pTabBar = nullptr;
        QStackedWidget*             m_pStackWidget = nullptr;
        CPythonPluginManagerWidget* m_pPythonWidget = nullptr;
        CCppNewPluginWidget*        m_pCppWidget = nullptr;
};

#endif // CPLUGINMAKERWIDGET_H
