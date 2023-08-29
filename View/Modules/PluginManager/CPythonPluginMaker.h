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

#ifndef CPYTHONPLUGINMAKER_H
#define CPYTHONPLUGINMAKER_H

#include <QString>
#include "PluginManagerDefine.hpp"

class CPythonPluginMaker
{
    public:

        enum QtBinding
        {
            PYQT,
            PYSIDE
        };

        CPythonPluginMaker();

        void    setName(const QString& name);
        void    setProcessBaseClass(PluginManager::ProcessBaseClass base);
        void    setWidgetBaseClass(PluginManager::WidgetBaseClass base);
        void    setQtBinding(QtBinding binding);

        void    generate();

    private:

        void    createMainFile(const QString& folder);
        void    createProcessFile(const QString& folder);
        void    createWidgetFile(const QString &folder);
        void    createTestFile(const QString& folder);
        void    createReadmeFile(const QString& folder);
        void    createImageFolder(const QString& folder);

        QString getProcessBaseClass() const;
        QString getWidgetBaseClass() const;
        QString getQtBinding() const;
        QString getQtLayout() const;

    private:

        QString                         m_name;
        PluginManager::ProcessBaseClass m_processBaseClass = PluginManager::CWORKFLOW_TASK;
        PluginManager::WidgetBaseClass  m_widgetBaseClass = PluginManager::CWORKFLOW_TASK_WIDGET;
        QtBinding                       m_qtBinding = PYQT;
};

#endif // CPYTHONPLUGINMAKER_H
