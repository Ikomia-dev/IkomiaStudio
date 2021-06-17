// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CPluginPythonDependencyModel.h"
#include "Main/LogCategory.h"
#include "Main/AppTools.hpp"
#include "PythonThread.hpp"
#include "CPluginTools.h"

CPluginPythonDependencyModel::CPluginPythonDependencyModel(QObject *parent)
    :QStandardItemModel(parent)
{
}

void CPluginPythonDependencyModel::setPluginName(const QString &name,
                                                 const QMap<QString,QPair<QString,QString>>& allPackages,
                                                 const QMap<QString, QString> &aliases)
{
    clear();
    QStringList labels = {tr("Packages"), tr("Version"), tr("Last version")};
    setHorizontalHeaderLabels(labels);

    try
    {
        CPyEnsureGIL gil;
        QString pluginFolder = Utils::CPluginTools::getPythonPluginFolder(name);
        boost::python::str strPluginFolder(pluginFolder.toStdString().c_str());
        boost::python::object ikomia = Utils::CPluginTools::loadPythonModule("ikomia");
        boost::python::object utils = ikomia.attr("utils");
        boost::python::object modules = utils.attr("get_plugin_dependencies")(strPluginFolder);
        boost::python::object validModules = modules[0];
        boost::python::object missingModules = modules[1];

        for(int i=0; i<boost::python::len(missingModules); ++i)
        {
            QString name = QString::fromStdString(boost::python::extract<std::string>(missingModules[i]));
            addModule(name, "", "", true);
        }

        for(int i=0; i<boost::python::len(validModules); ++i)
        {
            QString name = QString::fromStdString(boost::python::extract<std::string>(validModules[i]));
            QString alias = name;

            auto it = aliases.find(name);
            if(it != aliases.end())
                alias = it.value();

            QString version = allPackages[alias].first;
            QString lastVersion = allPackages[alias].second;
            addModule(name, version, lastVersion, false);
        }
    }
    catch (boost::python::error_already_set&)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
    }
}

void CPluginPythonDependencyModel::addModule(const QString &name, const QString version, const QString lastVersion, bool bMissing)
{
    int row = rowCount();
    QStandardItem* pItem = new QStandardItem(name);
    pItem->setData(bMissing, StatusRole);
    setItem(row, 0, pItem);

    QStandardItem* pItemVersion = new QStandardItem(version);
    setItem(row, 1, pItemVersion);

    QStandardItem* pItemLastVersion = new QStandardItem(lastVersion);
    setItem(row, 2, pItemLastVersion);
}

QVariant CPluginPythonDependencyModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::ForegroundRole)
    {
        bool bMissing = data(index, StatusRole).toBool();
        if(bMissing == true)
            return QVariant(QColor(Qt::red));
    }
    return QStandardItemModel::data(index, role);
}
