#include "CPluginManagerWidget.h"
#include <QtWidgets>
#include <QFileDialog>
#include "Widgets/FancyTabBar/fancytabbar.h"
#include "Main/AppTools.hpp"
#include "CPythonPluginMaker.h"
#include "CCppPluginMaker.h"

CPluginManagerWidget::CPluginManagerWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Plugin Manager"));
    initLayout();
    initConnections();
}

void CPluginManagerWidget::setPythonQueryModel(CPluginPythonModel *pModel)
{
    m_pPythonWidget->setModel(pModel);
}

void CPluginManagerWidget::setPythonDependencyModel(CPluginPythonDependencyModel *pModel)
{
    m_pPythonWidget->setDependencyModel(pModel);
}

void CPluginManagerWidget::initLayout()
{
    m_pTabBar = new FancyTabBar(FancyTabBar::Left);
    m_pTabBar->insertTab(0, QIcon(":/Images/python-language-logo.png"), tr("Python"));
    m_pTabBar->insertTab(1, QIcon(":/Images/C++-language-logo.png"), tr("C++"));
    m_pTabBar->setTabEnabled(0, true);
    m_pTabBar->setTabEnabled(1, false);
    m_pTabBar->setCurrentIndex(0);

    m_pPythonWidget = new CPythonPluginManagerWidget;
    m_pCppWidget = new CCppNewPluginWidget;

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pPythonWidget);
    m_pStackWidget->addWidget(m_pCppWidget);
    m_pStackWidget->setCurrentIndex(0);

    auto pMainLayout = new QHBoxLayout;
    pMainLayout->setSpacing(50);
    pMainLayout->addWidget(m_pTabBar);
    pMainLayout->addWidget(m_pStackWidget);

    setLayout(pMainLayout);
}

void CPluginManagerWidget::initConnections()
{
    //Language tab bar
    connect(m_pTabBar, &FancyTabBar::currentChanged, [&](int index){ m_pStackWidget->setCurrentIndex(index); });

    //Python
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doPluginCreated, [&](const QString& name)
    {
        emit doNewNotification(QString(tr("The plugin %1 is now created and ready to implement").arg(name)), Notification::INFO, nullptr, Ikomia::_NotifDefaultDuration);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doReloadAll, [&]{ emit doReloadAll(); });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doReload, [&](const QString& pluginName)
    {
        emit doReload(pluginName, CProcessInfo::PYTHON);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doEdit, [&](const QString& pluginName)
    {
        emit doEditPythonPlugin(pluginName);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doShowLocation, [&](const QString& pluginName)
    {
        emit doShowLocation(pluginName, CProcessInfo::PYTHON);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doGetPluginDependencyModel, [&](const QString& pluginName)
    {
        emit doGetPythonDependencyModel(pluginName);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doInstall, [&](const QString& moduleName)
    {
       emit doInstallPythonPkg(moduleName);
    });
    connect(m_pPythonWidget, &CPythonPluginManagerWidget::doUpdate, [&](const QString& moduleName)
    {
       emit doUpdatePythonPkg(moduleName);
    });

    //C++
    connect(m_pCppWidget, &CCppNewPluginWidget::doPluginCreated, [&](const QString& name)
    {
        emit doNewNotification(QString(tr("The plugin %1 is now created and ready to implement").arg(name)), Notification::INFO, nullptr, Ikomia::_NotifDefaultDuration);
    });
}

