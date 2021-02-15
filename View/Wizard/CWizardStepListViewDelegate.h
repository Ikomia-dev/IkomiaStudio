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

#ifndef CWIZARDSTEPLISTVIEWDELEGATE_H
#define CWIZARDSTEPLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>

class CWizardStepListViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:

        CWizardStepListViewDelegate(QObject* parent = nullptr);

    protected:

        void    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        QSize   sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    private:

        void    paintBorders(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex & index) const;
        void    paintInfo(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:

        QSize   m_size = QSize(INT_MAX, 32);
};

#endif // CWIZARDSTEPLISTVIEWDELEGATE_H
