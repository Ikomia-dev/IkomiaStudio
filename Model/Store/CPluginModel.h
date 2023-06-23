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
        void                setCurrentWorkspace(const QString& workspace);
        void                setCurrentRequestUrl(const QString& url);
        void                setQuery(const QString& query, const QSqlDatabase& db);
        void                setTotalPluginCount(int count);
        void                setPluginField(int index, const QString &key, const QString &value);
        void                setPackageFile(const QString& file);

        Type                getType() const;
        CStoreQueryModel*   getModel() const;
        CUser               getCurrentUser() const;
        QModelIndex         getCurrentIndex() const;
        QString             getCurrentWorkspace() const;
        QString             getCurrentRequestUrl() const;
        int                 getIntegerField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        std::string         getStringField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        QString             getQStringField(const QString &fieldName, const QModelIndex& index=QModelIndex()) const;
        QJsonArray          getJsonPlugins() const;
        QJsonObject         getJsonPlugin(const QString& name) const;
        int                 getTotalPluginCount() const;
        ApiLanguage         getLanguageFromString(const QString strLanguage) const;
        QString             getPackageFile() const;

        bool                isComplete() const;
        bool                isPluginExists(const QString& name) const;

        void                init(const CUser &user, const QString& query, const QSqlDatabase& db);

        void                addJsonPlugin(const QJsonObject& jsonPlugin);

        void                filterCompatiblePlugins();

        void                clear();
        void                clearContext();

    private:

        void                updatePluginPackagesInfo(int index);

        bool                checkPackageCompatibility(const QJsonObject& package, ApiLanguage language) const;
        bool                checkOSCompatibility(const QJsonObject& package) const;
        bool                checkIkomiaCompatibility(const QJsonObject& package, ApiLanguage language) const;
        bool                checkPythonCompatibility(const QJsonObject& package) const;
        bool                checkArchitecture(const QJsonObject& package, ApiLanguage language) const;

    private:

        Type                m_type = Type::HUB;
        CStoreQueryModel*   m_pModel = nullptr;
        QModelIndex         m_currentIndex = QModelIndex();
        QJsonArray          m_jsonPlugins;
        CUser               m_user;
        QString             m_currentWorkspace;
        QString             m_currentRequestUrl;
        QString             m_packageFile;
        int                 m_totalPluginCount = 0;
};

#endif // CPLUGINMODEL_H
