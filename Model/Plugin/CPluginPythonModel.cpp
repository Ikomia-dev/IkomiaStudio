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

#include "CPluginPythonModel.h"
#include <QSqlDatabase>
#include "Main/AppTools.hpp"
#include "Core/CPluginTools.h"

CPluginPythonModel::CPluginPythonModel(QObject *parent) : QStandardItemModel(parent)
{
}

void CPluginPythonModel::update()
{
    clear();
    m_loadedPluginsFolder.clear();
    initHeaderLabels();
    fill();
}

QVariant CPluginPythonModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::ForegroundRole)
    {
        bool bMissing = data(index, StatusRole).toBool();
        if(bMissing == true)
            return QVariant(QColor(Qt::red));
    }
    return QStandardItemModel::data(index, role);
}

void CPluginPythonModel::initHeaderLabels()
{
    QStringList labels = {tr("Plugin name"), tr("Authors"), tr("Modified"), tr("Version")};
    setHorizontalHeaderLabels(labels);
}

void CPluginPythonModel::fill()
{
    fillLoadedPlugins();
    fillUnloadedPlugins();
}

void CPluginPythonModel::fillLoadedPlugins()
{
    QSqlDatabase db = Utils::Database::connect(":memory:", Utils::Database::getProcessConnectionName());
    if(db.isValid() == false)
        throw CException(DatabaseExCode::INVALID_DB_CONNECTION, db.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    QSqlQuery q(db);
    if(!q.exec("SELECT * FROM process WHERE isInternal=False AND language=1;"))
        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

    int i=0;
    while(q.next())
    {
        QString name = q.value("name").toString();
        addItem(i, 0, name);
        QString authors = q.value("authors").toString();
        addItem(i, 1, authors);
        QString modified = q.value("modifiedDate").toString();
        addItem(i, 2, modified);
        QString version = q.value("version").toString();
        addItem(i, 3, version);
        m_loadedPluginsFolder.insert(QString::fromStdString(Utils::CPluginTools::getPythonPluginFolder(name.toStdString())));
        i++;
    }
}

void CPluginPythonModel::fillUnloadedPlugins()
{
    int row = rowCount();
    QDir pluginsDir(QString::fromStdString(Utils::Plugin::getPythonPath()));

    foreach (QString directory, pluginsDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot))
    {
        if(m_loadedPluginsFolder.contains(pluginsDir.absoluteFilePath(directory)) == false)
        {
            auto pItem = addItem(row, 0, directory);
            pItem->setData(true, StatusRole);
            row++;
        }
    }
}

QStandardItem* CPluginPythonModel::addItem(int row, int col, const QString &data)
{
    QStandardItem* pItem = new QStandardItem(data);
    setItem(row, col, pItem);
    return pItem;
}
