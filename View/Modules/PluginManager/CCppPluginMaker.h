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

#ifndef CCPPPLUGINMAKER_H
#define CCPPPLUGINMAKER_H

#include "PluginManagerDefine.hpp"

class CCppPluginMaker
{
    public:

        CCppPluginMaker();

        void    setName(const QString& name);
        void    setSrcFolder(const QString& folder);
        void    setApiFolder(const QString& folder);
        void    setProcessBaseClass(PluginManager::ProcessBaseClass base);
        void    setWidgetBaseClass(PluginManager::WidgetBaseClass base);

        void    generate();

    private:

        void    createProjectFile();
        void    createGlobalHeader();
        void    createCoreHeader();
        void    createCoreSource();

        QString getProcessBaseClass() const;
        QString getProcessBaseClassHeader() const;
        QString getWidgetBaseClass() const;
        QString getWidgetBaseClassHeader() const;

    private:

        QString                         m_name;
        QString                         m_srcFolder;
        QString                         m_apiFolder;
        PluginManager::ProcessBaseClass   m_processBaseClass = PluginManager::CWorkflow_TASK;
        PluginManager::WidgetBaseClass    m_widgetBaseClass = PluginManager::CWorkflow_TASK_WIDGET;
};

#endif // CCPPPLUGINMAKER_H
