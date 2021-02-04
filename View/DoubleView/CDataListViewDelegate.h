#ifndef CDATALISTVIEWDELEGATE_H
#define CDATALISTVIEWDELEGATE_H

/**
 * @file      CDataListViewDelegate.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListViewDelegate definition
 *
 * @details   Details
 */

#include <QStyledItemDelegate>

/**
 * @brief
 *
 */
class CDataListViewDelegate : public QStyledItemDelegate
{
    public:
        /**
         * @brief
         *
         * @param parent
         */
        CDataListViewDelegate(QObject* parent = nullptr);

    protected:
        /**
         * @brief
         *
         * @param painter
         * @param option
         * @param index
         */
        void    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        /**
         * @brief
         *
         * @param option
         * @param index
         * @return QSize
         */
        QSize   sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const;

    private:
        /**
         * @brief
         *
         * @param index
         * @return QIcon
         */
        QIcon   getDataIcon(const QModelIndex& index) const;
        /**
         * @brief
         *
         * @param index
         * @return QString
         */
        QString getDataName(const QModelIndex& index) const;

};

#endif // CDATALISTVIEWDELEGATE_H
