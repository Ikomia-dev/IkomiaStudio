/**
 * @file      CModuleViewLayout.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CModuleViewLayout
 *
 * @details   Details
 */

#include "CToolbarBorderLayout.h"

CToolbarBorderLayout::CToolbarBorderLayout(QWidget* parent, int margin, int spacing) : CBorderLayout(parent, margin, spacing)
{
    m_pDelegate = new CListWidgetDelegate(this);
    initWidgetItemProperties();
}

CToolbarBorderLayout::CToolbarBorderLayout(int spacing) : CBorderLayout(spacing)
{
    m_pDelegate = new CListWidgetDelegate(this);
    initWidgetItemProperties();
}

void CToolbarBorderLayout::setWidgetItemSize(Position pos, QSize size)
{
    m_barItemSizes[pos] = size;
}

void CToolbarBorderLayout::setWidgetItemSpacing(Position pos, int spacing)
{
    m_barItemSpacings[pos] = spacing;
}

QListWidget *CToolbarBorderLayout::getContent(CBorderLayout::Position pos) const
{
    switch(pos)
    {
        case North: return m_pListTop;
        case South: return m_pListBottom;
        case West: return m_pListLeft;
        case East: return m_pListRight;
    }
    return nullptr;
}

QListWidget *CToolbarBorderLayout::getLeftContent() const
{
    return m_pListLeft;
}

QListWidget *CToolbarBorderLayout::getRightContent() const
{
    return m_pListRight;
}

QListWidget *CToolbarBorderLayout::getTopContent() const
{
    return m_pListTop;
}

QListWidget *CToolbarBorderLayout::getBottomContent() const
{
    return m_pListBottom;
}

void CToolbarBorderLayout::removeButton(int index, Position pos)
{
    QListWidget* pList = getContent(pos);
    if(pList == nullptr)
        return;

    QListWidgetItem* pItem = pList->item(index);
    delete pItem;
    fitListWidgetToContent(pList, pos);
}

void CToolbarBorderLayout::initWidgetItemProperties()
{
    const QSize defaultSize = QSize(24, 24);
    m_barItemSizes[West] = defaultSize;
    m_barItemSizes[East] = defaultSize;
    m_barItemSizes[North] = defaultSize;
    m_barItemSizes[South] = defaultSize;
    m_barItemSizes[Center] = defaultSize;

    const int defaultSpacing = 10;
    m_barItemSpacings[West] = defaultSpacing;
    m_barItemSpacings[East] = defaultSpacing;
    m_barItemSpacings[North] = defaultSpacing;
    m_barItemSpacings[South] = defaultSpacing;
    m_barItemSpacings[Center] = defaultSpacing;
}

void CToolbarBorderLayout::addToolbar(QBoxLayout* pLayout, QListWidget* pList, Position pos, Qt::Alignment alignement)
{
    assert(pLayout && pList);

    pLayout->addWidget(pList, 0, alignement);
    QWidget* pContainer = new QWidget;
    pContainer->setLayout(pLayout);
    pContainer->setContentsMargins(0,0,0,0);
    addWidget(pContainer, pos);
}

void CToolbarBorderLayout::addLeftBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(West, itemSize);
    setWidgetItemSpacing(West, itemSpacing);
    auto pLayout = createLayout(West);
    m_pListLeft = createListWidget(West);
    addToolbar(pLayout, m_pListLeft, West, Qt::AlignCenter);
}

void CToolbarBorderLayout::addRightBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(East, itemSize);
    setWidgetItemSpacing(East, itemSpacing);
    auto pLayout = createLayout(East);
    m_pListRight = createListWidget(East);
    addToolbar(pLayout, m_pListRight, East, Qt::AlignCenter);
}

void CToolbarBorderLayout::addTopBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(North, itemSize);
    setWidgetItemSpacing(North, itemSpacing);
    auto pLayout = createLayout(North);
    m_pListTop = createListWidget(North);
    addToolbar(pLayout, m_pListTop, North, Qt::AlignCenter);
}

void CToolbarBorderLayout::addBottomBar(const QSize &itemSize, int itemSpacing)
{
    setWidgetItemSize(South, itemSize);
    setWidgetItemSpacing(South, itemSpacing);
    auto pLayout = createLayout(South);
    m_pListBottom = createListWidget(South);
    addToolbar(pLayout, m_pListBottom, South, Qt::AlignCenter);
}

QToolButton* CToolbarBorderLayout::addButtonToToolbar(QListWidget* pList, Position pos, const QString& name, QSize size, const QIcon& icon)
{
    QToolButton* pBtn = new QToolButton();
    pBtn->setText(name);
    pBtn->setIcon(icon);
    pBtn->setIconSize(0.8 * size);
    pBtn->setMinimumSize(size);
    pBtn->setMaximumSize(size);
    QListWidgetItem* pItem = new QListWidgetItem(name, pList);
    pItem->setSizeHint(size);
    pList->setItemWidget(pItem, pBtn);
    fitListWidgetToContent(pList, pos);
    return pBtn;
}

QToolButton* CToolbarBorderLayout::addButtonToLeft(const QString& name, QSize size, const QIcon& icon)
{
    return addButtonToToolbar(m_pListLeft, West, name, size, icon);
}

QToolButton *CToolbarBorderLayout::addButtonToRight(const QString &name, QSize size, const QIcon &icon)
{
    return addButtonToToolbar(m_pListRight, East, name, size, icon);
}

QToolButton *CToolbarBorderLayout::addButtonToTop(const QString &name, QSize size, const QIcon &icon)
{
    return addButtonToToolbar(m_pListTop, North, name, size, icon);
}

QToolButton* CToolbarBorderLayout::addButtonToBottom(const QString& name, QSize size, const QIcon& icon)
{
    return addButtonToToolbar(m_pListBottom, South, name, size, icon);
}

void CToolbarBorderLayout::addSeparatorToLeft()
{
    addSeparatorToToolbar(m_pListLeft, West);
}

void CToolbarBorderLayout::addSeparatorToRight()
{
    addSeparatorToToolbar(m_pListRight, East);
}

void CToolbarBorderLayout::addSeparatorToTop()
{
    addSeparatorToToolbar(m_pListTop, North);
}

void CToolbarBorderLayout::addSeparatorToBottom()
{
    addSeparatorToToolbar(m_pListTop, North);
}

void CToolbarBorderLayout::addSeparatorToToolbar(QListWidget* pList, Position pos)
{
    QListWidgetItem* pItem = new QListWidgetItem("", pList);
    pItem->setSizeHint(m_barItemSizes[pos]);
    pItem->setFlags(Qt::NoItemFlags);
    fitListWidgetToContent(pList, pos);
}

QBoxLayout *CToolbarBorderLayout::createLayout(CBorderLayout::Position pos)
{
    QBoxLayout* pLayout = nullptr;
    if(pos == North || pos == South)
        pLayout = new QHBoxLayout;
    else
        pLayout = new QVBoxLayout;

    pLayout->setContentsMargins(0,0,0,0);
    pLayout->setMargin(0);
    pLayout->setSpacing(0);
    return pLayout;
}

QListWidget *CToolbarBorderLayout::createListWidget(CBorderLayout::Position pos)
{
    auto pListWidget = new QListWidget;
    if(pos == North)
    {
        auto pDelegate = new CListWidgetDelegate(this);
        pDelegate->setSelection(false);
        pListWidget->setItemDelegate(pDelegate);
    }
    else
        pListWidget->setItemDelegate(m_pDelegate);

    pListWidget->setSpacing(1);
    pListWidget->setStyleSheet("background: transparent; border: transparent;");

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

void CToolbarBorderLayout::fitListWidgetToContent(QListWidget* pList, Position pos)
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
