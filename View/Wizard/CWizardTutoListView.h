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

#ifndef CWIZARDTUTOLISTVIEW_H
#define CWIZARDTUTOLISTVIEW_H

#include <QListView>

class CWizardTutoListViewDelegate;

class CWizardTutoListView : public QListView
{
    Q_OBJECT

    public:

        CWizardTutoListView(QWidget* parent = nullptr);

    signals:

        void    doShowTutoInfo(const QModelIndex& index);
        void    doPlayTuto(const QModelIndex& index);

    protected:
        virtual void    mouseMoveEvent(QMouseEvent* event) override;

    private:

        void    initConnections();

    private:

        QSize                           m_itemSize = QSize(150, 120);
        CWizardTutoListViewDelegate*    m_pDelegate = nullptr;
};

#endif // CWIZARDTUTOLISTVIEW_H
