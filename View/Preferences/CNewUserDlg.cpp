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

#include "CNewUserDlg.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QSqlQueryModel>
#include "Model/User/CUserManager.h"

CNewUserDlg::CNewUserDlg(QSqlQueryModel *pModel, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f | Qt::FramelessWindowHint)
{
    m_pModel = pModel;
    initLayout();
    initConnections();

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(5);
    effect->setColor(qApp->palette().base().color().darker());
    setGraphicsEffect(effect);
}

void CNewUserDlg::initLayout()
{
    setAttribute(Qt::WA_TranslucentBackground);

    //Title with line underneath
    auto title = new QLabel(tr("User information"));
    title->setAlignment(Qt::AlignCenter);
    auto pLineSeparator = createLine();

    auto pLabelFullname = new QLabel(tr("Fullname"));
    m_pEditFullName = new QLineEdit;

    auto pLabelLogin = new QLabel(tr("Login"));
    m_pEditLogin = new QLineEdit;

    auto pLabelPwd = new QLabel(tr("Password"));
    m_pEditPwd = new QLineEdit;
    m_pEditPwd->setEchoMode(QLineEdit::Password);

    auto pLabelConfirmPwd = new QLabel(tr("Confirm password"));
    m_pEditConfirmPwd = new QLineEdit;
    m_pEditConfirmPwd->setEchoMode(QLineEdit::Password);

    auto pLabelRole = new QLabel(tr("Role"));
    m_pComboRole = new QComboBox;
    m_pComboRole->addItem(tr("Administrator"), CUserManager::ADMINISTRATOR);
    m_pComboRole->addItem(tr("User"), CUserManager::USER);
    m_pComboRole->setCurrentIndex(0);

    auto pGridLayout = new QGridLayout;
    pGridLayout->addWidget(pLabelFullname, 0, 0);
    pGridLayout->addWidget(m_pEditFullName, 0, 1);
    pGridLayout->addWidget(pLabelLogin, 1, 0);
    pGridLayout->addWidget(m_pEditLogin, 1, 1);
    pGridLayout->addWidget(pLabelPwd, 2, 0);
    pGridLayout->addWidget(m_pEditPwd, 2, 1);
    pGridLayout->addWidget(pLabelConfirmPwd, 3, 0);
    pGridLayout->addWidget(m_pEditConfirmPwd, 3, 1);
    pGridLayout->addWidget(pLabelRole, 4, 0);
    pGridLayout->addWidget(m_pComboRole, 4, 1);

    m_pBtnOk = new QPushButton("OK");
    m_pBtnCancel = new QPushButton(tr("Cancel"));

    auto pHLayout = new QHBoxLayout;
    pHLayout->addWidget(m_pBtnOk);
    pHLayout->addWidget(m_pBtnCancel);

    auto pVLayout = new QVBoxLayout;
    pVLayout->addWidget(title);
    pVLayout->addWidget(pLineSeparator);
    pVLayout->addLayout(pGridLayout);
    pVLayout->addLayout(pHLayout);

    auto pFrame = new QFrame;
    pFrame->setProperty("class", "CDialogStyle");
    pFrame->setLayout(pVLayout);

    auto pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(pFrame);

    setLayout(pMainLayout);
}

void CNewUserDlg::initConnections()
{
    connect(m_pBtnOk, &QPushButton::clicked, this, &CNewUserDlg::onOK);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &CNewUserDlg::reject);
    connect(m_pEditPwd, &QLineEdit::editingFinished, [&]{ m_bPwdModified = true; });
    connect(m_pEditConfirmPwd, &QLineEdit::editingFinished, [&]{ m_bPwdModified = true; });
}

QFrame *CNewUserDlg::createLine()
{
    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameStyle(QFrame::Plain);
    pLine->setFrameShape(QFrame::HLine);
    auto pal = qApp->palette();
    pLine->setStyleSheet(QString("color:%1;").arg(pal.highlight().color().name()));
    return pLine;
}

QString CNewUserDlg::getFullName() const
{
    return m_pEditFullName->text();
}

QString CNewUserDlg::getLogin() const
{
    return m_pEditLogin->text();
}

QString CNewUserDlg::getPassword() const
{
    return m_pEditPwd->text();
}

int CNewUserDlg::getRole() const
{
    return m_pComboRole->currentData().toInt();
}

bool CNewUserDlg::isPasswordModified() const
{
    return m_bPwdModified;
}

void CNewUserDlg::setUserInfo(int id, const QString &fullName, const QString &login, const QString &pwd, int role)
{
    m_bPwdModified = false;
    m_userId = id;
    m_pEditFullName->setText(fullName);
    m_pEditLogin->setText(login);
    m_pEditPwd->setText(pwd);
    m_pEditConfirmPwd->setText(pwd);
    m_pComboRole->setCurrentIndex(m_pComboRole->findData(role));
}

void CNewUserDlg::messageBox(const QString &text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.exec();
}

bool CNewUserDlg::checkUnicity() const
{
    if(m_pModel == nullptr)
        return false;

    auto newLogin = m_pEditLogin->text();
    for(int i=0; i<m_pModel->rowCount(); ++i)
    {
        auto id = m_pModel->data(m_pModel->index(i, 0)).toInt();
        if(id != m_userId)
        {
            auto login = m_pModel->data(m_pModel->index(i, 2)).toString();
            if(newLogin == login)
                return false;
        }
    }
    return true;
}

void CNewUserDlg::onOK()
{
    //Check user information
    if(m_pEditLogin->text().isEmpty())
    {
        messageBox(tr("Enter a login please"));
        return;
    }

    if(m_pEditPwd->text().isEmpty())
    {
        messageBox(tr("Enter a password please"));
        return;
    }

    if(m_bPwdModified == true && m_pEditPwd->text() != m_pEditConfirmPwd->text())
    {
        messageBox(tr("Confirmation password does not match"));
        return;
    }

    if(checkUnicity() == false)
    {
        messageBox(tr("This login already exists. Please enter a new one"));
        return;
    }
    CNewUserDlg::accept();
}
