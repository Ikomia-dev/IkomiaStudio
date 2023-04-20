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
            m_namespace == other.m_namespace;
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
            m_namespace != other.m_namespace;
}

bool CUser::isConnected() const
{
    return m_sessionCookies.size() > 0;
}

QByteArray CUser::getSessionCookie(const QString &name)
{
    for (int i=0; i<m_sessionCookies.size(); ++i)
    {
        if (m_sessionCookies[i].name() == name)
            return m_sessionCookies[i].value();
    }
    return nullptr;
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
    m_namespace.clear();
    m_sessionCookies.clear();
}
