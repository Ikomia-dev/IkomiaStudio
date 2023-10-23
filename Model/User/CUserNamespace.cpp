#include "CUserNamespace.h"

CUserNamespace::CUserNamespace()
{
}

CUserNamespace::CUserNamespace(const QJsonObject &jsonNamespace)
{
    if (jsonNamespace.contains("url"))
        m_url = jsonNamespace["url"].toString();
    else
        throw CException(CoreExCode::INVALID_JSON_FORMAT, "Missing url in namespace JSON content", __func__, __FILE__, __LINE__);

    if (jsonNamespace.contains("name"))
        m_name = jsonNamespace["name"].toString();
    else
        throw CException(CoreExCode::INVALID_JSON_FORMAT, "Missing name in namespace JSON content", __func__, __FILE__, __LINE__);

    if (jsonNamespace.contains("path"))
        m_path = jsonNamespace["path"].toString();

    if (jsonNamespace.contains("visibility"))
        m_visibility = jsonNamespace["visibility"].toString();

    if (jsonNamespace.contains("children"))
    {
        QJsonArray children = jsonNamespace["children"].toArray();
        for (int i=0; i<children.size(); ++i)
            m_childrenUrl.push_back(children[i].toString());
    }
}
