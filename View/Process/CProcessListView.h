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

#ifndef CPROCESSLISTVIEW_H
#define CPROCESSLISTVIEW_H

#include "Main/forwards.hpp"
#include "Model/CMainModel.h"
#include <QWidget>

class CProcessListViewDelegate;

class CProcessListView : public QWidget
{
        Q_OBJECT
    public:

        CProcessListView();

        void                        setModel(QAbstractItemModel* pModel);
        void                        setModelColumn(int column);

        QListView*                  getListView();
        QSize                       getPluginSize();

    signals:

        void                        doListViewDoubleCLicked(const QString& processName);
        void                        doListViewLicked(const QModelIndex& index);        
        void                        doAddProcess(const QString& processName);
        void                        doShowProcessInfo(const QModelIndex& index);
        void                        doCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    public slots:

        void                        onUpdateModel(QAbstractItemModel* pModel);
        void                        onListViewDoubleClicked(const QModelIndex& index);
        void                        onListViewClicked(const QModelIndex& index);
        void                        onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
        void                        onTreeViewClicked(const QModelIndex& index);

    protected:

        bool                        eventFilter(QObject *watched, QEvent *event) override;

    private:

        void                        initConnections();

    private:

        QListView*                  m_pListView = nullptr;
        QHBoxLayout*                m_pLayout = nullptr;
        CProcessListViewDelegate*   m_pDelegate = nullptr;
        QSize                       m_itemSize = QSize(300,150);
};

#endif // CPROCESSLISTVIEW_H
