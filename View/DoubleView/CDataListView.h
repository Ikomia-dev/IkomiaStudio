#ifndef CDATALISTVIEW_H
#define CDATALISTVIEW_H

/**
 * @file      CDataListView.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListView definition
 *
 * @details   Details
 */

#include "Main/forwards.hpp"
#include <QWidget>


class CDataListView : public QWidget
{
    Q_OBJECT
    public:

        CDataListView();

        void                    setSourceModel(QAbstractItemModel* pModel);

        void                    updateIndex(const QModelIndex& index);

        QAbstractProxyModel*    proxyModel();

    signals:

        void                    doDisplayData(const QModelIndex& index);

        void                    doUpdateIndex(const QModelIndex& index);

    public slots:

        void                    onUpdateSourceModel(QAbstractItemModel* pModel);
        void                    onListViewDoubleClicked(const QModelIndex& index);
        void                    onListViewClicked(const QModelIndex& index);

    protected:

        bool                    eventFilter(QObject *watched, QEvent *event) override;

    private:

        void                    initConnections();

        QModelIndex             getImageIndex(const QModelIndex& imgChildIndex);

    private:

        QListView*              m_pListView = nullptr;
        QHBoxLayout*            m_pLayout = nullptr;
        CProjectViewProxyModel* m_pProxyModel = nullptr;
};

#endif // CDATALISTVIEW_H
