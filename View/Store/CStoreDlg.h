#ifndef CPLUGINSTOREDLG_H
#define CPLUGINSTOREDLG_H

#include <QDialog>
#include "View/Common/CDialog.h"
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class CStorePluginListView;
class CStoreQueryModel;
class CProcessDocWidget;

class CStoreDlg : public CDialog
{
    Q_OBJECT

    public:

        CStoreDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            setCurrentUser(const CUser& user);

    signals:

        void            doGetServerModel();
        void            doGetLocalModel();
        void            doPublishPlugin(const QModelIndex& index);
        void            doInstallPlugin(const QModelIndex& index);
        void            doUpdatePluginInfo(bool bFullEdit, const CProcessInfo& info);
        void            doServerSearchChanged(const QString& text);
        void            doLocalSearchChanged(const QString& text);
        void            doClose();

    public slots:

        void            onSetServerPluginModel(CStoreQueryModel* pModel);
        void            onSetLocalPluginModel(CStoreQueryModel* pModel);
        void            onShowLocalPluginInfo(const QModelIndex& index);
        void            onShowServerPluginInfo(const QModelIndex& index);

    protected:

        virtual void    showEvent(QShowEvent* event) override;
        virtual void    closeEvent(QCloseEvent* event) override;
        virtual void    hideEvent(QHideEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

        QWidget*        createLeftWidget();
        QWidget*        createRightWidget();
        QLabel*         createMessageLabel(const QString &msg);

        void            showProcessInfo(const QModelIndex& index);

    private:

        QFrame*                 m_pTitleBar = nullptr;
        QStackedWidget*         m_pPluginStackWidget = nullptr;
        QStackedWidget*         m_pRightStackWidget = nullptr;
        QPushButton*            m_pBtnServerPlugins = nullptr;
        QPushButton*            m_pBtnLocalPlugins = nullptr;
        QPushButton*            m_pBtnServerRefresh = nullptr;
        QLineEdit*              m_pEditServerSearch = nullptr;
        QLineEdit*              m_pEditLocalSearch = nullptr;
        QLabel*                 m_pLabelMsgServer = nullptr;
        QLabel*                 m_pLabelMsgLocal = nullptr;
        CStorePluginListView*   m_pServerPluginsView = nullptr;
        CStorePluginListView*   m_pLocalPluginsView = nullptr;
        CProcessDocWidget*      m_pDocWidget = nullptr;
};

#endif // CPLUGINSTOREDLG_H
