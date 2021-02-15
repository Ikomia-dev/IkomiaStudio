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

#ifndef CSTOREPLUGINLISTVIEWDELEGATE_H
#define CSTOREPLUGINLISTVIEWDELEGATE_H

#include "View/Common/CListViewDelegate.h"

class CStoreQueryModel;

class CStorePluginListViewDelegate: public CListViewDelegate
{
    Q_OBJECT

    public:

        enum Actions { NONE, INFO, INSTALL, PUBLISH };
        enum PluginSource { SERVER, LOCAL };

        CStorePluginListViewDelegate(int pluginSource, QObject* parent = nullptr);

    signals:

        void                doShowInfo(const QModelIndex& index);
        void                doPublishPlugin(const QModelIndex& index);
        void                doInstallPlugin(const QModelIndex& index);

    protected:

        void                paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        virtual bool        editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

        virtual int         getBtnAction(int index) const override;
        virtual QString     getActionIconPath(int action) const override;

        virtual bool        isBtnEnabled(const QModelIndex& itemIndex, int index) const override;

        virtual void        executeAction(int action, const QModelIndex& index) override;

        virtual void        showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const override;

    private:

        virtual QPolygon    getRibbonRect(const QStyleOptionViewItem& option) const;

        virtual void        paintText(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        virtual void        paintLanguageIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        virtual void        paintOSIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        virtual void        paintStars(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        virtual QRect       paintName(QPainter* painter, const QStyleOptionViewItem &option, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const;
        virtual QRect       paintShortDescription(QPainter* painter, int left, int top, int width, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const;
        virtual QRect       paintContributor(QPainter* painter, int left, int top, int width, const CStoreQueryModel* pModel, const QModelIndex& index, QFont font, const QColor& color) const;
        virtual void        paintCertification(QPainter* painter, const QStyleOptionViewItem& option, const CStoreQueryModel* pModel, const QModelIndex& index) const;

        bool                isDeprecated(const QModelIndex& index) const;

    private:

        int                 m_source = LOCAL;
        int                 m_ribbonSize = 96;
};

#endif // CSTOREPLUGINLISTVIEWDELEGATE_H
