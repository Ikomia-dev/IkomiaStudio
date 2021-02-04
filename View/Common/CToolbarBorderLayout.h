#ifndef CMODULEVIEWLAYOUT_H
#define CMODULEVIEWLAYOUT_H

/**
 * @file      CModuleViewLayout.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CModuleViewLayout definition
 *
 * @details   Details
 */

#include "CBorderLayout.h"

/**
 * @brief
 *
 */
class CToolbarBorderLayout : public CBorderLayout
{
    public:

        explicit CToolbarBorderLayout(QWidget *parent, int margin = 0, int spacing = 0);
        CToolbarBorderLayout(int spacing = 0);

        void            addLeftBar(const QSize &itemSize, int itemSpacing);
        void            addRightBar(const QSize &itemSize, int itemSpacing);
        void            addTopBar(const QSize &itemSize, int itemSpacing);
        void            addBottomBar(const QSize &itemSize, int itemSpacing);

        QToolButton*    addButtonToLeft(const QString& name = "", QSize size = QSize(32,32), const QIcon& icon = QIcon());
        QToolButton*    addButtonToRight(const QString& name = "", QSize size = QSize(32,32), const QIcon& icon = QIcon());
        QToolButton*    addButtonToTop(const QString& name = "", QSize size = QSize(32,32), const QIcon& icon = QIcon());
        QToolButton*    addButtonToBottom(const QString& name = "", QSize size = QSize(32,32), const QIcon& icon = QIcon());

        void            addSeparatorToLeft();
        void            addSeparatorToRight();
        void            addSeparatorToTop();
        void            addSeparatorToBottom();

        QListWidget*    getContent(Position pos) const;
        QListWidget*    getLeftContent() const;
        QListWidget*    getRightContent() const;
        QListWidget*    getTopContent() const;
        QListWidget*    getBottomContent() const;

        void            removeButton(int index, Position pos);

    private:

        void            initWidgetItemProperties();

        void            setWidgetItemSize(Position pos, QSize size);
        void            setWidgetItemSpacing(Position pos, int spacing);

        void            addToolbar(QBoxLayout *pLayout, QListWidget *pList, Position pos, Qt::Alignment alignement);
        QToolButton*    addButtonToToolbar(QListWidget* pList, Position pos, const QString& name, QSize size, const QIcon& icon);
        void            addSeparatorToToolbar(QListWidget* pList, Position pos);

        QBoxLayout*     createLayout(Position pos);
        QListWidget*    createListWidget(Position pos);

        void            fitListWidgetToContent(QListWidget* pList, Position pos);

    private:

        QListWidget*            m_pListLeft = nullptr;
        QListWidget*            m_pListRight = nullptr;
        QListWidget*            m_pListTop = nullptr;
        QListWidget*            m_pListBottom = nullptr;
        CListWidgetDelegate*    m_pDelegate = nullptr;
        QSize                   m_barItemSizes[5];
        int                     m_barItemSpacings[5];
};

#endif // CMODULEVIEWLAYOUT_H
