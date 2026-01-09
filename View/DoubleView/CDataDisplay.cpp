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
 * @file      CDataDisplay.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CDataDisplay
 *
 * @details   Details
 */

#include "CDataDisplay.h"
#include <QCheckBox>
#include "Workflow/CViewPropertyIO.h"

CDataDisplay::CDataDisplay(QWidget* parent, const QString& name, int flags) : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_flags = flags;
    initLayout(name);
    installEventFilter(this);
    initConnections();
}

void CDataDisplay::initLayout(const QString& name)
{
    setObjectName("CDataDisplay");
    m_pLayout = new CDataDisplayLayout;
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(5);

    m_pVbox = new QVBoxLayout;
    m_pVbox->setContentsMargins(3, 3, 3, 3);
    m_pVbox->setSpacing(0);

    if(m_flags)
    {
        m_pHbox = new QHBoxLayout;
        m_pHbox->setContentsMargins(0, 0, 0, 0);
        m_pHbox->setSpacing(2);
    }

    if(m_flags & CHECKBOX)
    {
        m_pCheckBox = new QCheckBox;
        m_pCheckBox->setFixedHeight(25);
        m_pHbox->addWidget(m_pCheckBox);
    }

    if(m_flags & TITLE)
    {
        m_pLabel = new QLabel;
        m_pHbox->addWidget(m_pLabel);
        setName(name);
    }

    if(m_flags & MAXIMIZE_BUTTON || m_flags & CLOSE_BUTTON)
        m_pHbox->addStretch(1);

    if(m_flags & MAXIMIZE_BUTTON)
    {
        m_pMaximizeBtn = createButton(QIcon(":/Images/view-fullscreen.png"), tr("Minimize/Maximize window"));
        m_pHbox->addWidget(m_pMaximizeBtn);
    }

    if(m_flags & CLOSE_BUTTON)
    {
        m_pCloseBtn = createButton(QIcon(":/Images/close.png"), tr("Close window"));
        m_pHbox->addWidget(m_pCloseBtn);
    }

    if(m_flags)
        m_pVbox->addLayout(m_pHbox);        

    m_pVbox->addLayout(m_pLayout);
    setLayout(m_pVbox);
}

void CDataDisplay::initConnections()
{
    if(m_flags & CLOSE_BUTTON)
        connect(m_pCloseBtn, &QPushButton::clicked, this, &CDataDisplay::onClose);

    if(m_flags & MAXIMIZE_BUTTON)
        connect(m_pMaximizeBtn, &QPushButton::clicked, this, &CDataDisplay::onMaximizeClicked);

    if(m_flags & CHECKBOX)
        connect(m_pCheckBox, &QCheckBox::clicked, this, &CDataDisplay::onChecked);
}

QPushButton *CDataDisplay::createButton(const QIcon& icon, const QString& tooltip)
{
    auto pal = qApp->palette();
    auto color = pal.highlight().color();
    auto pBtn = new QPushButton;

    pBtn->setIcon(icon);
    pBtn->setFlat(true);
    pBtn->setFixedSize(QSize(22, 22));
    pBtn->setIconSize(QSize(16, 16));
    pBtn->setToolTip(tooltip);
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    return pBtn;
}

void CDataDisplay::addDataView(CDataDisplay* pDataView, int r, int c)
{
    assert(pDataView);

    QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
    if(pItem != nullptr)
    {
        m_pLayout->removeItem(pItem);
        auto pData = static_cast<CDataDisplay*>(pItem->widget());
        pData->onClose();
        delete pItem;
    }
    m_pLayout->addWidget(pDataView, r, c);
}

void CDataDisplay::addDataView(CDataDisplay *pDataView)
{
    assert(pDataView);
    int row = 0, column = 0;

    auto freeCell = getFirstFreePosition();
    if(freeCell.first != -1 && freeCell.second != -1)
    {
        //Insert view at free position
        row = freeCell.first;
        column = freeCell.second;
    }
    else
    {
        //Need to create new row or column according to view size
        auto dataViewSize = pDataView->size();
        auto containerSize = size();
        //Calculate aspect ratio for new view
        float dataViewAspectRatio = (float)dataViewSize.width() / (float)dataViewSize.height();
        //Calculate aspect ratio for case row+1
        float cellWidth1, cellHeight1;
        cellWidth1 = (float)containerSize.width() / (float)m_pLayout->columnCount();
        cellHeight1 = (float)containerSize.height() / (float)(m_pLayout->rowCount() + 1);
        float aspectRatio1 = cellWidth1 / cellHeight1;
        //Calculate aspect ratio for case column+1
        float cellWidth2, cellHeight2;
        cellWidth2 = (float)containerSize.width() / (float)(m_pLayout->columnCount() + 1);
        cellHeight2 = (float)containerSize.height() / (float)m_pLayout->rowCount();
        float aspectRatio2 = cellWidth2 / cellHeight2;
        //Keep the solution that minimizes the difference of aspect ration
        auto diff1 = fabs(dataViewAspectRatio - aspectRatio1);
        auto diff2 = fabs(dataViewAspectRatio - aspectRatio2);

        if(diff1 < diff2)
        {
            //New row
            row = m_pLayout->rowCount();
            column = 0;
        }
        else
        {
            //New column
            row = 0;
            column = m_pLayout->columnCount();
        }
    }

    QLayoutItem* pItem = m_pLayout->itemAtPosition(row, column);
    if(pItem != nullptr)
    {
        m_pLayout->removeItem(pItem);
        auto pData = static_cast<CDataDisplay*>(pItem->widget());
        assert(pData != nullptr);
        pData->onClose();
        delete pItem;
    }
    m_pLayout->addWidget(pDataView, row, column);
}

void CDataDisplay::removeDataView(CDataDisplay* pDataView)
{
    assert(pDataView != nullptr);
    // pDataView is not deleted!
    m_pLayout->removeWidget(pDataView);
}

void CDataDisplay::removeDataView(int r, int c)
{
    auto pItem = m_pLayout->itemAtPosition(r, c);
    if(pItem == nullptr)
        return;

    m_pLayout->removeWidget(pItem->widget());
}

void CDataDisplay::removeDataViews(const QList<CDataDisplay *> &toRemoveList, bool bDelete)
{
    QList<CDataDisplay*> displays;
    auto nRow = m_pLayout->rowCount();
    auto nCol = m_pLayout->columnCount();

    for(int r=0; r<nRow; ++r)
    {
        for(int c=0; c<nCol; ++c)
        {
            QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
            if(pItem != nullptr)
            {
                m_pLayout->removeItem(pItem);
                displays.push_back(static_cast<CDataDisplay*>(pItem->widget()));
            }
        }
    }

    for(int i=0; i<displays.size(); ++i)
    {
        if(toRemoveList.indexOf(displays[i]) != -1)
        {
            if(bDelete)
                displays[i]->onClose();
        }
        else
            addDataView(displays[i]);
    }
}

void CDataDisplay::removeAll()
{
    auto nRow = m_pLayout->rowCount();
    auto nCol = m_pLayout->columnCount();

    for(int r=0; r<nRow; ++r)
    {
        for(int c=0; c<nCol; ++c)
        {
            QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
            if(pItem != nullptr)
            {
                m_pLayout->removeItem(pItem);
                auto pData = static_cast<CDataDisplay*>(pItem->widget());
                pData->onClose();
            }
        }
    }
}

void CDataDisplay::replaceDataView(CDataDisplay* pDataViewFrom, CDataDisplay* pDataViewTo)
{
    assert(pDataViewFrom);
    assert(pDataViewTo);

    auto pItem = m_pLayout->replaceWidget(pDataViewFrom, pDataViewTo);
    if(pItem == nullptr)
        m_pLayout->addWidget(pDataViewTo, 0, 0);
}

int CDataDisplay::getDataViewCount() const
{
    int nb = 0;
    for(int r=0; r<m_pLayout->rowCount(); ++r)
    {
        for(int c=0; c<m_pLayout->columnCount(); ++c)
        {
            QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
            if(pItem != nullptr)
                nb++;
        }
    }
    return nb;
}

CDataDisplay* CDataDisplay::getDataView(int r, int c)
{
    QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
    if(pItem == nullptr)
        return nullptr;

    return dynamic_cast<CDataDisplay*>(pItem->widget());
}

QList<CDataDisplay*> CDataDisplay::getDataViews() const
{
    QList<CDataDisplay*> views;
    for(int r=0; r<m_pLayout->rowCount(); ++r)
    {
        for(int c=0; c<m_pLayout->columnCount(); ++c)
        {
            QLayoutItem* pItem = m_pLayout->itemAtPosition(r, c);
            if(pItem != nullptr)
                views.push_back(static_cast<CDataDisplay*>(pItem->widget()));
        }
    }
    return views;
}

QPair<int, int> CDataDisplay::getFirstFreePosition() const
{
    for(int r=0; r<m_pLayout->rowCount(); ++r)
    {
        for(int c=0; c<m_pLayout->columnCount(); ++c)
        {
            if(m_pLayout->itemAtPosition(r, c) == nullptr)
                return QPair<int,int>(r, c);
        }
    }
    return QPair<int,int>(-1, -1);
}

CViewPropertyIO *CDataDisplay::getViewProperty()
{
    return m_pViewProperty;
}

int CDataDisplay::getTypeId() const
{
    return m_typeId;
}

void CDataDisplay::setName(QString name)
{
    // Widget Must be created and added at constructor
    if(m_pLabel == nullptr)
        return;

    if(!name.isEmpty())
        m_pLabel->setText(name);
}

void CDataDisplay::setViewProperty(CViewPropertyIO *pView)
{
    m_pViewProperty = pView;
}

void CDataDisplay::setViewSpaceShared(bool bShared)
{
    Q_UNUSED(bShared);
}

void CDataDisplay::setUpdateViewProperty(bool bUpdate)
{
    m_bUpdateViewProperty = bUpdate;
}

void CDataDisplay::setSelectOnClick(bool bSelect)
{
    m_bSelectOnClick = bSelect;
}

void CDataDisplay::setSelected(bool bSelected)
{
    Q_UNUSED(bSelected);
}

QSize CDataDisplay::minimumSizeHint() const
{
    return QSize(0, 0);
}

void CDataDisplay::toggleMaximizeView(CDataDisplay* pDataView)
{
    if(pDataView)
    {
        bool bMaximized = !pDataView->isMaximized();
        pDataView->setMaximized(bMaximized);
        pDataView->setUpdateViewProperty(true);

        for(int i=0; i<m_pLayout->rowCount(); i++)
        {
            for(int j=0; j<m_pLayout->columnCount(); j++)
            {
                auto pDataViewTmp = getDataView(i, j);
                if(pDataViewTmp != nullptr)
                {
                    if(pDataViewTmp != pDataView)
                    {
                        pDataViewTmp->setMaximized(false);
                        if(bMaximized)
                        {
                            //Maximize the given view by hidding others (layout stretches automatically)
                            pDataViewTmp->hide();
                        }
                        else
                        {
                            //Set all views visible (layout stretches automatically)
                            pDataViewTmp->show();
                            pDataViewTmp->setUpdateViewProperty(true);
                            pDataViewTmp->applyViewProperty();
                        }
                    }
                }
            }
        }
    }
}

void CDataDisplay::maximizeView(CDataDisplay *pDataView)
{
    if(pDataView)
    {
        for(int i=0; i<m_pLayout->rowCount(); i++)
        {
            for(int j=0; j<m_pLayout->columnCount(); j++)
            {
                auto pDataViewTmp = getDataView(i, j);
                if(pDataViewTmp != nullptr)
                {
                    if(pDataViewTmp != pDataView)
                    {
                        pDataViewTmp->hide();
                        pDataViewTmp->setMaximized(false);
                    }
                }
            }
        }
        pDataView->show();
        pDataView->setMaximized(true);
        pDataView->setUpdateViewProperty(true);
    }
}

void CDataDisplay::applyViewProperty()
{
}

void CDataDisplay::setMaximized(bool bMaximized)
{
    m_bIsMaximized = bMaximized;
}

void CDataDisplay::setActive(bool bActive)
{
    m_bActive = bActive;
}

bool CDataDisplay::isMaximized() const
{
    return m_bIsMaximized;
}

QSize CDataDisplay::sizeHint() const
{
    return QSize(0, 0);
}

bool CDataDisplay::eventFilter(QObject* obj, QEvent * event)
{
    int c = m_pLayout->columnCount();
    int r = m_pLayout->rowCount();

    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            if(r == 1 && c == 1)
                emit doDoubleClicked(this);
            else
            {
                for(int i=0; i<r; i++)
                {
                    for(int j=0; j<c; j++)
                    {
                        auto wid = getDataView(i, j);
                        if(wid != nullptr)
                        {
                            if(!wid->underMouse())
                                wid->isVisible() ? wid->hide() : wid->show();
                        }
                    }
                }
            }
            return true;
        }
        else if(event->type() == QEvent::MouseButtonPress)
        {
            if(m_bSelectOnClick)
                emit doSelect(this);
        }
    }
    return QFrame::eventFilter(obj, event);
}

void CDataDisplay::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
}

void CDataDisplay::onClose()
{
    close();
    emit doClose(this);
}

//UNUSED
void CDataDisplay::onChecked(bool bChecked)
{
    if(bChecked == true)
        emit doCheck(this);
    else
        emit doUncheck(this);
}

void CDataDisplay::onMaximizeClicked()
{
    if(m_pViewProperty)
        m_pViewProperty->toggleMaximized();

    emit doToggleMaximize(this);
}

