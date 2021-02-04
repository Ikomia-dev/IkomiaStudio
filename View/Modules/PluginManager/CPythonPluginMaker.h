#ifndef CPYTHONPLUGINMAKER_H
#define CPYTHONPLUGINMAKER_H

#include <QString>
#include "PluginManagerDefine.hpp"

class CPythonPluginMaker
{
    public:

        enum QtBinding
        {
            PYQT,
            PYSIDE
        };

        CPythonPluginMaker();

        void    setName(const QString& name);
        void    setProcessBaseClass(PluginManager::ProcessBaseClass base);
        void    setWidgetBaseClass(PluginManager::WidgetBaseClass base);
        void    setQtBinding(QtBinding binding);

        void    generate();

    private:

        void    createMainFile(const QString& folder);
        void    createProcessFile(const QString& folder);
        void    createWidgetFile(const QString &folder);

        QString getProcessBaseClass() const;
        QString getWidgetBaseClass() const;
        QString getQtBinding() const;
        QString getQtLayout() const;

    private:

        QString                         m_name;
        PluginManager::ProcessBaseClass m_processBaseClass = PluginManager::CPROTOCOL_TASK;
        PluginManager::WidgetBaseClass  m_widgetBaseClass = PluginManager::CPROTOCOL_TASK_WIDGET;
        QtBinding                       m_qtBinding = PYQT;
};

#endif // CPYTHONPLUGINMAKER_H
