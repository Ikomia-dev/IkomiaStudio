#ifndef CPLUGINMAKERWIDGET_H
#define CPLUGINMAKERWIDGET_H

#include <QWidget>
#include "CPythonPluginManagerWidget.h"
#include "CCppNewPluginWidget.h"

class FancyTabBar;
class CProgressCircle;

class CPluginManagerWidget : public QWidget
{
    Q_OBJECT

    public:

        CPluginManagerWidget(QWidget* parent = nullptr);

        void    setPythonQueryModel(CPluginPythonModel* pModel);
        void    setPythonDependencyModel(CPluginPythonDependencyModel* pModel);

    signals:

        void    doNewNotification(const QString text, Notification type, CProgressCircle* pItem, int duration);
        void    doGetPythonQueryModel();
        void    doGetPythonDependencyModel(const QString& pluginName);
        void    doReload(const QString& pluginName, int language);
        void    doReloadAll();
        void    doEditPythonPlugin(const QString& pluginName);
        void    doShowLocation(const QString& pluginName, int language);
        void    doInstallPythonPkg(const QString& moduleName);
        void    doUpdatePythonPkg(const QString& moduleName);

    private:

        void    initLayout();
        void    initConnections();

    private:

        FancyTabBar*                m_pTabBar = nullptr;
        QStackedWidget*             m_pStackWidget = nullptr;
        CPythonPluginManagerWidget* m_pPythonWidget = nullptr;
        CCppNewPluginWidget*        m_pCppWidget = nullptr;
};

#endif // CPLUGINMAKERWIDGET_H
