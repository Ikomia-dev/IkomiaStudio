#ifndef CPLUGINMODEL_H
#define CPLUGINMODEL_H

#include "UtilsDefine.hpp"
#include "Model/Store/CStoreQueryModel.h"

class CPluginModel
{
    public:

        enum class Type : int
        {
            HUB,
            WORKSPACE,
            LOCAL
        };

        CPluginModel(Type type);
        ~CPluginModel();

        void                setType(Type type);
        void                setCurrentUser(const CUser &user);
        void                setCurrentIndex(const QModelIndex& index);
        void                setCurrentPluginId(int id);
        void                setQuery(const QString& query, const QSqlDatabase& db);
        void                setTotalPluginCount(int count);
        void                setPluginField(int index, const QString &key, const QString &value);

        Type                getType() const;
        CStoreQueryModel*   getModel() const;
        CUser               getCurrentUser() const;
        QModelIndex         getCurrentIndex() const;
        int                 getCurrentPluginId() const;
        int                 getIntegerField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        std::string         getStringField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        QString             getQStringField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        QJsonArray          getJsonPlugins() const;
        int                 getTotalPluginCount() const;
        ApiLanguage         getLanguageFromString(const QString strLanguage) const;

        bool                isComplete() const;

        void                init(const CUser &user, const QString& query, const QSqlDatabase& db);

        void                addJsonPlugin(const QJsonObject& jsonPlugin);

        void                filterCompatiblePlugins();

        void                clear();

    private:

        bool                checkOSCompatibility(const QJsonObject& plugin) const;
        bool                checkIkomiaCompatibility(const QJsonObject& plugin) const;
        bool                checkArchitecture(const QJsonObject& plugin) const;

    private:

        Type                m_type = Type::HUB;
        CStoreQueryModel*   m_pModel = nullptr;
        QModelIndex         m_currentIndex = QModelIndex();
        QJsonArray          m_jsonPlugins;
        CUser               m_user;
        int                 m_currentPluginId = -1;
        int                 m_totalPluginCount = 0;
};

#endif // CPLUGINMODEL_H
