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

#ifndef CPROCESSLISTPOPUP_H
#define CPROCESSLISTPOPUP_H

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QListView>
#include "Model/Process/CProcessModel.hpp"
#include "View/Common/CDialog.h"

class QListView;
class QSortFilterProxyModel;
class QFrame;
class QLabel;

class CResizeListView : public QListView
{
    public:
        QSize sizeHint() const override
        {
            if (model()->rowCount() == 0)
                return QSize(width(), 0);

            int nToShow = m_nItemsToShow < model()->rowCount() ? m_nItemsToShow : model()->rowCount();
            // Get height of name column
            int height = sizeHintForIndex(model()->index(0, 1)).height();
            return QSize(sizeHintForColumn(1) + 10, nToShow * height);
        }

        QSize minimumSizeHint() const override
        {
            return QListView::minimumSizeHint();
        }

    private:

        int m_nItemsToShow = 100;
};

class CProcessListProxyModel : public QSortFilterProxyModel
{
    public:

        CProcessListProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent){}

    protected:

        virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override
        {;
            // check the current item
            QModelIndex currIndex = sourceModel()->index(source_row, 0, source_parent);
            if(!currIndex.isValid())
                return false;

            /*auto pItem = static_cast<CProcessModel::TreeItem*>(currSrcIndex.internalPointer());
            if(pItem->getTypeId() != TreeItemType::PROCESS)
                return true;*/

            return true;
        }
};

class CProcessListPopup : public CDialog
{
    Q_OBJECT

    public:
        CProcessListPopup(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void                setModel(QSortFilterProxyModel* pModel);
        void                setCurrentCategory(QString name);

        bool                isCurrent(QString name) const;

        CResizeListView*    getListView();

        void                resizeListView();

    signals:

        void                doShowWidget(const QString& name);

    public slots:

        void                onListViewClicked(const QModelIndex& index);

    private:

        void                initLayout();
        void                initConnections();

    private:

        CResizeListView*        m_pProcessList = nullptr;
        QLabel*                 m_pLabel = nullptr;
        QString                 m_currentCategory;
};

#endif // CPROCESSLISTPOPUP_H
