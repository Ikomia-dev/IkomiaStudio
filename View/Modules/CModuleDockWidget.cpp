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

#include "CModuleDockWidget.h"
#include <QStackedWidget>
#include "../Common/CToolbarBorderLayout.h"
#include "../Common/CDockWidgetTitleBar.h"
#include "Workflow/CWorkflowModuleWidget.h"

CModuleDockWidget::CModuleDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    init();
}

CModuleDockWidget::CModuleDockWidget(QWidget *parent, Qt::WindowFlags flags) : QDockWidget(parent, flags)
{
    init();
}

void CModuleDockWidget::init()
{
    m_pModuleViews = new QStackedWidget(this);

    m_pLayout = new CToolbarBorderLayout;
    m_pLayout->addWidget(m_pModuleViews, CBorderLayout::Center);
    m_pLayout->addBottomBar(QSize(40,40), 5);

    m_pView = new QWidget;
    m_pView->setMinimumHeight(40);
    m_pView->setMaximumHeight(40);
    m_pView->setLayout(m_pLayout);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setWidget(m_pView);

    auto pTitleBar = new CDockWidgetTitleBar(this);
    setTitleBarWidget(pTitleBar);
}

QPropertyAnimation *CModuleDockWidget::createAnimation(QByteArray name, QVariant start, QVariant end, int duration)
{
    QPropertyAnimation* pAnimation = new QPropertyAnimation(m_pView, name);
    pAnimation->setDuration(duration);
    pAnimation->setStartValue(start);
    pAnimation->setEndValue(end);
    pAnimation->setEasingCurve(QEasingCurve::OutQuad);
    return pAnimation;
}

void CModuleDockWidget::animate()
{
    m_animations.clear();

    desactivateScrollBar();

    if(m_bIsOpened == true)
    {        
        m_currentHeight = m_pView->height();
        m_animations.push_back(createAnimation("maximumHeight", m_currentHeight, 40, 500));
        m_animations.push_back(createAnimation("minimumHeight", m_minHeight, 40, 500));
        m_bIsOpened = false;
    }
    else
    {
        if(m_currentHeight == 0)
            m_currentHeight = m_pLayout->sizeHint().height();

        m_animations.push_back(createAnimation("maximumHeight", 40, m_currentHeight, 500));
        connect(m_animations[0], &QPropertyAnimation::finished, [&]{ m_pView->setMaximumHeight(QWIDGETSIZE_MAX); });
        m_animations.push_back(createAnimation("minimumHeight", 40, m_currentHeight, 500));
        connect(m_animations[1], &QPropertyAnimation::finished, [&]{ m_pView->setMinimumHeight(m_minHeight); });
        m_bIsOpened = true;
    }

    auto pGroup = new QParallelAnimationGroup;
    for(int i=0; i<m_animations.size(); ++i)
        pGroup->addAnimation(m_animations[i]);

    pGroup->start(QPropertyAnimation::DeleteWhenStopped);

    connect(pGroup, &QParallelAnimationGroup::finished, this, &CModuleDockWidget::onActivateScrollBar);
}

void CModuleDockWidget::toggleModule(int index)
{
    //Animate if necessary
    if(m_pModuleViews->currentIndex() == index || m_bIsOpened == false)
        animate();

    //Change current module
    m_pModuleViews->setCurrentIndex(index);
    m_pLayout->getBottomContent()->setCurrentRow(index);

    //Update name
    updateWindowTitle();
}

void CModuleDockWidget::desactivateScrollBar()
{
    for(int i=0; i<m_pModuleViews->count(); ++i)
    {
        if(QString(m_pModuleViews->widget(i)->metaObject()->className()) == "CWorkflowModuleWidget")
        {
            auto pWid = qobject_cast<CWorkflowModuleWidget*>(m_pModuleViews->widget(i));
            auto pView = pWid->getView();
            // Desactivate scroll for animation during animation
            pView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
            pView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
        }
    }
}

void CModuleDockWidget::updateWindowTitle()
{
    QString title;
    if(m_bIsOpened == true)
        title = m_pModuleViews->currentWidget()->windowTitle();
    else
        title = tr("Modules");

    setWindowTitle(title);
    auto pTitleBar = static_cast<CDockWidgetTitleBar*>(titleBarWidget());
    assert(pTitleBar);
    pTitleBar->updateTitle();
}

void CModuleDockWidget::showModule(int index)
{
    //Animate if necessary
    if(m_bIsOpened == false)
        animate();

    //Change current module
    m_pModuleViews->setCurrentIndex(index);
    m_pLayout->getBottomContent()->setCurrentRow(index);

    //Update name
    updateWindowTitle();
}

void CModuleDockWidget::onActivateScrollBar()
{
    for(int i=0; i<m_pModuleViews->count(); ++i)
    {
        if(QString(m_pModuleViews->widget(i)->metaObject()->className()) == "CWorkflowModuleWidget")
        {
            auto pWid = qobject_cast<CWorkflowModuleWidget*>(m_pModuleViews->widget(i));
            auto pView = pWid->getView();
            // Activate scroll for animation and activate when animation finishes
            pView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
            pView->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
        }
    }
}

void CModuleDockWidget::addModuleWidget(QWidget *pWidget, const QIcon& icon)
{
    assert(pWidget);

    const int index = m_pModuleViews->addWidget(pWidget);
    m_minHeight = m_pLayout->sizeHint().height();

    auto pBtn = m_pLayout->addButtonToBottom("", QSize(40,40), icon);
    assert(pBtn != nullptr);
    pBtn->setToolTip(pWidget->windowTitle());

    m_moduleBtnList.push_back(pBtn);
    connect(pBtn, &QToolButton::clicked, [this, index, pBtn, icon]{ toggleModule(index);  pBtn->setIcon(icon); });
}

void CModuleDockWidget::removeModuleWidget(QWidget *pWidget)
{
    assert(pWidget);

    int index = m_pModuleViews->indexOf(pWidget);
    m_pLayout->removeButton(index, CToolbarBorderLayout::South);
    m_moduleBtnList[index]->deleteLater();
    m_moduleBtnList.erase(m_moduleBtnList.begin() + index);
    m_pModuleViews->removeWidget(pWidget);
}

QToolButton* CModuleDockWidget::getModuleBtn(int id)
{
    assert(m_moduleBtnList.size()>id);
    return m_moduleBtnList[id];
}

bool CModuleDockWidget::isModuleOpen(int id)
{
    return (m_pModuleViews->currentIndex() == id && m_bIsOpened == false);
}
