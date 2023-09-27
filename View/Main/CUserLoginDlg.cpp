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

#include "CUserLoginDlg.h"

CUserLoginDlg::CUserLoginDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(parent, CDialog::NO_TITLE_BAR, f)
{
    setWindowOpacity(0.9);
    initLayout();
    initConnections();
}

void CUserLoginDlg::setCurrentUser(const QString &userName)
{
    m_userName = userName;

    if(m_userName.isEmpty() == true)
    {
        m_pBtnConnect->setText(tr("Connect"));
        m_pEditLogin->setText("");
        m_pEditLogin->setReadOnly(false);
        m_pEditPwd->setText("");
        m_pEditPwd->setReadOnly(false);
        m_pCheckRememberMe->setEnabled(true);
    }
    else
    {
        m_pBtnConnect->setText(tr("Disconnect"));
        m_pEditLogin->setText(m_userName);
        m_pEditLogin->setReadOnly(true);
        m_pEditPwd->setReadOnly(true);
        m_pCheckRememberMe->setEnabled(false);
    }
}

void CUserLoginDlg::initLayout()
{
    auto pLabelLogin = new QLabel(tr("Login"));
    m_pEditLogin = new QLineEdit;

    auto pLabelPwd = new QLabel(tr("Password"));
    m_pEditPwd = new QLineEdit;
    m_pEditPwd->setEchoMode(QLineEdit::Password);

    m_pCheckRememberMe = new QCheckBox(tr("Remember me"));

    m_pBtnConnect = new QPushButton(tr("Connect"));

    auto pLabelSignUp = new QLabel;
    pLabelSignUp->setText(tr("<a href=\"https://app.ikomia.ai/signup/\">Go to sign up page</a>"));
    pLabelSignUp->setTextFormat(Qt::RichText);
    pLabelSignUp->setTextInteractionFlags(Qt::TextBrowserInteraction);
    pLabelSignUp->setOpenExternalLinks(true);
    pLabelSignUp->setAlignment(Qt::AlignHCenter);

    auto pGridLayout = new QGridLayout;
    pGridLayout->addWidget(pLabelLogin, 0, 0);
    pGridLayout->addWidget(m_pEditLogin, 0, 1);
    pGridLayout->addWidget(pLabelPwd, 1, 0);
    pGridLayout->addWidget(m_pEditPwd, 1, 1);
    pGridLayout->addWidget(m_pCheckRememberMe, 2, 0, 1, 2);
    pGridLayout->addWidget(m_pBtnConnect, 3, 0, 1, 2);
    pGridLayout->addWidget(pLabelSignUp, 4, 0, 1, 2);

    auto pLayout = getContentLayout();
    pLayout->addLayout(pGridLayout);
}

void CUserLoginDlg::initConnections()
{
    connect(m_pBtnConnect, &QPushButton::clicked, [&]
    {
        if(m_pBtnConnect->text() == tr("Connect"))
            emit doConnectUser(m_pEditLogin->text(), m_pEditPwd->text(), m_pCheckRememberMe->isChecked());
        else
            emit doDisconnectUser();
    });
}

void CUserLoginDlg::hideEvent(QHideEvent* event)
{
    emit doHide();
    CDialog::hideEvent(event);
}
