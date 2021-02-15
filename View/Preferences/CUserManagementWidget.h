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

#ifndef CUSERMANAGEMENTWIDGET_H
#define CUSERMANAGEMENTWIDGET_H

#include <QObject>
#include <QWidget>

class QSqlQueryModel;

class CUserManagementWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CUserManagementWidget(QWidget *parent = nullptr);

        void            setModel(QSqlQueryModel* pModel);
        void            setCurrentUserInfo(int id, int role);

        virtual QSize   sizeHint() const override;

    signals:

        void            doAddUser(const QString& fullname, const QString& login, const QString& pwd, int role);
        void            doDeleteUser(int id);
        void            doModifyUser(int id, const QString& fullname, const QString& login, const QString& pwd, int role);

    private slots:

        void            onNewUser();
        void            onEditUser();
        void            onDeleteUser();
        void            onTableViewClicked(const QModelIndex& index);

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QString& text, const QIcon& icon);

        void            enableAdministration(bool bEnable);

    private:

        int             m_currentUserId = -1;
        int             m_currentUserRole = -1;
        QTableView*     m_pUserTable = nullptr;
        QPushButton*    m_pBtnNew = nullptr;
        QPushButton*    m_pBtnEdit = nullptr;
        QPushButton*    m_pBtnDelete = nullptr;
};

#endif // CUSERMANAGEMENTWIDGET_H
