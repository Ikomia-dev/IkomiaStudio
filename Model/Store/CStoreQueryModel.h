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

#ifndef CSTORETABLEMODEL_H
#define CSTORETABLEMODEL_H

#include <QSqlQueryModel>
#include "Model/User/CUser.h"

class CStoreQueryModel : public QSqlQueryModel
{
    public:

        CStoreQueryModel(QObject *parent = Q_NULLPTR);

        QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void        setCurrentUser(const CUser& user);

        CUser       getCurrentUser() const;

    private:

        CUser       m_currentUser;
};

#endif // CSTORETABLEMODEL_H
