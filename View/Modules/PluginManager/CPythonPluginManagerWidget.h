#ifndef CPYTHONPLUGINMANAGERWIDGET_H
#define CPYTHONPLUGINMANAGERWIDGET_H

#include <QWidget>

class CPluginPythonModel;
class CPluginPythonDependencyModel;

class CPythonPluginManagerWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CPythonPluginManagerWidget(QWidget *parent = nullptr);

        void            setModel(CPluginPythonModel* pModel);
        void            setDependencyModel(CPluginPythonDependencyModel* pModel);

    signals:

        void            doPluginCreated(const QString& pluginName);
        void            doReload(const QString& pluginName);
        void            doReloadAll();
        void            doEdit(const QString& pluginName);
        void            doShowLocation(const QString& pluginName);
        void            doGetPluginDependencyModel(const QString& pluginName);
        void            doInstall(const QString& moduleName);
        void            doUpdate(const QString& moduleName);

    private slots:

        void            onNewPlugin();
        void            onReloadPlugin();
        void            onEditPlugin();
        void            onShowLocation();
        void            onPluginSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
        void            onInstallDependency();
        void            onUpdateDependency();

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QIcon& icon, const QString& tooltip);

        void            adjustTableSize(QTableView *pTableView);

    private:

        QTableView*         m_pPluginsView = nullptr;
        QTableView*         m_pDependenciesView = nullptr;
        QPushButton*        m_pNewBtn = nullptr;
        QPushButton*        m_pReloadAllBtn = nullptr;
        QPushButton*        m_pReloadBtn = nullptr;
        QPushButton*        m_pEditBtn = nullptr;
        QPushButton*        m_pShowLocationBtn = nullptr;
        QPushButton*        m_pInstallPkgBtn = nullptr;
        QPushButton*        m_pUpdatePkgBtn = nullptr;

};

#endif // CPYTHONPLUGINMANAGERWIDGET_H
