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
        case PluginManager::CPROTOCOL_TASK: baseClass = "CProtocolTask"; break;
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
        case PluginManager::CPROTOCOL_TASK: header = "Protocol/CProtocolTask.h"; break;
        case PluginManager::CIMAGE_PROCESS_2D: header = "Core/CImageProcess2d.h"; break;
        case PluginManager::CINTERACTIVE_IMAGE_PROCESS_2D: header = "Core/CInteractiveImageProcess2d.h"; break;
        case PluginManager::CVIDEO_PROCESS: header = "Core/CVideoProcess.h"; break;
        case PluginManager::CVIDEO_PROCESS_OF: header = "Core/CVideoProcessOF.h"; break;
    }
    return header;
}

QString CCppPluginMaker::getWidgetBaseClass() const
{
    return "CProtocolTaskWidget";
}

QString CCppPluginMaker::getWidgetBaseClassHeader() const
{
    return "Protocol/CProtocolTaskWidget.hpp";
}

