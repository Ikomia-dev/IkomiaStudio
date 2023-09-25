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

#include "CHubPluginListView.h"
#include "CHubPluginListViewDelegate.h"

CHubPluginListView::CHubPluginListView(int pluginSource, QWidget *parent) : QListView(parent)
{
    m_pDelegate = new CHubPluginListViewDelegate(pluginSource, this);

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setEditTriggers(QListView::NoEditTriggers);
    setMouseTracking(true);
    setSpacing(20);
    setItemDelegate(m_pDelegate);

    initConnections();
}

void CHubPluginListView::mouseMoveEvent(QMouseEvent *event)
{
    // update each listview item delegate look when we move
    update();
    return QListView::mouseMoveEvent(event);
}

void CHubPluginListView::initConnections()
{
    connect(m_pDelegate, &CHubPluginListViewDelegate::doShowInfo, [&](const QModelIndex& index){ emit doShowPluginInfo(index); });
    connect(m_pDelegate, &CHubPluginListViewDelegate::doInstallPlugin, [&](const QModelIndex& index){ emit doInstallPlugin(index); });
    connect(m_pDelegate, &CHubPluginListViewDelegate::doPublishPlugin, [&](const QModelIndex& index){ emit doPublishPlugin(index); });
}
