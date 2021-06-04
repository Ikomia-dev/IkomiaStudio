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

#include "CWorkflowNewDlg.h"
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>

CWorkflowNewDlg::CWorkflowNewDlg(const QString &name, const QStringList &names, QWidget *parent, Qt::WindowFlags f) :
    CDialog(tr("Workflow information"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_name = name;
    m_protocolNames = names;
    initLayout();
    initConnections();
}

QString CWorkflowNewDlg::getName() const
{
    return m_name;
}

QString CWorkflowNewDlg::getKeywords() const
{
    return m_keywords;
}

QString CWorkflowNewDlg::getDescription() const
{
    return m_description;
}

void CWorkflowNewDlg::initLayout()
{
    QLabel* pLabelName = new QLabel(tr("Name"));
    m_pEditName = new QLineEdit;
    m_pEditName->setText(m_name);
    QTimer::singleShot(0, m_pEditName, SLOT(setFocus()));

    QLabel* pLabelKeywords = new QLabel(tr("Keywords"));
    m_pEditKeywords = new QPlainTextEdit;

    QLabel* pLabelDescription = new QLabel(tr("Description"));
    m_pEditDescription = new QPlainTextEdit;

    QGridLayout* pCentralLayout = new QGridLayout;
    pCentralLayout->addWidget(pLabelName, 0, 0);
    pCentralLayout->addWidget(m_pEditName, 0, 1);
    pCentralLayout->addWidget(pLabelKeywords, 1, 0);
    pCentralLayout->addWidget(m_pEditKeywords, 1, 1);
    pCentralLayout->addWidget(pLabelDescription, 2, 0);
    pCentralLayout->addWidget(m_pEditDescription, 2, 1);

    m_pBtnOk = new QPushButton(tr("OK"));
    m_pBtnOk->setDefault(true);

    m_pBtnCancel = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pBtnOk);
    pBtnLayout->addWidget(m_pBtnCancel);

    QVBoxLayout* pMainLayout = getContentLayout();
    pMainLayout->addLayout(pCentralLayout);
    pMainLayout->addLayout(pBtnLayout);
}

void CWorkflowNewDlg::initConnections()
{
    connect(m_pBtnOk, &QPushButton::clicked, this, &CWorkflowNewDlg::validate);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &CWorkflowNewDlg::reject);
}

void CWorkflowNewDlg::validate()
{
    m_name = m_pEditName->text();
    m_keywords = m_pEditKeywords->toPlainText();
    m_description = m_pEditDescription->toPlainText();

    if(m_name.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("You must at least set the workflow name please."));
        msgBox.exec();
    }
    else if(m_protocolNames.contains(m_name))
    {
        //Unicity constraint
        QMessageBox msgBox;
        msgBox.setText(tr("This workflow already exists. Please enter another name."));
        msgBox.exec();
    }
    else
        emit accept();
}
