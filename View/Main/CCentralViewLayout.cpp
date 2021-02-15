// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @file      CCentralViewLayout.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CCentralViewLayout
 *
 * @details   Details
 */

#include "CCentralViewLayout.h"
#include <QToolButton>
#include <QMenu>
#include <QListWidgetItem>

CCentralViewLayout::CCentralViewLayout(QWidget* parent, int margin, int spacing) : CBorderLayout(parent, margin, spacing)
{
    m_pDelegate = new CListWidgetDelegate(this);
    initWidgetItemProperties();
}

CCentralViewLayout::CCentralViewLayout(int spacing) : CBorderLayout(spacing)
{
    m_pDelegate = new CListWidgetDelegate(this);
    initWidgetItemProperties();
}

void CCentralViewLayout::addLeftBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(West, itemSize);
    setWidgetItemSpacing(West, itemSpacing);

    m_pListLeft = createListWidget(West);

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0,10,0,0);
    pLayout->setSpacing(0);
    pLayout->addWidget(m_pListLeft);

    QWidget* pContainer = new QWidget;
    pContainer->setLayout(pLayout);

    addWidget(pContainer, CBorderLayout::West);
}

void CCentralViewLayout::addRightBar(const QSize& itemSize, int itemSpacing)
{
    setWidgetItemSize(East, itemSize);
    setWidgetItemSpacing(East, itemSpacing);

    m_pListRight = createListWidget(East);

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0,10,0,0);
    pLayout->setSpacing(0);
    pLayout->addWidget(m_pListRight);

    QWidget* pContainer = new QWidget;
    pContainer->setLayout(pLayout);

    addWidget(pContainer, CBorderLayout::East);
}

void CCentralViewLayout::addUpperBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(North, itemSize);
    setWidgetItemSpacing(North, itemSpacing);

    QString style = QString(
        "QToolButton {"
        // StyleSheet for your push button
        "    background: transparent;"
        "    border: none;      "
        //"    border : 1px solid %1;"
        "}"
        "QToolButton::menu-indicator {"
        // StyleSheet for your push button
        "    image: url(none.jpg);"
        "}"
        );

    QWidget* pMenuContainer = new QWidget;
    QHBoxLayout* pHLayout = new QHBoxLayout;
    pHLayout->setContentsMargins(10,5,0,0);
    pHLayout->setSpacing(0);
    pMenuContainer->setLayout(pHLayout);
    QMenu* pMenu = new QMenu;
    pMenu->setAttribute(Qt::WA_TranslucentBackground);
    auto color = qApp->palette().text().color();
    pMenu->setStyleSheet(QString("border: 1px solid %1; border-radius: 5px;").arg(color.name()));

    const QIcon menuIcon(":/Images/menu.png");
    m_pMenu = new QToolButton;
    m_pMenu->setPopupMode(QToolButton::InstantPopup);
    m_pMenu->setIcon(menuIcon);
    m_pMenu->setStyleSheet(style);
    m_pMenu->setIconSize(itemSize);
    m_pMenu->setToolTip("Menu");
    m_pMenu->setFixedWidth(itemSize.width());
    m_pMenu->setFixedHeight(itemSize.height());
    m_pMenu->setMenu(pMenu);

    pHLayout->addWidget(m_pMenu);

    m_pListUpCenter = createListWidget(North);
    m_pListUpRight = createListWidget(North);

    m_pUpperLayout = new QHBoxLayout;
    m_pUpperLayout->setContentsMargins(5,5,5,5);
    m_pUpperLayout->setSpacing(0);
    m_pUpperLayout->addWidget(pMenuContainer, 0, Qt::AlignLeft);
    m_pUpperLayout->addStretch(1);
    m_pUpperLayout->addWidget(m_pListUpCenter, 0, Qt::AlignCenter);
    m_pUpperLayout->addStretch(1);
    m_pUpperLayout->addWidget(m_pListUpRight, 0, Qt::AlignRight);

    QWidget* pContainer = new QWidget;
    pContainer->setLayout(m_pUpperLayout);

    addWidget(pContainer, CBorderLayout::North);
}

QToolButton* CCentralViewLayout::addButtonToLeftBar(const QString& name, const QSize& size, const QIcon& icon)
{
    QToolButton* pBtn = new QToolButton;
    pBtn->setProperty("class", "CCentralViewLayoutListWidgetBtn");
    pBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    pBtn->setIcon(icon);
    pBtn->setIconSize(0.8 * size);
    pBtn->setText(name);
    pBtn->setMinimumSize(size);
    QListWidgetItem* pItem = new QListWidgetItem("", m_pListLeft);
    pItem->setSizeHint(size);
    m_pListLeft->setItemWidget(pItem, pBtn);
    fitListWidgetToContent(m_pListLeft, West);
    return pBtn;
}

QToolButton* CCentralViewLayout::addButtonToRightBar(const QString& name, const QSize& size, const QIcon& icon)
{
    QToolButton* pBtn = new QToolButton;
    pBtn->setProperty("class", "CCentralViewLayoutListWidgetBtn");
    pBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    pBtn->setIcon(icon);
    pBtn->setIconSize(0.8 * size);
    pBtn->setText(name);
    pBtn->setMinimumSize(size);
    QListWidgetItem* pItem = new QListWidgetItem("", m_pListRight);
    pItem->setSizeHint(size);
    m_pListRight->setItemWidget(pItem, pBtn);
    fitListWidgetToContent(m_pListRight, East);
    return pBtn;
}

void CCentralViewLayout::addSeparatorToUpperBar()
{
    QListWidgetItem* pItem = new QListWidgetItem("", m_pListUpCenter);
    pItem->setSizeHint(m_barItemSizes[North]);
    pItem->setFlags(Qt::NoItemFlags);
    fitListWidgetToContent(m_pListUpCenter, North);
}

QToolButton* CCentralViewLayout::addButtonToUpperBar(const QString& name, const QSize& size, const QIcon& icon, Qt::Alignment alignment)
{
    QListWidget* pParentListWidget = nullptr;

    if(alignment == Qt::AlignCenter)
        pParentListWidget = m_pListUpCenter;
    else if(alignment == Qt::AlignRight)
        pParentListWidget = m_pListUpRight;

    if(pParentListWidget == nullptr)
        return nullptr;

    QToolButton* pBtn = new QToolButton;
    pBtn->setProperty("class", "CCentralViewLayoutListWidgetBtn");
    //pBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    pBtn->setIcon(icon);
    pBtn->setText(name);
    pBtn->setMinimumSize(size);
    pBtn->setMaximumSize(size);
    pBtn->setIconSize(0.8 * size);

    QListWidgetItem* pItem = new QListWidgetItem(name, pParentListWidget);
    pItem->setSizeHint(size);
    pParentListWidget->setItemWidget(pItem, pBtn);
    fitListWidgetToContent(pParentListWidget, North);
    return pBtn;
}

void CCentralViewLayout::addWidgetToUpperTab(QWidget *pWidget, Qt::Alignment alignment)
{
    assert(pWidget);

    if(alignment == Qt::AlignLeft)
    {
        int index = m_pUpperLayout->indexOf(m_pMenu);
        m_pUpperLayout->insertWidget(index + 1, pWidget, 0, alignment);
    }
    else if(alignment == Qt::AlignCenter)
    {
        int index = m_pUpperLayout->indexOf(m_pListUpCenter);
        m_pUpperLayout->insertWidget(index + 1, pWidget, 0, alignment);
    }
    else if(alignment == Qt::AlignRight)
    {
        int index = m_pUpperLayout->indexOf(m_pListUpRight);
        m_pUpperLayout->insertWidget(index, pWidget, 0, alignment);
    }
}

void CCentralViewLayout::addActionToMenu(QAction* pAction)
{
    m_pMenu->menu()->addAction(pAction);
}

QRect CCentralViewLayout::getUpperBarRect() const
{
    return getBorderRect(North);
}

QListWidget* CCentralViewLayout::getLeftTab()
{
    return m_pListLeft;
}

QListWidget* CCentralViewLayout::getRightTab()
{
    return m_pListRight;
}

QListWidget*CCentralViewLayout::getUpCenterTab()
{
    return m_pListUpCenter;
}

QListWidget*CCentralViewLayout::getUpRightTab()
{
    return m_pListUpRight;
}

void CCentralViewLayout::initWidgetItemProperties()
{
    const QSize defaultSize = QSize(24, 24);
    m_barItemSizes[West] = defaultSize;
    m_barItemSizes[East] = defaultSize;
    m_barItemSizes[North] = defaultSize;
    m_barItemSizes[South] = defaultSize;
    m_barItemSizes[Center] = defaultSize;

    const int defaultSpacing = 5;
    m_barItemSpacings[West] = defaultSpacing;
    m_barItemSpacings[East] = defaultSpacing;
    m_barItemSpacings[North] = defaultSpacing;
    m_barItemSpacings[South] = defaultSpacing;
    m_barItemSpacings[Center] = defaultSpacing;
}

void CCentralViewLayout::setWidgetItemSize(CBorderLayout::Position pos, QSize size)
{
    m_barItemSizes[pos] = size;
}

void CCentralViewLayout::setWidgetItemSpacing(CBorderLayout::Position pos, int spacing)
{
    m_barItemSpacings[pos] = spacing;
}

QListWidget *CCentralViewLayout::createListWidget(Position pos)
{
    auto pListWidget = new QListWidget;
    pListWidget->setProperty("class", "CCentralViewLayoutListWidget");

    if(pos == North)
    {
        auto pDelegate = new CListWidgetDelegate(this);
        pDelegate->setSelection(false);
        pListWidget->setItemDelegate(pDelegate);
    }
    else
        pListWidget->setItemDelegate(m_pDelegate);


    pListWidget->setSpacing(1);

    if(pos == North || pos == South)
    {
        pListWidget->setGridSize(QSize(m_barItemSizes[pos].width()+m_barItemSpacings[pos], m_barItemSizes[pos].height()));
        pListWidget->setFlow(QListWidget::LeftToRight);
        pListWidget->setMinimumHeight(m_barItemSizes[pos].height());
        pListWidget->setMaximumHeight(m_barItemSizes[pos].height());
    }
    else
    {
        pListWidget->setGridSize(QSize(m_barItemSizes[pos].width(), m_barItemSizes[pos].height()+m_barItemSpacings[pos]));
        pListWidget->setFlow(QListWidget::TopToBottom);
        pListWidget->setMinimumWidth(m_barItemSizes[pos].width());
        pListWidget->setMaximumWidth(m_barItemSizes[pos].width());
    }
    return pListWidget;
}

QWidget* CCentralViewLayout::createOpenListWidget(CBorderLayout::Position pos)
{
    m_pListUpCenter = createListWidget(pos);

    QHBoxLayout* pLayout = new QHBoxLayout;
    QLabel* pLabel = new QLabel(tr("Open"));
    pLabel->setFixedWidth(64);
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setProperty("class", "COpenLabel");
    pLayout->setContentsMargins(2,2,2,2);
    pLayout->setSpacing(0);
    pLayout->addWidget(pLabel);
    pLayout->addWidget(m_pListUpCenter);

    QWidget* pWidget = new QWidget;
    pWidget->setProperty("class", "COpenListWidget");
    pWidget->setLayout(pLayout);

    return pWidget;
}

void CCentralViewLayout::fitListWidgetToContent(QListWidget *pList, CBorderLayout::Position pos)
{
    int itemCount = pList->count();
    if(pos == North || pos == South)
    {
        int listSize = itemCount * (m_barItemSizes[pos].width() + m_barItemSpacings[pos]);
        pList->setMinimumWidth(listSize);
        pList->setMaximumWidth(listSize);
    }
    else
    {
        int listSize = itemCount * (m_barItemSizes[pos].height() + m_barItemSpacings[pos]);
        pList->setMinimumHeight(listSize);
        pList->setMaximumHeight(listSize);
    }
}
