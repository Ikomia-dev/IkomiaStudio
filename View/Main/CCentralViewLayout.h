#ifndef CCENTRALVIEWLAYOUT_H
#define CCENTRALVIEWLAYOUT_H

/**
 * @file      CCentralViewLayout.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CCentralViewLayout definition
 *
 * @details   Details
 */

#include "View/Common/CBorderLayout.h"
#include "Main/forwards.hpp"

/**
 * @brief Class for managing the central view layout which is composed by a left-bar, an upper-bar and a central view.
 *
 */
class CCentralViewLayout : public CBorderLayout
{
    public:

        explicit CCentralViewLayout(QWidget *parent, int margin = 0, int spacing = 0);
        CCentralViewLayout(int spacing = 0);

        /**
         * @brief Add a left bar to the layout with a width of size \b width.
         *
         * @param width Width size of the left bar.
         */
        void            addLeftBar(const QSize &itemSize, int itemSpacing);
        void            addRightBar(const QSize &itemSize, int itemSpacing);
        /**
         * @brief Add an upper bar to the layout with a haight of size \b height.
         *
         * @param height Height size ot the upper bar.
         */
        void            addUpperBar(const QSize &itemSize, int itemSpacing);
        /**
         * @brief Add a button to the left bar with tooltip name \b name, icon \b icon of size \b size.
         *
         * @param name Tooltip name of the button.
         * @param size Size of the icon.
         * @param icon Icon.
         * @return QPushButton Returns a pointer to the button for customizing its look.
         */
        QToolButton*    addButtonToLeftBar(const QString& name, const QSize &size = QSize(32,32), const QIcon& icon = QIcon());
        QToolButton*    addButtonToRightBar(const QString& name, const QSize &size = QSize(32,32), const QIcon& icon = QIcon());

        void            addSeparatorToUpperBar();
        /**
         * @brief Add a button to the upper bar with toolTip name \b name and icon \b icon.
         *
         * @param name Tooltip name of the button.
         * @param icon Icon.
         * @return QToolButton Returns a QToolButton pointer to the button for customizing its look.
         */
        QToolButton*    addButtonToUpperBar(const QString& name, const QSize& size = QSize(32,32), const QIcon& icon = QIcon(), Qt::Alignment alignment=Qt::AlignCenter);
        void            addWidgetToUpperTab(QWidget* pWidget, Qt::Alignment alignmen=Qt::AlignCenter);
        /**
         * @brief Add an action to the main menu.
         *
         * @param pAction Pointer to a QAction.
         */
        void            addActionToMenu(QAction* pAction);

        QRect           getUpperBarRect() const;

        QListWidget*    getLeftTab();
        QListWidget*    getRightTab();
        QListWidget*    getUpCenterTab();
        QListWidget*    getUpRightTab();

    private:

        void            initWidgetItemProperties();
        /**
         * @brief Defines the grid size in QListWidget associated with each bar.
         * @param Position pos: identification of the bar.
         * @param QSize size: size of items.
         */
        void            setWidgetItemSize(Position pos, QSize size);
        /**
         * @brief Defines the space between each items.
         * @param Position pos: identification of the bar.
         * @param int spacing: space between items.
         */
        void            setWidgetItemSpacing(Position pos, int spacing);

        QListWidget*    createListWidget(Position pos);
        QWidget*        createOpenListWidget(Position pos);

        void            fitListWidgetToContent(QListWidget* pList, Position pos);

    private:

        QListWidget*            m_pListLeft = nullptr;
        QListWidget*            m_pListRight = nullptr;
        QListWidget*            m_pListUpCenter = nullptr;
        QListWidget*            m_pListUpRight = nullptr;
        QToolButton*            m_pMenu = nullptr;
        CListWidgetDelegate*    m_pDelegate = nullptr;
        QHBoxLayout*            m_pUpperLayout = nullptr;
        QSize                   m_barItemSizes[5];
        int                     m_barItemSpacings[5];
};

#endif // CCENTRALVIEWLAYOUT_H
