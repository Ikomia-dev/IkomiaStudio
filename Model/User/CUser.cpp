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

#include "CUser.h"

CUser::CUser()
{
}

CUser::~CUser()
{
}

bool CUser::operator==(const CUser &other) const
{
    return m_id == other.m_id &&
            m_role == other.m_role &&
            m_name == other.m_name &&
            m_firstName == other.m_firstName &&
            m_lastName == other.m_lastName &&
            m_email == other.m_email &&
            m_url == other.m_url &&
            m_namespaceUrl == other.m_namespaceUrl;
}

bool CUser::operator!=(const CUser &other) const
{
    return m_id != other.m_id ||
            m_role != other.m_role ||
            m_name != other.m_name ||
            m_firstName != other.m_firstName ||
            m_lastName != other.m_lastName ||
            m_email != other.m_email ||
            m_url != other.m_url ||
            m_namespaceUrl != other.m_namespaceUrl;
}

bool CUser::isConnected() const
{
    return !m_token.isEmpty();
}

QByteArray CUser::getAuthHeader() const
{
    return QString("Token %1").arg(m_token).toLocal8Bit();
}

CUserNamespace CUser::getNamespace(const QString &name) const
{
    for (size_t i=0; i<m_namespaces.size(); ++i)
    {
        if (m_namespaces[i].m_name == name)
            return m_namespaces[i];
    }

    QString msg = QString("Namespace %1 not found for user %2").arg(name).arg(m_name);
    throw CException(CoreExCode::NOT_FOUND, msg.toStdString(), __func__, __FILE__, __LINE__);
}

QString CUser::getMyNamespaceUrl() const
{
    return m_namespaceUrl;
}

std::vector<QString> CUser::getNamespaceNames() const
{
    std::vector<QString> names;
    for (size_t i=0; i<m_namespaces.size(); ++i)
        names.push_back(m_namespaces[i].m_name);

    return names;
}

void CUser::logout()
{
    m_id = -1;
    m_role = -1;
    m_reputation = 0;
    m_name.clear();
    m_firstName.clear();
    m_lastName.clear();
    m_email.clear();
    m_token.clear();
    m_url.clear();
    m_namespaces.clear();
}

void CUser::addNamespace(const QJsonObject &ns)
{
    m_namespaces.push_back(CUserNamespace(ns));
}
