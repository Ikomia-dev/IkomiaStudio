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

#ifndef CPLUGINPYTHONDEPENDENCYMODEL_H
#define CPLUGINPYTHONDEPENDENCYMODEL_H

#include <QStandardItemModel>

class CPluginPythonDependencyModel: public QStandardItemModel
{
    public:

        enum Role
        {
            StatusRole = Qt::UserRole + 1
        };

        CPluginPythonDependencyModel(QObject *parent = nullptr);

        void                setPluginName(const QString& name,
                                          const QMap<QString,QPair<QString,QString>>& allPackages,
                                          const QMap<QString,QString>& aliases);

        virtual QVariant    data(const QModelIndex &index, int role) const override;

    private:

        void                addModule(const QString& name, const QString version, const QString lastVersion, bool bMissing);
};

#endif // CPLUGINPYTHONDEPENDENCYMODEL_H
