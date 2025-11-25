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

#include "CPathNavigator.h"
#include <QModelIndex>
#include <QLabel>
#include "Model/Project/CMultiProjectModel.h"

CPathNavigator::CPathNavigator()
{
    m_pLayout = new QHBoxLayout;
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setSpacing(5);

    QPushButton* pHomeBtn = new QPushButton;

    QIcon icon;
    QPixmap pixmap(1024,512);
    pixmap.fill(Qt::transparent);
    QPixmap home(":/Images/home.png");
    QPixmap chevron(":/Images/chevron.png");

    QPainter painter(&pixmap);
    painter.drawPixmap(0, 0, home);
    painter.drawPixmap(home.width(), 0, chevron);
    icon.addPixmap(pixmap);
    pHomeBtn->setIcon(icon);
    pHomeBtn->setIconSize(QSize(32,16));

    connect(pHomeBtn, &QPushButton::clicked, this, &CPathNavigator::onGoHome);
    m_pLayout->addWidget(pHomeBtn);

    m_pLayout->addStretch();

    setLayout(m_pLayout);
    setObjectName("CPathNavigator");
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

void CPathNavigator::clear()
{
    foreach (CPathButton *button, m_buttons)
    {
        m_pLayout->removeWidget(button);
        disconnect(button, &CPathButton::doUpdateIndex, this, &CPathNavigator::onUpdateIndex);
        delete button;
    }

    /*for (auto widget : this->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
      delete widget;*/

    m_buttons.clear();
    m_parentIndex = QModelIndex();
}

void CPathNavigator::addPath(const QPersistentModelIndex& index)
{
    if(m_parentIndex == index.parent() && index.parent() != QModelIndex())
        return;

    // Clear all previous buttons
    clear();

    // Keep parent index in order to avoid flashing when same parent (stack images)
    m_parentIndex = index.parent();

    // Insert button for all parent from index
    // Start at parent index since current index is already displayed
    QModelIndex currIndex = index;
    while(currIndex.isValid())
    {
        if(isDataItem(currIndex))
        {
            currIndex = currIndex.parent();
            continue;
        }

        auto btn = new CPathButton;
        btn->setToolTip(currIndex.data().toString());
        btn->setMaximumWidth(120);
        btn->setIndex(currIndex);
        btn->setFullText(currIndex.data().toString());
        btn->initConnections();

        connect(btn, &CPathButton::doUpdateIndex, this, &CPathNavigator::onUpdateIndex);

        m_buttons.insert(0, btn);
        m_pLayout->insertWidget(1, btn);

        currIndex = currIndex.parent();
    }
}

void CPathNavigator::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    for(auto&& it : m_buttons)
        it->resizeEvent(event);
}

QSize CPathNavigator::sizeHint() const
{
    return QSize(0,0);
}

void CPathNavigator::onUpdateIndex(const QModelIndex& index)
{
    emit doUpdateIndex(index);
}

void CPathNavigator::onGoHome()
{
    clear();

    emit doGoHome(QModelIndex());
}

bool CPathNavigator::isDataItem(const QModelIndex& index)
{
    // Get item type by wrapping multimodel index to project model index
    auto pModel = static_cast<const CMultiProjectModel*>(index.model());
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    auto wrapIndex = pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
    auto itemPtr = static_cast<ProjectTreeItem*>(wrapIndex.internalPointer());

    return (itemPtr->getTypeId() == TreeItemType::IMAGE ||
            itemPtr->getTypeId() == TreeItemType::VIDEO ||
            itemPtr->getTypeId() == TreeItemType::LIVE_STREAM);
}
