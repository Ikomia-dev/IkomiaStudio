#ifndef CPROCESSPANE_H
#define CPROCESSPANE_H

#include "../Common/CPane.h"
#include "Model/CMainModel.h"
#include "CProcessParameters.h"
#include "CProcessListPopup.h"

class CProcessPane : public QWidget
{
    Q_OBJECT

    public:

        CProcessPane(QWidget* parent = nullptr);

        void                setCurrentUser(const CUser& user);
        void                update();

        QTreeView*          getTreeView();
        CProcessListPopup*  getListPopup();
        CProcessParameters* getProcessParameters();
        QLineEdit*          getSearchBox();

    public slots:

        void    onSetModel(QSortFilterProxyModel* pModel);
        void    onSetTableModel(QSortFilterProxyModel* pTableModel);
        void    onTreeItemClicked(const QModelIndex& index);
        void    onSearchProcess(const QString& text);
        void    onSetWidgetInstance(const std::string &processName, ProtocolTaskWidgetPtr& widgetPtr);
        void    onSetProcessInfo(const CProcessInfo& info);
        void    onShowWidgetFromList(const QString& name);
        void    onAllProcessReloaded();
        void    onProcessReloaded(const QString& name);

    signals:

        void    doApplyProcess(const std::string& name, const std::shared_ptr<CProtocolTaskParam>& pParam);
        void    doSearchProcess(const QString& text);
        void    doInitWidgetFactoryConnection();
        void    doEndWidgetFactoryConnection();
        void    doQueryWidgetInstance(const std::string& processName);
        void    doQueryProcessInfo(const std::string& processName);
        void    doTreeViewClicked(const QModelIndex& index);

    protected:

        void    hideEvent(QHideEvent* event) override;
        void    resizeEvent(QResizeEvent *event) override;

    private:

        void    initLayout();
        void    initConnections();

        void    expandProcessItem(const QModelIndex& index, QSortFilterProxyModel* pModel, int depth);
        void    expandProcess(QSortFilterProxyModel* pModel, int depth);

        void    showProcessWidget(const std::string& processName);
        void    showProcessWidgetFromList(const std::string& processName);
        void    showProcessListWidget(const std::string& categoryName);

        void    adjustProcessWidget();

        void    updateWidgetPosition(QWidget* pWidget, QPoint itemPosition, int borderSize) ;

    private:

        using UMapNameWidget = std::unordered_map<std::string, ProtocolTaskWidgetPtr>;

        QTreeView*          m_pProcessTree = nullptr;
        //We need to have persistent storage due to shared_ptr
        //-> unique_ptr would have been best but Boost.Python is not compatible
        UMapNameWidget      m_widgets;
        CProcessParameters* m_pParamsWidget = nullptr;
        CProcessListPopup*  m_pProcessList = nullptr;
        QLineEdit*          m_pSearchBox = nullptr;
        QAbstractItemView*  m_pCurrentView = nullptr;
        bool                m_bQueryWidget = false;
};

#endif // CPROCESSPANE_H
