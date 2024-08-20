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

#ifndef CPYTHONPLUGINMANAGERWIDGET_H
#define CPYTHONPLUGINMANAGERWIDGET_H

#include <QWidget>

class CPluginPythonModel;
class CPluginPythonDependencyModel;

class CPythonPluginManagerWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CPythonPluginManagerWidget(QWidget *parent = nullptr);

        void            setModel(CPluginPythonModel* pModel);
        void            setDependencyModel(CPluginPythonDependencyModel* pModel);

    signals:

        void            doPluginCreated(const QString& pluginName);
        void            doReload(const QString& pluginName);
        void            doReloadAll();
        void            doEdit(const QString& pluginName);
        void            doShowLocation(const QString& pluginName);
        void            doGetPluginDependencyModel(const QString& pluginName);
        void            doInstall(const QString& moduleName);
        void            doUpdate(const QString& moduleName);
        void            doInstallRequirements(const QString& pluginName);

    private slots:

        void            onNewPlugin();
        void            onReloadPlugin();
        void            onEditPlugin();
        void            onShowLocation();
        void            onPluginSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
        void            onInstallDependency();
        void            onUpdateDependency();
        void            onInstallRequirements();

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QIcon& icon, const QString& tooltip);

        void            adjustTableSize(QTableView *pTableView);

    private:

        QTableView*         m_pPluginsView = nullptr;
        QTableView*         m_pDependenciesView = nullptr;
        QPushButton*        m_pNewBtn = nullptr;
        QPushButton*        m_pReloadAllBtn = nullptr;
        QPushButton*        m_pReloadBtn = nullptr;
        QPushButton*        m_pEditBtn = nullptr;
        QPushButton*        m_pShowLocationBtn = nullptr;
        QPushButton*        m_pInstallPkgBtn = nullptr;
        QPushButton*        m_pUpdatePkgBtn = nullptr;
        QPushButton*        m_pInstallBtn = nullptr;

};

#endif // CPYTHONPLUGINMANAGERWIDGET_H
