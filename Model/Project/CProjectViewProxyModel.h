#ifndef CDATALISTVIEWPROXYMODEL_H
#define CDATALISTVIEWPROXYMODEL_H

/**
 * @file      CDataListViewProxyModel.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListViewProxyModel definition
 *
 * @details   Details
 */

#include <QSortFilterProxyModel>

/**
 * @brief
 *
 */
class CProjectViewProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        /**
         * @brief
         *
         * @param parent
         */
        CProjectViewProxyModel(QObject * parent = nullptr);

        /**
         * @brief
         *
         * @param index
         * @param role
         * @return QVariant
         */
        QVariant    data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    signals:        

        void        doCreateIcon(const QPersistentModelIndex& index, QSize size) const;

    public slots:

        void        onCreateIcon(const QPersistentModelIndex& index, QSize size);

    private:

        QMap<QString, QIcon> m_iconMap; /**< TODO: describe */
};

#endif // CDATALISTVIEWPROXYMODEL_H
