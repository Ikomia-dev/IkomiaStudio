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

#ifndef CDATALISTVIEW_H
#define CDATALISTVIEW_H

/**
 * @file      CDataListView.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataListView definition
 *
 * @details   Details
 */

#include "Main/forwards.hpp"
#include <QWidget>


class CDataListView : public QWidget
{
    Q_OBJECT
    public:

        CDataListView();

        void                    setSourceModel(QAbstractItemModel* pModel);

        void                    updateIndex(const QModelIndex& index);

        QAbstractProxyModel*    proxyModel();

    signals:

        void                    doDisplayData(const QModelIndex& index);

        void                    doUpdateIndex(const QModelIndex& index);

    public slots:

        void                    onUpdateSourceModel(QAbstractItemModel* pModel);
        void                    onListViewDoubleClicked(const QModelIndex& index);
        void                    onListViewClicked(const QModelIndex& index);

    protected:

        bool                    eventFilter(QObject *watched, QEvent *event) override;

    private:

        void                    initConnections();

        QModelIndex             getImageIndex(const QModelIndex& imgChildIndex);

    private:

        QListView*              m_pListView = nullptr;
        QHBoxLayout*            m_pLayout = nullptr;
        CProjectViewProxyModel* m_pProxyModel = nullptr;
};

#endif // CDATALISTVIEW_H
