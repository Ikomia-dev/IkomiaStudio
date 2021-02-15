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

#include "CRollupWidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <assert.h>
#include "CRollupBtn.h"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

CRollupWidget::CRollupWidget(QWidget *parent) : QWidget(parent)
{    
    setObjectName("CRollupWidget");
    m_pContainer = new QWidget;

    QVBoxLayout* pInnerLayout = new QVBoxLayout;
    pInnerLayout->setAlignment(Qt::AlignTop);
    pInnerLayout->setContentsMargins(0, 0, 0, 0);
    pInnerLayout->setSpacing(0);
    pInnerLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_pContainer->setLayout(pInnerLayout);

    QScrollArea* pScrollArea = new QScrollArea;
    pScrollArea->setFrameShape(QFrame::NoFrame);
    pScrollArea->setWidget(m_pContainer);
    pScrollArea->setWidgetResizable(true);

    QVBoxLayout* pOuterLayout = new QVBoxLayout;
    pOuterLayout->setAlignment(Qt::AlignTop);
    pOuterLayout->addWidget(pScrollArea);
    setLayout(pOuterLayout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void CRollupWidget::addWidget(const QIcon& icon, const QString& name, QWidget *pWidget)
{
    assert(pWidget);

    QFrame* pFrame = makeFrameWidget(pWidget);

    CRollupBtn* pBtn = new CRollupBtn(icon, name, true);
    m_pContainer->layout()->addWidget(pBtn);

    pBtn->setWidget(pFrame);

    m_buttons.push_back(pBtn);
    m_widgets.push_back(pFrame);
}

QFrame* CRollupWidget::makeFrameWidget(QWidget* pWidget)
{
    QFrame* pFrame = new QFrame;
    pFrame->setObjectName("CRollupFrame");
    QVBoxLayout* pFrameLayout = new QVBoxLayout;
    pFrameLayout->addWidget(pWidget);
    pFrame->setLayout(pFrameLayout);
    return pFrame;
}

void CRollupWidget::insertWidget(int position, QString name, QWidget *pWidget)
{
    assert(pWidget);

    QFrame* pFrame = makeFrameWidget(pWidget);

    CRollupBtn* pBtn = new CRollupBtn(QIcon(), name, true);
    auto pLayout = static_cast<QVBoxLayout*>(m_pContainer->layout());
    pLayout->insertWidget(position*2, pBtn);

    pBtn->setWidget(pFrame);

    m_widgets.insert(position, pFrame);
    m_buttons.insert(position, pBtn);
}

QWidget* CRollupWidget::replaceWidget(int position, QString newName, QWidget *pWidget)
{
    assert(pWidget);
    QWidget* pOld = nullptr;

    if(position >= 0 && position < m_widgets.size())
    {
        if(newName.isEmpty() == false)
            m_buttons[position]->setText(newName);

        QFrame* pFrame = makeFrameWidget(pWidget);
        pOld = m_widgets[position];
        m_buttons[position]->replaceWidget(pOld, pFrame);
        m_widgets[position] = pFrame;
        m_buttons[position]->setFold(true);
    }
    return pOld;
}

void CRollupWidget::removeWidget(QWidget* pWidget)
{
    int index = m_widgets.indexOf(pWidget);
    if(index != -1)
    {
        m_buttons[index]->removeWidget(pWidget);
        m_pContainer->layout()->removeWidget(m_buttons[index]);
        delete m_buttons[index];
        m_buttons.removeAt(index);
        m_widgets.removeAt(index);
    }
}

QWidget* CRollupWidget::removeWidget(int position)
{
    QWidget* pRemoved = nullptr;
    if(position >= 0 && position < m_widgets.size())
    {
        pRemoved = m_widgets[position];
        m_buttons[position]->removeWidget(m_widgets[position]);
        m_pContainer->layout()->removeWidget(m_buttons[position]);
        delete m_buttons[position];
        m_buttons.removeAt(position);
        m_widgets.removeAt(position);
    }
    return pRemoved;
}

void CRollupWidget::expand(int position)
{
    if(position >= 0 && position < m_widgets.size())
        m_widgets[position]->show();
}

void CRollupWidget::expand(QWidget *pWidget)
{
    assert(pWidget);
    pWidget->show();
}

void CRollupWidget::expandAll()
{
    for(int i=0; i<m_widgets.size(); ++i)
        m_widgets[i]->show();
}

void CRollupWidget::collapse(int position)
{
    if(position >= 0 && position < m_widgets.size())
        m_widgets[position]->hide();
}

void CRollupWidget::collapse(QWidget *pWidget)
{
    assert(pWidget);
    pWidget->hide();
}

void CRollupWidget::collapseAll()
{
    for(int i=0; i<m_widgets.size(); ++i)
        m_widgets[i]->hide();
}
