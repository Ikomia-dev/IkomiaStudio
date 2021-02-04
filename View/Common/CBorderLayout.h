#ifndef CBORDERLAYOUT_H
#define CBORDERLAYOUT_H

/**
 * @file      CBorderLayout.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CBorderLayout definition
 * @details   Details
 */

#include <QLayout>
#include "Main/forwards.hpp"
#include <QApplication>
#include <QStyledItemDelegate>
#include <QPainter>

class CListWidgetDelegate : public QStyledItemDelegate
{
    public:

        CListWidgetDelegate(QObject* parent=nullptr) : QStyledItemDelegate(parent)
        {
        }

        void setSelection(bool bEnable) { m_bSelection = bEnable; }
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
        {
            if(index.flags() == Qt::NoItemFlags)
                return;

            // Make the rect smaller to accommodate the border.
            static const qint32 penSize = 1;
            QPoint strokeOffset(penSize, penSize);
            QRect borderRect(option.rect.topLeft() + strokeOffset, option.rect.bottomRight() - strokeOffset);

            if(option.state.testFlag(QStyle::State_MouseOver))
            {
                auto pal = qApp->palette();
                auto color = pal.highlight().color();
                auto brushCol = color;
                brushCol.setAlpha(100);
                QPen pen(color, 1);

                QPainterPath path;
                path.addRoundedRect(borderRect, 5, 5);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(pen);
                painter->fillPath(path, brushCol);
                painter->drawPath(path);
                return;
            }
            else if(m_bSelection && option.state.testFlag(QStyle::State_Selected))
            {
                auto pal = qApp->palette();
                auto color = pal.highlight().color();
                color.setAlpha(100);

                QPainterPath path;
                path.addRoundedRect(borderRect, 5, 5);
                painter->setRenderHint(QPainter::Antialiasing);
                QPen pen(Qt::transparent, 1);
                painter->setPen(pen);
                painter->fillPath(path, color);
                painter->drawPath(path);
                return;
            }

            //default
            //QStyledItemDelegate::paint(painter, option, index);
        }

    private:
        bool    m_bSelection = true;
};

/**
 * @brief Base class for managing a border layout
 *
 */
class CBorderLayout : public QLayout
{
    Q_OBJECT

    public:
        /**
         * @brief Scoped enum for all available positions in the layout
         *
         */
        enum Position { West, North, South, East, Center };

        /**
         * @brief Ctor
         * @param parent Parent widget
         * @param margin Margin size inside the layout
         * @param spacing Spacing between widget in the layout
         */
        explicit CBorderLayout(QWidget *parent, int margin = 0, int spacing = -1);
        /**
         * @brief Ctor
         * @param spacing Spacing between widget in the layout
         */
        CBorderLayout(int spacing = -1);

        ~CBorderLayout();


        void                addItem(QLayoutItem *item) override;
        /**
         * @brief Add a \b widget at \b position in the layout
         * @param widget
         * @param position
         */
        void                addWidget(QWidget *widget, Position position);

        Qt::Orientations    expandingDirections() const override;

        bool                hasHeightForWidth() const override;

        int                 count() const override;

        QLayoutItem*        itemAt(int index) const override;

        QSize               minimumSize() const override;

        void                setGeometry(const QRect &rect) override;

        QRect               getBorderRect(Position pos) const;

        QSize               sizeHint() const override;

        QLayoutItem*        takeAt(int index) override;

        void                add(QLayoutItem *item, Position position);

    protected:

        struct ItemWrapper
        {
            ItemWrapper(QLayoutItem *i, Position p)
            {
                m_item = i;
                m_position = p;
            }

            QLayoutItem*    m_item;
            Position        m_position;
        };


        enum SizeType { MinimumSize, SizeHint, MaximumSize };

        QSize               calculateSize(SizeType sizeType) const;

        QList<ItemWrapper*> m_list; /**< TODO: describe */
};

#endif // CBORDERLAYOUT_H
