#ifndef CPROCESSLISTVIEW_H
#define CPROCESSLISTVIEW_H

#include "Main/forwards.hpp"
#include "Model/CMainModel.h"
#include <QWidget>

class CProcessListViewDelegate;

class CProcessListView : public QWidget
{
        Q_OBJECT
    public:

        CProcessListView();

        void                        setModel(QAbstractItemModel* pModel);
        void                        setModelColumn(int column);

        QListView*                  getListView();
        QSize                       getPluginSize();

    signals:

        void                        doListViewDoubleCLicked(const QString& processName);
        void                        doListViewLicked(const QModelIndex& index);        
        void                        doAddProcess(const QString& processName);
        void                        doShowProcessInfo(const QModelIndex& index);
        void                        doCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    public slots:

        void                        onUpdateModel(QAbstractItemModel* pModel);
        void                        onListViewDoubleClicked(const QModelIndex& index);
        void                        onListViewClicked(const QModelIndex& index);
        void                        onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void                        onTreeViewClicked(const QModelIndex& index);

    protected:

        bool                        eventFilter(QObject *watched, QEvent *event) override;

    private:

        void                        initConnections();

    private:

        QListView*                  m_pListView = nullptr;
        QHBoxLayout*                m_pLayout = nullptr;
        CProcessListViewDelegate*   m_pDelegate = nullptr;
        QSize                       m_itemSize = QSize(300,150);
};

#endif // CPROCESSLISTVIEW_H
