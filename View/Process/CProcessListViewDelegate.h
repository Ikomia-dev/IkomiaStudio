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

#ifndef CPROCESSLISTVIEWDELEGATE_H
#define CPROCESSLISTVIEWDELEGATE_H

#include "View/Common/CListViewDelegate.h"

/**
 * @brief
 *
 */
class CProcessListViewDelegate : public CListViewDelegate
{
    Q_OBJECT

    public:

        enum Actions { NONE, ADD, INFO };

        CProcessListViewDelegate(QObject* parent = nullptr);

    signals:

        void            doShowInfo(const QModelIndex& index);
        void            doAddProcess(const QString& processName);

    protected:

        virtual int     getBtnAction(int index) const override;
        virtual QString getActionIconPath(int action) const override;

        virtual bool    isBtnEnabled(const QModelIndex& itemIndex, int index) const override;

        virtual void    executeAction(int action, const QModelIndex& index) override;

        virtual void    showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const override;

    private:

        void            paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        virtual void    paintText(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const;
        virtual void    paintLanguageIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        virtual void    paintOSIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // CPROCESSLISTVIEWDELEGATE_H
