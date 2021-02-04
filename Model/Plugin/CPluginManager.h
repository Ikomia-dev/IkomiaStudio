#ifndef CPLUGINMANAGER_H
#define CPLUGINMANAGER_H

#include <vector>
#include <QObject>
#include <QDir>
#include <QPluginLoader>
#include "CPluginTools.h"
#include "CPluginPythonModel.h"
#include "CPluginPythonDependencyModel.h"
#include "Model/User/CUser.h"

class CProcessRegistration;
class CProgressCircle;

class CPluginManager : public QObject
{
    Q_OBJECT

    public:

        CPluginManager();

        void                loadProcessPlugins();
        ProcessFactoryPtr   loadProcessPlugin(const QString& name, int language);
        ProcessFactoryPtr   loadCppProcessPlugin(const QString& fileName);
        ProcessFactoryPtr   loadPythonProcessPlugin(const QString& directory);

        void                setRegistrator(CProcessRegistration* pRegistrator);
        void                setCurrentUser(const CUser& user);

        bool                isProcessExists(const QString& name) const;

        void                notifyViewShow();
        void                notifyPluginsLoaded();

    signals:

        void                doSetPythonQueryModel(CPluginPythonModel* pModel);
        void                doSetPythonDependencyModel(CPluginPythonDependencyModel* pModel);
        void                doShowNotification(const QString& text, Notification type, CProgressCircle* pProgress=nullptr, int duration=Ikomia::_NotifDefaultDuration);

    public slots:

        void                onRequestPythonModel();
        void                onRequestPythonDependencyModel(const QString pluginName);
        void                onEditPythonPlugin(const QString& pluginName);
        void                onShowLocation(const QString& pluginName, int language);
        void                onInstallPythonPkg(const QString& moduleName);
        void                onUpdatePythonPkg(const QString& moduleName);

    private:

        void                initPackageAliases();

        QString             getPythonInstalledPkgVersion(const QString& name) const;
        void                getPythonPackageInfo(const QString& name) const;

        void                addToPythonPath(const QString& path);

        void                    loadCppProcessPlugins();
        void                    loadPythonProcessPlugins();
        boost::python::object   loadPythonMainModule(const std::string& folder, const std::string& name);

        void                updatePythonQueryModel();
        void                updateOutdatedPackages();

        void                fillPythonPackages();
        void                fillPythonPackagesFromScript();

    private:

        CProcessRegistration*                   m_pRegistrator = nullptr;
        CPluginPythonModel*                     m_pPythonModel = nullptr;
        CPluginPythonDependencyModel*           m_pPythonDependencyModel = nullptr;
        CUser                                   m_currentUser;
        QMap<QString, QPluginLoader*>           m_loaders;
        QMap<QString, QPair<QString,QString>>   m_pythonPackages;
        QMap<QString,QString>                   m_pythonPackageAliases;
        QString                                 m_cppPath;
        QString                                 m_pythonPath;
        QString                                 m_currentPluginName;
        const QSet<QString>                     m_systemModules = {"numpy","PyQt5"};
};

#endif // CPLUGINMANAGER_H
