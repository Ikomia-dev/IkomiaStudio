#ifndef CUSERNAMESPACE_H
#define CUSERNAMESPACE_H

class CUserNamespace
{
    public:

        CUserNamespace();
        CUserNamespace(const QJsonObject& jsonNamespace);

    public:

        QString                 m_url;
        QString                 m_name;
        QString                 m_path;
        QString                 m_visibility;
        std::vector<QString>    m_childrenUrl;
};

#endif // CUSERNAMESPACE_H
