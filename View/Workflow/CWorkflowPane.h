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

#ifndef CWORKFLOWPANE_H
#define CWORKFLOWPANE_H

#include <QWidget>
#include "Main/forwards.hpp"
#include "View/Common/CContextMenu.hpp"

class CWorkflowInfoDlg;

class CWorkflowPane : public QWidget
{
    Q_OBJECT

    public:

        explicit CWorkflowPane(QWidget *parent = nullptr);

    public slots:

        void    onSetModel(QStringListModel* pModel);
        void    onSetFromImageModel(QStringListModel* pModel);
        void    onSetDescription(const QString& text);
        void    onSearchWorkflow(const QString& text);
        void    onShowContextMenu(const QPoint &pos);
        void    onShowWorkflowInfo(const QModelIndex& index);
        void    onLoadSelectedWorkflow();
        void    onDeleteSelectedWorkflow();

    signals:

        void    doLoadWorkflow(const QModelIndex& index);
        void    doDeleteWorkflow(const QModelIndex& index);
        void    doSearchWorkflow(const QString& text);
        void    doOpenWorkflowView();
        void    doGetWorkflowInfo(const QModelIndex& index);

    protected:

        bool    eventFilter(QObject *watched, QEvent *event) override;
        void    hideEvent(QHideEvent* event) override;

    private:

        void    initLayout();
        void    initContextMenu();
        void    initConnections();

        void    deleteCurrentWorkflow();

        void    updateWorkflowInfoPosition(QPoint itemPosition);

    private:

        QLineEdit*                  m_pSearchBox = nullptr;
        QListView*                  m_pListView = nullptr;
        QListView*                  m_pFromImageListView = nullptr;
        QTextEdit*                  m_pWorkflowDesc = nullptr;
        CMultiContextMenu<menu_sig> m_contextMenu;
        CWorkflowInfoDlg*           m_pInfoDlg = nullptr;
};

#endif // CWORKFLOWPANE_H
