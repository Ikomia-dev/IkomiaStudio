#ifndef CPROCESSPOPUPDLG_H
#define CPROCESSPOPUPDLG_H

#include <QtWidgets>
#include "View/Common/CDialog.h"
#include "CProcessListView.h"
#include "CProcessParameters.h"

class CProcessDocWidget;

class CProcessPopupDlg : public CDialog
{
    Q_OBJECT

    public:

        CProcessPopupDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            initLayout();
        void            initConnections();

        void            setCurrentUser(const CUser& user);

        int             exec() override;

    signals:

        void            doTreeViewClicked(const QModelIndex& index);
        void            doTextChanged(const QString& text);
        void            doListViewDoubleClicked(const QString& processName);
        void            doAddProcess(const std::string& processName, const ProtocolTaskParamPtr& pParam);
        void            doQueryWidgetInstance(const std::string& processName);
        void            doUpdateProcessInfo(bool bFullEdit, const CProcessInfo& info);

    public slots:

        void            onSetProcessListModel(QSortFilterProxyModel* pModel);
        void            onSetProcessTreeModel(QSortFilterProxyModel* pModel);
        void            onSetWidgetInstance(const std::string &processName, ProtocolTaskWidgetPtr& widgetPtr);
        void            onTreeViewClicked(const QModelIndex& index);
        void            onTextChanged(const QString& text);
        void            onActivateScrollBar();
        void            onListViewDoubleClicked(const QString& processName);
        void            onListViewClicked(const QModelIndex& index);
        void            onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void            onAddProcess(const QString& processName);
        void            onShowProcessInfo(const QModelIndex &index);
        void            done(int r) override;
        void            reject() override;

    private:

        QWidget*        createLeftWidget();
        QWidget*        createRightWidget();

        CProcessInfo    getProcessInfo(const QModelIndex& index) const;

        void            adjustParamsWidget(const QModelIndex &index);

    private:

        using UMapNameWidget = QMap<QString, ProtocolTaskWidgetPtr>;

        QStackedWidget*     m_pRightStackedWidget = nullptr;
        QLineEdit*          m_pSearchProcess = nullptr;
        CProcessListView*   m_pProcessView = nullptr;
        QTreeView*          m_pTreeView = nullptr;
        CProcessParameters* m_pParamsWidget = nullptr;
        CProcessDocWidget*  m_pDocWidget = nullptr;
        UMapNameWidget      m_widgets;
        QSize               m_size = QSize();
        bool                m_bQueryWidget = false;
};

#endif // CPROCESSPOPUPDLG_H
