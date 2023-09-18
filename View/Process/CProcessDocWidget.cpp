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
#include "Main/AppTools.hpp"
#include "CProcessDocFrame.h"
#include "Core/CPluginTools.h"

CProcessDocWidget::CProcessDocWidget(int actions, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_actions = actions;
    initLayout();
}

void CProcessDocWidget::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

void CProcessDocWidget::setProcessInfo(const CTaskInfo& info, bool bFromHub)
{
    m_processInfo = info;
    m_pInstallBtn->setVisible(bFromHub);

    if(m_pDocFrame)
        m_pDocFrame->setProcessInfo(info);
}

void CProcessDocWidget::onShowSourceCode()
{
    if(m_processInfo.m_language == ApiLanguage::PYTHON)
    {
        auto pluginDir = QString::fromStdString(Utils::CPluginTools::getPythonPluginFolder(m_processInfo.m_name));
        if(!pluginDir.isEmpty() && Utils::File::isFileExist(pluginDir.toStdString()))
            Utils::File::showLocation(pluginDir);
        else if(!m_processInfo.m_repo.empty())
            QDesktopServices::openUrl(QUrl(QString::fromStdString(m_processInfo.m_repo)));
    }
    else if(!m_processInfo.m_repo.empty())
        QDesktopServices::openUrl(QUrl(QString::fromStdString(m_processInfo.m_repo)));
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
        QPushButton* pBackBtn = new QPushButton(QIcon(":/Images/back.png"), " ");
        pBackBtn->setToolTip(tr("Back to algorithms"));
        connect(pBackBtn, &QPushButton::clicked, [&]{ emit doBack();});
        pActionsLayout->addWidget(pBackBtn);
    }

    QPushButton* pCodeBtn = new QPushButton(QIcon(":/Images/code.png"), "Source code");
    pCodeBtn->setToolTip(tr("Open code source folder"));
    connect(pCodeBtn, &QPushButton::clicked, this, &CProcessDocWidget::onShowSourceCode);
    pActionsLayout->addWidget(pCodeBtn);

    m_pInstallBtn = new QPushButton(QIcon(":/Images/download-color.png"), "Install algorithm");
    m_pInstallBtn->setToolTip(tr("Install algorithm from Ikomia HUB"));
    connect(m_pInstallBtn, &QPushButton::clicked, [&]{ emit doInstallPlugin(); });
    pActionsLayout->addWidget(m_pInstallBtn);
    m_pInstallBtn->setVisible(false);

    pActionsLayout->addStretch(1);

    m_pDocFrame = new CProcessDocFrame;

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pDocFrame);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pActionsLayout);
    pMainLayout->addWidget(m_pStackWidget);

    setLayout(pMainLayout);
}
