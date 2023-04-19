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
    createTestFile(pluginFolder);

    //Requirements files
    QFile requirementsFile(pluginFolder + "/requirements.txt");
    requirementsFile.open(QIODevice::WriteOnly | QFile::Text);
}

void CPythonPluginMaker::createMainFile(const QString& folder)
{
    QFile templateFile(":/Templates/Python/template.py");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read main template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto className = Utils::String::toCamelCase(m_name);
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_PluginClassName_", className);
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
    auto className = Utils::String::toCamelCase(m_name);
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_PluginClassName_", className);
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
    auto className = Utils::String::toCamelCase(m_name);
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_PluginClassName_", className);
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

void CPythonPluginMaker::createTestFile(const QString& folder)
{
    QFile templateFile(":/Templates/Python/template_test.py");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read test template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    QString filePath = folder + "/" + m_name + "_test.py";
    QFile testFile(filePath);

    if(testFile.open(QIODevice::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write test file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream testTextStream(&testFile);
    testTextStream << templateContent;
}

QString CPythonPluginMaker::getProcessBaseClass() const
{
    QString baseClass;
    switch(m_processBaseClass)
    {
        case PluginManager::CWORKFLOW_TASK: baseClass = "core.CWorkflowTask"; break;
        case PluginManager::C2DIMAGE_TASK: baseClass = "dataprocess.C2dImageTask"; break;
        case PluginManager::C2DIMAGE_INTERACTIVE_TASK: baseClass = "dataprocess.C2dImageInteractiveTask"; break;
        case PluginManager::CVIDEO_TASK: baseClass = "dataprocess.CVideoTask"; break;
        case PluginManager::CCLASSIFICATION_TASK: baseClass = "dataprocess.CClassificationTask"; break;
        case PluginManager::COBJECT_DETECTION_TASK: baseClass = "dataprocess.CObjectDetectionTask"; break;
        case PluginManager::CSEMANTIC_SEGMENTATION_TASK: baseClass = "dataprocess.CSemanticSegmentationTask"; break;
        case PluginManager::CINSTANCE_SEGMENTATION_TASK: baseClass = "dataprocess.CInstanceSegmentationTask"; break;
        case PluginManager::CKEYPOINT_DETECTION_TASK: baseClass = "dataprocess.CKeypointDetectionTask"; break;
        case PluginManager::CVIDEO_OF_TASK: baseClass = "dataprocess.CVideoOFTask"; break;
    }
    return baseClass;
}

QString CPythonPluginMaker::getWidgetBaseClass() const
{
    return "core.CWorkflowTaskWidget";
}

QString CPythonPluginMaker::getQtBinding() const
{
    QString bindingBlock;
    switch(m_qtBinding)
    {
        case PYQT: bindingBlock = "# PyQt GUI framework\nfrom PyQt5.QtWidgets import *"; break;
        case PYSIDE: bindingBlock = "# PySide GUI framwork\nfrom PySide2 import QtCore, QtGui, QtWidgets"; break;
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
