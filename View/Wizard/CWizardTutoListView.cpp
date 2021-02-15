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

#include "CWizardTutoListView.h"
#include "CWizardTutoListViewDelegate.h"

CWizardTutoListView::CWizardTutoListView(QWidget* parent) : QListView(parent)
{
    m_pDelegate = new CWizardTutoListViewDelegate(this);
    m_pDelegate->setSizeHint(m_itemSize);

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QListView::NoEditTriggers);
    setMouseTracking(true);
    setSpacing(5);
    setItemDelegate(m_pDelegate);

    initConnections();
}

void CWizardTutoListView::initConnections()
{
    connect(m_pDelegate, &CWizardTutoListViewDelegate::doShowInfo, [&](const QModelIndex& index){ emit doShowTutoInfo(index); });
    connect(m_pDelegate, &CWizardTutoListViewDelegate::doPlayTuto, [&](const QModelIndex& index){ emit doPlayTuto(index); });
}

void CWizardTutoListView::mouseMoveEvent(QMouseEvent *event)
{
    // update each listview item delegate look when we move
    update();

    return QListView::mouseMoveEvent(event);
}
