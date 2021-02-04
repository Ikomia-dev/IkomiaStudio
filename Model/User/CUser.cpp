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
            m_token == other.m_token;
}

bool CUser::operator!=(const CUser &other) const
{
    return m_id != other.m_id ||
            m_role != other.m_role ||
            m_name != other.m_name ||
            m_firstName != other.m_firstName ||
            m_lastName != other.m_lastName ||
            m_email != other.m_email ||
            m_token != other.m_token;
}

bool CUser::isConnected() const
{
    return !m_token.isEmpty();
}

void CUser::logout()
{
    m_id = -1;
    m_role = -1;
    m_name.clear();
    m_firstName.clear();
    m_lastName.clear();
    m_email.clear();
    m_token.clear();
}
