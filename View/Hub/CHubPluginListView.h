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

#ifndef CHUBPLUGINLISTVIEW_H
#define CHUBPLUGINLISTVIEW_H

#include <QListView>

class CHubPluginListViewDelegate;

class CHubPluginListView : public QListView
{
    Q_OBJECT

    public:

        CHubPluginListView(int pluginSource, QWidget *parent = nullptr);

    signals:

        void            doPublishPlugin(const QModelIndex& index);
        void            doInstallPlugin(const QModelIndex& index);
        void            doShowPluginInfo(const QModelIndex& index);

    protected:

        virtual void    mouseMoveEvent(QMouseEvent* event) override;

    private:

        void            initConnections();

    private:

        CHubPluginListViewDelegate*   m_pDelegate = nullptr;
};

#endif // CHUBPLUGINLISTVIEW_H
