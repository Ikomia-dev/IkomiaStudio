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
#include "CProcessEditDocFrame.h"
#include "Model/Plugin/CPluginTools.h"

CProcessDocWidget::CProcessDocWidget(int actions, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_actions = actions;
    initLayout();
    initConnections();
}

void CProcessDocWidget::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

void CProcessDocWidget::setProcessInfo(const CTaskInfo& info)
{
    m_processInfo = info;

    if(m_pDocFrame)
        m_pDocFrame->setProcessInfo(info);

    if(m_pEditDocFrame)
    {
        m_pEditDocFrame->setCurrentUser(m_currentUser);
        m_pEditDocFrame->setProcessInfo(info);
    }
}

void CProcessDocWidget::onShowSourceCode()
{
    if(m_processInfo.m_language == CTaskInfo::PYTHON)
    {
        auto pluginDir = Utils::CPluginTools::getPythonPluginFolder(QString::fromStdString(m_processInfo.m_name));
        if(!pluginDir.isEmpty() && Utils::File::isFileExist(pluginDir.toStdString()))
            Utils::File::showLocation(pluginDir);
        else if(!m_processInfo.m_repo.empty())
            QDesktopServices::openUrl(QUrl(QString::fromStdString(m_processInfo.m_repo)));
    }
    else if(!m_processInfo.m_repo.empty())
        QDesktopServices::openUrl(QUrl(QString::fromStdString(m_processInfo.m_repo)));
}

void CProcessDocWidget::onExportDocumentation()
{
    QString pluginDir;;
    if(m_processInfo.m_language == CTaskInfo::CPP)
        pluginDir = Utils::CPluginTools::getCppPluginFolder(QString::fromStdString(m_processInfo.m_name));
    else
        pluginDir = Utils::CPluginTools::getPythonPluginFolder(QString::fromStdString(m_processInfo.m_name));

    auto path = Utils::File::saveFile(this, tr("Export documentation"), pluginDir, tr("HTML (*.html *.htm);;Markdown (*.md)"), QStringList({"html", "htm", "md"}), ".html");
    if(!path.isEmpty())
        m_pDocFrame->saveContent(path);
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
        pBackBtn->setToolTip(tr("Back to process store"));
        connect(pBackBtn, &QPushButton::clicked, [&]{ emit doBack();});
        pActionsLayout->addWidget(pBackBtn);
    }

    if(m_actions & EDIT)
    {
        QPushButton* pEditBtn = new QPushButton(QIcon(":/Images/edit.png"), " ");
        pEditBtn->setToolTip(tr("Edit documentation"));
        connect(pEditBtn, &QPushButton::clicked, [&]{ m_pStackWidget->setCurrentIndex(1); });
        pActionsLayout->addWidget(pEditBtn);
    }

    QPushButton* pCodeBtn = new QPushButton(QIcon(":/Images/code.png"), "Source code");
    pCodeBtn->setToolTip(tr("Open code source folder"));
    connect(pCodeBtn, &QPushButton::clicked, this, &CProcessDocWidget::onShowSourceCode);
    pActionsLayout->addWidget(pCodeBtn);

    m_pExportDocBtn = new QPushButton(QIcon(":/Images/export.png"), "Export documentation");
    m_pExportDocBtn->setToolTip(tr("Export algorithm documentation to file"));
    connect(m_pExportDocBtn, &QPushButton::clicked, this, &CProcessDocWidget::onExportDocumentation);
    pActionsLayout->addWidget(m_pExportDocBtn);

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
    connect(m_pEditDocFrame, &CProcessEditDocFrame::doSave, [&](bool bFullEdit, const CTaskInfo& info)
    {
        emit doSave(bFullEdit, info);
        m_pDocFrame->setProcessInfo(info);
        m_pStackWidget->setCurrentIndex(0);
    });
}
