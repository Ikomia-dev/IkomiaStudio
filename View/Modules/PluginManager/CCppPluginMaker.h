#ifndef CCPPPLUGINMAKER_H
#define CCPPPLUGINMAKER_H

#include "PluginManagerDefine.hpp"

class CCppPluginMaker
{
    public:

        CCppPluginMaker();

        void    setName(const QString& name);
        void    setSrcFolder(const QString& folder);
        void    setApiFolder(const QString& folder);
        void    setProcessBaseClass(PluginManager::ProcessBaseClass base);
        void    setWidgetBaseClass(PluginManager::WidgetBaseClass base);

        void    generate();

    private:

        void    createProjectFile();
        void    createGlobalHeader();
        void    createCoreHeader();
        void    createCoreSource();

        QString getProcessBaseClass() const;
        QString getProcessBaseClassHeader() const;
        QString getWidgetBaseClass() const;
        QString getWidgetBaseClassHeader() const;

    private:

        QString                         m_name;
        QString                         m_srcFolder;
        QString                         m_apiFolder;
        PluginManager::ProcessBaseClass   m_processBaseClass = PluginManager::CPROTOCOL_TASK;
        PluginManager::WidgetBaseClass    m_widgetBaseClass = PluginManager::CPROTOCOL_TASK_WIDGET;
};

#endif // CCPPPLUGINMAKER_H
