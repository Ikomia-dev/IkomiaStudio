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

#ifndef CUSER_H
#define CUSER_H

#include <QString>
#include <QNetworkCookie>
#include <QByteArray>
#include "CUserNamespace.h"

class CUser
{
    public:

        CUser();
        ~CUser();

        bool                    operator==(const CUser& other) const;
        bool                    operator!=(const CUser& other) const;

        bool                    isConnected() const;

        QByteArray              getAuthHeader() const;
        CUserNamespace          getNamespace(const QString& name) const;
        QString                 getMyNamespaceUrl() const;
        std::vector<QString>    getNamespaceNames() const;

        void                    logout();

        void                    addNamespace(const QJsonObject& ns);

    public:

        int                         m_id = -1;
        int                         m_role = -1;
        int                         m_reputation = 0;
        QString                     m_name;
        QString                     m_firstName;
        QString                     m_lastName;
        QString                     m_email;
        QString                     m_url;
        QString                     m_namespaceUrl;
        QString                     m_token;
        std::vector<CUserNamespace> m_namespaces;
};

#endif // CUSER_H
