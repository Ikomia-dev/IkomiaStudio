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

#ifndef CPLUGINQUERYMODEL_H
#define CPLUGINQUERYMODEL_H

#include <QStandardItemModel>

class CPluginPythonModel : public QStandardItemModel
{
    public:

        enum Role
        {
            StatusRole = Qt::UserRole + 1
        };

        CPluginPythonModel(QObject *parent = Q_NULLPTR);

        void                update();

        virtual QVariant    data(const QModelIndex &index, int role) const override;

    private:

        void                initHeaderLabels();

        void                fill();
        void                fillLoadedPlugins();
        void                fillUnloadedPlugins();
        QStandardItem*      addItem(int row, int col, const QString& data);

    private:

        QSet<QString>   m_loadedPluginsFolder;
};

#endif // CPLUGINQUERYMODEL_H
