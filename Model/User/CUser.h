#ifndef CUSER_H
#define CUSER_H

#include <QString>

class CUser
{
    public:

        CUser();
        ~CUser();

        bool    operator==(const CUser& other) const;
        bool    operator!=(const CUser& other) const;

        bool    isConnected() const;

        void    logout();

    public:

        int         m_id = -1;
        int         m_role = -1;
        int         m_reputation = 0;
        QString     m_name = "";
        QString     m_firstName = "";
        QString     m_lastName = "";
        QString     m_email = "";
        QString     m_token = "";
};

#endif // CUSER_H
