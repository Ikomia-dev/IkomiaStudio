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

#ifndef CWIZARDTUTOLISTVIEWDELEGATE_H
#define CWIZARDTUTOLISTVIEWDELEGATE_H

#include "View/Common/CListViewDelegate.h"

class CWizardTutoListViewDelegate : public CListViewDelegate
{
    Q_OBJECT

    public:

        enum Actions { NONE, INFO, PLAY };

        CWizardTutoListViewDelegate(QObject* parent = nullptr);

    signals:

        void            doShowInfo(const QModelIndex& index);
        void            doPlayTuto(const QModelIndex& index);

    protected:

        void            paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        virtual QString getActionIconPath(int action) const override;
        virtual int     getBtnAction(int index) const override;

        virtual bool    isBtnEnabled(const QModelIndex& itemIndex, int index) const override;

        virtual void    executeAction(int action, const QModelIndex& index) override;

        virtual void    showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const override;

    private:

        virtual void    initBrush() override;

        void            paintBorders(QPainter* painter, const QStyleOptionViewItem& option) const;
        void            paintIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void            paintTutoInfo(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void            paintActionButtons(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index) const;
        void            paintButton(QPainter* painter, const QRect& rc, const QString &pixmapPath, bool bHovered) const;

    private:

        const int               m_actionBtnCount = 2;
};

#endif // CWIZARDTUTOLISTVIEWDELEGATE_H
