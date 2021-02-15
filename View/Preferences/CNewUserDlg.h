/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CNEWUSERWIDGET_H
#define CNEWUSERWIDGET_H

class QSqlQueryModel;

class CNewUserDlg : public QDialog
{
    Q_OBJECT

    public:

        explicit CNewUserDlg(QSqlQueryModel* pModel, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString getFullName() const;
        QString getLogin() const;
        QString getPassword() const;
        int     getRole() const;

        bool    isPasswordModified() const;

        void    setUserInfo(int id, const QString& fullName, const QString& login, const QString& pwd, int role);

    private:

        void    initLayout();
        void    initConnections();

        QFrame* createLine();

        void    messageBox(const QString& text);

        bool    checkUnicity() const;

    private slots:

        void    onOK();

    private:

        QSqlQueryModel* m_pModel = nullptr;
        QLineEdit*      m_pEditFullName = nullptr;
        QLineEdit*      m_pEditLogin = nullptr;
        QLineEdit*      m_pEditPwd = nullptr;
        QLineEdit*      m_pEditConfirmPwd = nullptr;
        QComboBox*      m_pComboRole = nullptr;
        QPushButton*    m_pBtnOk = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        int             m_userId = -1;
        bool            m_bPwdModified = false;
};

#endif // CNEWUSERWIDGET_H
