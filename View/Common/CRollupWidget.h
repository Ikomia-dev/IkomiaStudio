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

#ifndef CROLLUPWIDGET_H
#define CROLLUPWIDGET_H

#include <QWidget>

class QPushButton;
class QWidget;
class QFrame;
class QHBoxLayout;
class CRollupBtn;

class CRollupWidget : public QWidget
{
    public:

        CRollupWidget(QWidget* parent = Q_NULLPTR);

        void        addWidget(const QIcon& icon, const QString& name, QWidget* pWidget);

        QFrame*     makeFrameWidget(QWidget* pWidget);

        void        insertWidget(int position, QString name, QWidget* pWidget);

        QWidget*    replaceWidget(int position, QString newName, QWidget* pWidget);

        void        removeWidget(QWidget *pWidget);
        QWidget*    removeWidget(int position);

        void        expand(int position);
        void        expand(QWidget* pWidget);
        void        expandAll();

        void        collapse(int position);
        void        collapse(QWidget* pWidget);
        void        collapseAll();

    private:

        QList<CRollupBtn*>  m_buttons;
        QList<QWidget*>     m_widgets;
        QWidget*            m_pContainer;
};

#endif // CROLLUPWIDGET_H
