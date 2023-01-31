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

#include "CPluginManager.h"
#include "Main/LogCategory.h"
#include "Main/AppTools.hpp"
#include "Core/CIkomiaRegistry.h"
#include "Core/CPluginTools.h"

CPluginManager::CPluginManager()
{
    //Plugin Directory creation
    m_cppPath = QString::fromStdString(Utils::Plugin::getCppPath());
    m_pythonPath = QString::fromStdString(Utils::Plugin::getPythonPath());
    QDir dir;
    dir.mkpath(m_cppPath);
    dir.mkpath(m_pythonPath);
    dir.mkpath(QString::fromStdString(Utils::CPluginTools::getTransferPath()));

    //Models for python plugins and dependencies
    m_pPythonModel = new CPluginPythonModel(this);
    m_pPythonDependencyModel = new CPluginPythonDependencyModel(this);

    //Static data initialisation
    initPackageAliases();
}

void CPluginManager::loadProcessPlugins()
{
    assert(m_pRegistry);
    m_pRegistry->loadPlugins();
}

TaskFactoryPtr CPluginManager::loadProcessPlugin(const QString &name, int language)
{
    assert(m_pRegistry);

    try
    {
        if (language == ApiLanguage::PYTHON)
            m_pRegistry->loadPythonPlugin(Utils::CPluginTools::getPythonPluginFolder(name).toStdString());
        else if (language == ApiLanguage::CPP)
            m_pRegistry->loadCppPlugin(Utils::CPluginTools::getCppPluginFolder(name).toStdString());

        return m_pRegistry->getTaskFactory(name.toStdString());
    }
    catch(CException& e)
    {
        Utils::print(e.getMessage(), QtCriticalMsg);
    }
    return nullptr;
}

void CPluginManager::setRegistry(CIkomiaRegistry* pRegistry)
{
    m_pRegistry = pRegistry;
}

void CPluginManager::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

bool CPluginManager::isProcessExists(const QString &name) const
{
    return m_pRegistry->getTaskRegistrator()->getProcessFactory().isCreatorExists(name.toStdString());
}

void CPluginManager::notifyViewShow()
{
    updatePythonQueryModel();
}

void CPluginManager::notifyPluginsLoaded()
{
    updatePythonQueryModel();
}

void CPluginManager::onRequestPythonModel()
{
    updatePythonQueryModel();
}

void CPluginManager::onRequestPythonDependencyModel(const QString pluginName)
{
    if(m_pythonPackages.empty())
        fillPythonPackages();

    m_currentPluginName = pluginName;
    m_pPythonDependencyModel->setPluginName(pluginName, m_pythonPackages, m_pythonPackageAliases);
    emit doSetPythonDependencyModel(m_pPythonDependencyModel);
}

void CPluginManager::onEditPythonPlugin(const QString &pluginName)
{
    QString pluginDir = Utils::CPluginTools::getPythonPluginFolder(pluginName);
    if(pluginDir.isEmpty())
    {
        qCCritical(logPlugin).noquote() << tr("Editing failed: unable to find directory for plugin %1").arg(pluginName);
        return;
    }

    //Edit the Python file where the algorithm is implemented
    QString processFile = pluginDir + "/" + pluginName + "_process.py";
    if(boost::filesystem::exists(processFile.toStdString()) == false)
    {
        qCCritical(logPlugin).noquote() << tr("Editing failed: file %1 does not exist").arg(processFile);
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(processFile));
}

void CPluginManager::onShowLocation(const QString &pluginName, int language)
{
    QString pluginDir;
    if(language == ApiLanguage::PYTHON)
        pluginDir = Utils::CPluginTools::getPythonPluginFolder(pluginName);
    else
        pluginDir = Utils::CPluginTools::getCppPluginFolder(pluginName);

    if(pluginDir.isEmpty() == false)
        Utils::File::showLocation(pluginDir);
}

void CPluginManager::onInstallPythonPkg(const QString &moduleName)
{
    QString pkgName = moduleName;

    auto it = m_pythonPackageAliases.find(moduleName);
    if(it != m_pythonPackageAliases.end())
        pkgName = it.value();

    if(m_pythonPackages.contains(pkgName))
    {
        qCInfo(logPlugin).noquote() << tr("Package %1 is already installed").arg(pkgName);
        return;
    }

    QString cmd;
    QStringList args;
    Utils::Python::prepareQCommand(cmd, args);
    args << "-m" << "pip" << "install" << pkgName;
    QProcess* pProcess = new QProcess(this);
    connect(pProcess, &QProcess::readyReadStandardOutput, [pProcess]
    {
        qCInfo(logPlugin).noquote() << pProcess->readAllStandardOutput();
    });
    connect(pProcess, &QProcess::readyReadStandardError, [pProcess]
    {
        qCCritical(logPlugin).noquote() << pProcess->readAllStandardError();
    });
    connect(pProcess, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), [this, pProcess, pkgName](int exitCode, QProcess::ExitStatus exitStatus)
    {
        pProcess->deleteLater();
        QString version = getPythonInstalledPkgVersion(pkgName);

        if(version.isEmpty() == false)
        {
            m_pythonPackages.insert(pkgName, QPair<QString,QString>(version, version));
            m_pPythonDependencyModel->setPluginName(m_currentPluginName, m_pythonPackages, m_pythonPackageAliases);
            emit doSetPythonDependencyModel(m_pPythonDependencyModel);
        }

        if(exitStatus == QProcess::ExitStatus::NormalExit && exitCode == 0)
            emit doShowNotification(tr("Package %1 successfully installed").arg(pkgName), Notification::INFO);
        else
            emit doShowNotification(tr("Installation of package %1 ended with errors or warnings").arg(pkgName), Notification::INFO);
    });
    pProcess->start(cmd, args);
}

void CPluginManager::onUpdatePythonPkg(const QString &moduleName)
{
    QString pkgName = moduleName;

    if(m_systemModules.contains(moduleName))
    {
        qCWarning(logPlugin).noquote() << tr("Package %1 is a core module of Ikomia and can't be updated").arg(moduleName);
        return;
    }

    auto it = m_pythonPackageAliases.find(moduleName);
    if(it != m_pythonPackageAliases.end())
        pkgName = it.value();

    if(!m_pythonPackages.contains(pkgName))
    {
        qCInfo(logPlugin).noquote() << tr("Package %1 is not installed").arg(pkgName);
        return;
    }

    QString cmd;
    QStringList args;
    Utils::Python::prepareQCommand(cmd, args);
    args << "-m" << "pip" << "install" << "--upgrade" << pkgName;
    QProcess* pProcess = new QProcess(this);
    connect(pProcess, &QProcess::readyReadStandardOutput, [pProcess]
    {
        qCInfo(logPlugin).noquote() << pProcess->readAllStandardOutput();
    });
    connect(pProcess, &QProcess::readyReadStandardError, [pProcess]
    {
        qCCritical(logPlugin).noquote() << pProcess->readAllStandardError();
    });
    connect(pProcess, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), [this, pProcess, pkgName](int exitCode, QProcess::ExitStatus exitStatus)
    {
        pProcess->deleteLater();
        QString version = getPythonInstalledPkgVersion(pkgName);

        if(version.isEmpty() == false)
        {
            m_pythonPackages[pkgName].first = version;
            m_pPythonDependencyModel->setPluginName(m_currentPluginName, m_pythonPackages, m_pythonPackageAliases);
            emit doSetPythonDependencyModel(m_pPythonDependencyModel);
        }

        if(exitStatus == QProcess::ExitStatus::NormalExit && exitCode == 0)
            emit doShowNotification(tr("Package %1 successfully updated").arg(pkgName), Notification::INFO);
        else
            emit doShowNotification(tr("Update of package %1 ended with errors or warnings").arg(pkgName), Notification::INFO);
    });
    pProcess->start(cmd, args);
}

void CPluginManager::initPackageAliases()
{
    m_pythonPackageAliases.insert("pims", "PIMS");
    m_pythonPackageAliases.insert("skimage", "scikit-image");
    m_pythonPackageAliases.insert("sklearn", "scikit-learn");
    m_pythonPackageAliases.insert("PIL", "Pillow");
}

QString CPluginManager::getPythonInstalledPkgVersion(const QString &name) const
{
    QString cmd;
    QStringList args;
    Utils::Python::prepareQCommand(cmd, args);
    args << "-m" << "pip" << "show" << name;

    QProcess proc;
    proc.start(cmd, args);
    proc.waitForFinished();
    QString out = proc.readAllStandardOutput();

    if(out.isEmpty())
    {
        qCWarning(logPlugin) << tr("Error while gathering outdated Python package list.");
        return "";
    }

    QString version;
    QRegularExpression re("Version:\\s+(.*)");
    QRegularExpressionMatch match = re.match(out);

    if(match.hasMatch())
        version = match.captured(0);

    return version;
}

void CPluginManager::getPythonPackageInfo(const QString &name) const
{
    try
    {
        const std::string script = QString(
                "import requests\n"
                "url = 'https://pypi.org/pypi/%1/json'\n"
                "pkgInfo = requests.get(url).json()\n")
                .arg(name).toStdString();

        Utils::Python::runScript(script);

        CPyEnsureGIL gil;
        boost::python::object main_module = boost::python::import("__main__");
        boost::python::object main_namespace = main_module.attr("__dict__");
        boost::python::object pkgInfo = main_namespace["pkgInfo"];
        //To get specific field
         QString version = QString::fromStdString(boost::python::extract<std::string>(pkgInfo["info"]["version"]));
         Q_UNUSED(version)
    }
    catch(boost::python::error_already_set&)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
    }
}

void CPluginManager::updatePythonQueryModel()
{    
    m_pPythonModel->update();
    emit doSetPythonQueryModel(m_pPythonModel);
}

void CPluginManager::updateOutdatedPackages()
{
    QString cmd;
    QStringList args;
    Utils::Python::prepareQCommand(cmd, args);
    args << "-m" << "pip" << "list" << "--outdated" << "--format" << "json";

    QProcess proc;
    proc.start(cmd, args);
    proc.waitForFinished();
    QByteArray out =proc.readAllStandardOutput();
    auto jsonDoc = QJsonDocument::fromJson(out);

    if(jsonDoc.isNull() || jsonDoc.isEmpty() || !jsonDoc.isArray())
    {
        qCWarning(logPlugin) << tr("Error while gathering outdated Python package list.");
        return;
    }

    QJsonArray packages = jsonDoc.array();
    for(int i=0; i<packages.size(); ++i)
    {
        auto package = packages[i].toObject();
        auto name = package["name"].toString();
        auto version = package["latest_version"].toString();
        m_pythonPackages[name].second = version;
    }
}

void CPluginManager::fillPythonPackages()
{
    auto modules = Utils::Python::getInstalledModules();
    for (size_t i=0; i<modules.size(); ++i)
    {
        auto name = QString::fromStdString(modules[i].first);
        auto version = QString::fromStdString(modules[i].second);
        m_pythonPackages.insert(name, QPair<QString,QString>(version, version));
    }
}

// Not used anymore because it makes Ikomia App launch twice.
// Hack with STARTUPINFO parameters for Windows does not work
// with Python 3.8.
void CPluginManager::fillPythonPackagesFromScript()
{
    try
    {
        const std::string script =
                "import sys\n"
                "import os\n"
                "import subprocess\n"
                "import json\n"
                "command = [sys.executable, '-m', 'pip', 'list', '--format', 'json']\n"
                "startupinfo = None\n"
                "if os.name == 'nt':\n"
                "   startupinfo = subprocess.STARTUPINFO()\n"
                "   startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW\n"
                "processOutput = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, startupinfo=startupinfo)\n"
                "if processOutput.returncode == 0:\n"
                "   pipListRes = json.loads(processOutput.stdout)\n"
                "else:\n"
                "   print(processOutput.stderr)\n";

        Utils::Python::runScript(script);

        CPyEnsureGIL gil;
        boost::python::object main_module = boost::python::import("__main__");
        boost::python::object main_namespace = main_module.attr("__dict__");
        boost::python::object result = main_namespace["pipListRes"];

        for(int i=0; i<boost::python::len(result); ++i)
        {
            boost::python::object module = result[i];
            QString name = QString::fromStdString(boost::python::extract<std::string>(module["name"]));
            QString version = QString::fromStdString(boost::python::extract<std::string>(module["version"]));
            m_pythonPackages.insert(name, QPair<QString,QString>(version, version));
        }
        updateOutdatedPackages();
    }
    catch(boost::python::error_already_set&)
    {
        qCCritical(logPlugin).noquote() << QString::fromStdString(Utils::Python::handlePythonException());
    }
}
