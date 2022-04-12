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

#ifndef CMODULEDOCKWIDGET_H
#define CMODULEDOCKWIDGET_H

#include <QDockWidget>
#include "Main/forwards.hpp"

class CModuleDockWidget : public QDockWidget
{
    public:

        CModuleDockWidget(const QString &title, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
        CModuleDockWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());

        void                addModuleWidget(QWidget* pWidget, const QIcon &icon);
        void                addModuleAction(std::function<void()> processAction, const QIcon& icon, const QString &tooltip);

        void                removeModuleWidget(QWidget* pWidget);

        QToolButton*        getModuleBtn(int id);

        bool                isModuleOpen(int id);

        void                updateWindowTitle();

        void                showModule(int index);

    private slots:

        void                onActivateScrollBar();

    private:

        void                init();

        QPropertyAnimation* createAnimation(QByteArray name, QVariant start, QVariant end, int duration);

        void                animate();

        void                toggleModule(int index);

        void                desactivateScrollBar();

    private:

        bool                            m_bIsOpened = false;
        int                             m_currentHeight = 0;
        int                             m_minHeight = 0;
        QWidget*                        m_pView = nullptr;
        CToolbarBorderLayout*           m_pLayout = nullptr;
        QStackedWidget*                 m_pModuleViews = nullptr;
        QVector<QPropertyAnimation*>    m_animations;
        QList<QToolButton*>             m_moduleBtnList;
};

#endif // CMODULEDOCKWIDGET_H
