#include "CPythonPluginMaker.h"
#include "CException.h"
#include "Main/AppTools.hpp"

CPythonPluginMaker::CPythonPluginMaker()
{   
}

void CPythonPluginMaker::setName(const QString &name)
{
    m_name = Utils::File::conformName(name);
}

void CPythonPluginMaker::setProcessBaseClass(PluginManager::ProcessBaseClass base)
{
    m_processBaseClass = base;
}

void CPythonPluginMaker::setWidgetBaseClass(PluginManager::WidgetBaseClass base)
{
    m_widgetBaseClass = base;
}

void CPythonPluginMaker::setQtBinding(CPythonPluginMaker::QtBinding binding)
{
    m_qtBinding = binding;
}

void CPythonPluginMaker::generate()
{
    if(m_name.isEmpty())
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Empty plugin name").toStdString(), __func__, __FILE__, __LINE__);

    QString pluginFolder = QString::fromStdString(Utils::Plugin::getPythonPath());
    QDir dir;
    dir.mkpath(pluginFolder);

    //Create new plugin folder
    pluginFolder += "/" + m_name;
    dir.mkpath(pluginFolder);

    //Create plugin files
    QFile moduleFile(pluginFolder + "/__init__.py");
    if(moduleFile.open(QIODevice::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write module init file to %1 failed").arg(pluginFolder).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }

    createMainFile(pluginFolder);
    createProcessFile(pluginFolder);
    createWidgetFile(pluginFolder);
}

void CPythonPluginMaker::createMainFile(const QString& folder)
{
    QFile templateFile(":/Templates/Python/template.py");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read main template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginName_", m_name);
    QString filePath = folder + "/" + m_name + ".py";

    QFile mainFile(filePath);
    if(mainFile.open(QIODevice::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write main file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream mainTextStream(&mainFile);
    mainTextStream << newContent;
}

void CPythonPluginMaker::createProcessFile(const QString& folder)
{
    QFile templateFile(":/Templates/Python/template_process.py");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read process template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_ProcessBaseClass_", getProcessBaseClass());
    QString filePath = folder + "/" + m_name + "_process.py";

    QFile processFile(filePath);
    if(processFile.open(QIODevice::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write process file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream processTextStream(&processFile);
    processTextStream << newContent;
}

void CPythonPluginMaker::createWidgetFile(const QString &folder)
{
    QFile templateFile(":/Templates/Python/template_widget.py");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read widget template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_WidgetBaseClass_", getWidgetBaseClass());
    newContent = newContent.replace("_QtBindingBlock_", getQtBinding());
    newContent = newContent.replace("_InitQtLayout_", getQtLayout());

    QString filePath = folder + "/" + m_name + "_widget.py";
    QFile widgetFile(filePath);

    if(widgetFile.open(QIODevice::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write widget file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream widgetTextStream(&widgetFile);
    widgetTextStream << newContent;
}

QString CPythonPluginMaker::getProcessBaseClass() const
{
    QString baseClass;
    switch(m_processBaseClass)
    {
        case PluginManager::CPROTOCOL_TASK: baseClass = "core.CProtocolTask"; break;
        case PluginManager::CIMAGE_PROCESS_2D: baseClass = "dataprocess.CImageProcess2d"; break;
        case PluginManager::CINTERACTIVE_IMAGE_PROCESS_2D: baseClass = "dataprocess.CInteractiveImageProcess2d"; break;
        case PluginManager::CVIDEO_PROCESS: baseClass = "dataprocess.CVideoProcess"; break;
        case PluginManager::CVIDEO_PROCESS_OF: baseClass = "dataprocess.CVideoProcessOF"; break;
    }
    return baseClass;
}

QString CPythonPluginMaker::getWidgetBaseClass() const
{
    return "core.CProtocolTaskWidget";
}

QString CPythonPluginMaker::getQtBinding() const
{
    QString bindingBlock;
    switch(m_qtBinding)
    {
        case PYQT: bindingBlock = "#PyQt GUI framework\nfrom PyQt5.QtWidgets import *"; break;
        case PYSIDE: bindingBlock = "#PySide GUI framwork\nfrom PySide2 import QtCore, QtGui, QtWidgets"; break;
    }
    return bindingBlock;
}

QString CPythonPluginMaker::getQtLayout() const
{
    QString templatePath;
    switch(m_qtBinding)
    {
        case PYQT: templatePath = ":/Templates/Python/template_PyQtLayout.py"; break;
        case PYSIDE: templatePath = ":/Templates/Python/template_PySideLayout.py"; break;
    }

    QFile templateFile(templatePath);
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read Qt layout template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto layoutCode = txtStream.readAll();
    return layoutCode;
}
