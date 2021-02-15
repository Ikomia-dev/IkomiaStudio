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

#ifndef CPROTOCOLPANE_H
#define CPROTOCOLPANE_H

#include <QWidget>
#include "Main/forwards.hpp"
#include "View/Common/CContextMenu.hpp"

class CProtocolInfoDlg;

class CProtocolPane : public QWidget
{
    Q_OBJECT

    public:

        explicit CProtocolPane(QWidget *parent = nullptr);

    public slots:

        void    onSetModel(QStringListModel* pModel);
        void    onSetFromImageModel(QStringListModel* pModel);
        void    onSetDescription(const QString& text);
        void    onSearchProtocol(const QString& text);
        void    onShowContextMenu(const QPoint &pos);
        void    onShowProtocolInfo(const QModelIndex& index);
        void    onLoadSelectedProtocol();
        void    onDeleteSelectedProtocol();

    signals:

        void    doLoadProtocol(const QModelIndex& index);
        void    doDeleteProtocol(const QModelIndex& index);
        void    doSearchProtocol(const QString& text);
        void    doOpenProtocolView();
        void    doGetProtocolInfo(const QModelIndex& index);

    protected:

        bool    eventFilter(QObject *watched, QEvent *event) override;
        void    hideEvent(QHideEvent* event) override;

    private:

        void    initLayout();
        void    initContextMenu();
        void    initConnections();

        void    deleteCurrentProtocol();

        void    updateProtocolInfoPosition(QPoint itemPosition);

    private:

        QLineEdit*                  m_pSearchBox = nullptr;
        QListView*                  m_pListView = nullptr;
        QListView*                  m_pFromImageListView = nullptr;
        QTextEdit*                  m_pProtocolDesc = nullptr;
        CMultiContextMenu<menu_sig> m_contextMenu;
        CProtocolInfoDlg*           m_pInfoDlg = nullptr;
};

#endif // CPROTOCOLPANE_H
