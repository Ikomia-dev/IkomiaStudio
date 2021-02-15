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

#include "CProcessDocWidget.h"
#include <QtWidgets>
#include "CProcessDocFrame.h"
#include "CProcessEditDocFrame.h"

CProcessDocWidget::CProcessDocWidget(int actions, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_actions = actions;
}

void CProcessDocWidget::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

void CProcessDocWidget::setProcessInfo(const CProcessInfo& info)
{
    if(m_bInit == false)
    {
        //We create layout at first use to ensure that QWebEngineView is created only when we need it
        //We suspect a bug on Windows: the QWidgetEngineProcess process is not closed when application is closed
        //except if the QWebEngineView is shown at least one time...
        initLayout();
        initConnections();
        m_bInit = true;
    }

    if(m_pDocFrame)
        m_pDocFrame->setProcessInfo(info);

    if(m_pEditDocFrame)
    {
        m_pEditDocFrame->setCurrentUser(m_currentUser);
        m_pEditDocFrame->setProcessInfo(info);
    }
}

void CProcessDocWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_pStackWidget->setCurrentIndex(0);
}

void CProcessDocWidget::initLayout()
{
    QHBoxLayout* pActionsLayout = new QHBoxLayout;
    if(m_actions & BACK)
    {
        QPushButton* pBackBtn = new QPushButton(QIcon(":/Images/back.png"), "");
        pBackBtn->setToolTip(tr("Back to process store"));
        connect(pBackBtn, &QPushButton::clicked, [&]{ emit doBack();});
        pActionsLayout->addWidget(pBackBtn);
    }

    if(m_actions & EDIT)
    {
        QPushButton* pEditBtn = new QPushButton(QIcon(":/Images/edit.png"), "");
        pEditBtn->setToolTip(tr("Edit documentation"));
        connect(pEditBtn, &QPushButton::clicked, [&]{ m_pStackWidget->setCurrentIndex(1); });
        pActionsLayout->addWidget(pEditBtn);
    }
    pActionsLayout->addStretch(1);

    m_pDocFrame = new CProcessDocFrame;
    m_pEditDocFrame = new CProcessEditDocFrame;

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pDocFrame);
    m_pStackWidget->addWidget(m_pEditDocFrame);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pActionsLayout);
    pMainLayout->addWidget(m_pStackWidget);

    setLayout(pMainLayout);
}

void CProcessDocWidget::initConnections()
{
    connect(m_pEditDocFrame, &CProcessEditDocFrame::doCancel, [&]{ m_pStackWidget->setCurrentIndex(0); });
    connect(m_pEditDocFrame, &CProcessEditDocFrame::doSave, [&](bool bFullEdit, const CProcessInfo& info)
    {
        emit doSave(bFullEdit, info);
        m_pDocFrame->setProcessInfo(info);
        m_pStackWidget->setCurrentIndex(0);
    });
}
