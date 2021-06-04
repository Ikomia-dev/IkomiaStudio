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

#include "CCppPluginMaker.h"
#include <QFile>

CCppPluginMaker::CCppPluginMaker()
{    
}

void CCppPluginMaker::setName(const QString &name)
{
    m_name = name;
}

void CCppPluginMaker::setSrcFolder(const QString &folder)
{
    m_srcFolder = folder;
}

void CCppPluginMaker::setApiFolder(const QString &folder)
{
    m_apiFolder = folder;
}

void CCppPluginMaker::setProcessBaseClass(PluginManager::ProcessBaseClass base)
{
    m_processBaseClass = base;
}

void CCppPluginMaker::setWidgetBaseClass(PluginManager::WidgetBaseClass base)
{
    m_widgetBaseClass = base;
}

void CCppPluginMaker::generate()
{
    if(m_name.isEmpty())
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Empty plugin name").toStdString(), __func__, __FILE__, __LINE__);

    //Create plugin folder if necessary
    QDir dir;
    dir.mkpath(m_srcFolder);

    //Create Qt project file (.pro)
    createProjectFile();
    //Create headers
    createGlobalHeader();
    createCoreHeader();
    //Create source file
    createCoreSource();
}

void CCppPluginMaker::createProjectFile()
{
    QFile templateFile(":/Templates/C++/template.pro");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read main template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_PluginNameUpperCase_", m_name.toUpper());
    newContent = newContent.replace("_APIFolder_", m_apiFolder);
    QString filePath = m_srcFolder + "/" + m_name + ".pro";

    QFile projectFile(filePath);
    if(projectFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write Qt project file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream mainTextStream(&projectFile);
    mainTextStream << newContent;
}

void CCppPluginMaker::createGlobalHeader()
{
    QFile templateFile(":/Templates/C++/template_global.hpp");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read global header template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginNameUpperCase_", m_name.toUpper());
    QString filePath = m_srcFolder + "/" + m_name + "Global.hpp";

    QFile headerFile(filePath);
    if(headerFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write global header file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream headerTextStream(&headerFile);
    headerTextStream << newContent;
}

void CCppPluginMaker::createCoreHeader()
{
    QFile templateFile(":/Templates/C++/template_core.h");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read core header template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginNameUpperCase_", m_name.toUpper());
    newContent = newContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_ProcessBaseClassHeader_", getProcessBaseClassHeader());
    newContent = newContent.replace("_ProcessBaseClass_", getProcessBaseClass());
    newContent = newContent.replace("_WidgetBaseClassHeader_", getWidgetBaseClassHeader());
    newContent = newContent.replace("_WidgetBaseClass_", getWidgetBaseClass());
    QString filePath = m_srcFolder + "/" + m_name + ".h";

    QFile headerFile(filePath);
    if(headerFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write core header file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream headerTextStream(&headerFile);
    headerTextStream << newContent;
}

void CCppPluginMaker::createCoreSource()
{
    QFile templateFile(":/Templates/C++/template_core.cpp");
    if(templateFile.open(QFile::ReadOnly | QFile::Text) == false)
        throw CException(CoreExCode::INVALID_PARAMETER, QObject::tr("Read core source template failed").toStdString(), __func__, __FILE__, __LINE__);

    QTextStream txtStream(&templateFile);
    auto templateContent = txtStream.readAll();
    auto newContent = templateContent.replace("_PluginName_", m_name);
    newContent = newContent.replace("_ProcessBaseClass_", getProcessBaseClass());
    newContent = newContent.replace("_WidgetBaseClass_", getWidgetBaseClass());
    QString filePath = m_srcFolder + "/" + m_name + ".cpp";

    QFile srcFile(filePath);
    if(srcFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate) == false)
    {
        std::string err = QObject::tr("Write core source file to %1 failed").arg(filePath).toStdString();
        throw CException(CoreExCode::INVALID_PARAMETER, err, __func__, __FILE__, __LINE__);
    }
    QTextStream srcTextStream(&srcFile);
    srcTextStream << newContent;
}

QString CCppPluginMaker::getProcessBaseClass() const
{
    QString baseClass;
    switch(m_processBaseClass)
    {
        case PluginManager::CWorkflow_TASK: baseClass = "CWorkflowTask"; break;
        case PluginManager::CIMAGE_PROCESS_2D: baseClass = "CImageProcess2d"; break;
        case PluginManager::CINTERACTIVE_IMAGE_PROCESS_2D: baseClass = "CInteractiveImageProcess2d"; break;
        case PluginManager::CVIDEO_PROCESS: baseClass = "CVideoProcess"; break;
        case PluginManager::CVIDEO_PROCESS_OF: baseClass = "CVideoProcessOF"; break;
    }
    return baseClass;
}

QString CCppPluginMaker::getProcessBaseClassHeader() const
{
    QString header;
    switch(m_processBaseClass)
    {
        case PluginManager::CWorkflow_TASK: header = "Workflow/CWorkflowTask.h"; break;
        case PluginManager::CIMAGE_PROCESS_2D: header = "Core/CImageProcess2d.h"; break;
        case PluginManager::CINTERACTIVE_IMAGE_PROCESS_2D: header = "Core/CInteractiveImageProcess2d.h"; break;
        case PluginManager::CVIDEO_PROCESS: header = "Core/CVideoProcess.h"; break;
        case PluginManager::CVIDEO_PROCESS_OF: header = "Core/CVideoProcessOF.h"; break;
    }
    return header;
}

QString CCppPluginMaker::getWidgetBaseClass() const
{
    return "CWorkflowTaskWidget";
}

QString CCppPluginMaker::getWidgetBaseClassHeader() const
{
    return "Workflow/CWorkflowTaskWidget.hpp";
}

