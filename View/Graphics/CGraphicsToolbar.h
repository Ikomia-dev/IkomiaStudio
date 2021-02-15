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

#ifndef CGRAPHICSTOOLBAR_H
#define CGRAPHICSTOOLBAR_H

#include <QWidget>
#include "Main/forwards.hpp"
#include "Graphics/CGraphicsItem.hpp"

class CGraphicsContext;
using GraphicsContextPtr = std::shared_ptr<CGraphicsContext>;

class CGraphicsToolbar : public QFrame
{
    Q_OBJECT

    public:

        explicit CGraphicsToolbar(int size, QWidget *parent = nullptr, Qt::WindowFlags f=Qt::WindowFlags());

        void            setContext(GraphicsContextPtr& contextPtr);

        void            contextChanged();

    signals:

        void            doToggleGraphicsProperties();
        void            doActivateGraphics(bool bActive);
        void            doSetGraphicsTool(const GraphicsShape tool);

    public slots:

        void            show();
        void            hide();
        void            onActivateClicked();
        void            onChangeActivationState(bool bActive);

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createToolButton(const QString &title, const QIcon &icon, bool bCheckable=true);
        QFrame*         createSeparator();

    private:

        int                 m_size = 0;
        int                 m_iconSizeRate = 60;
        QPushButton*        m_pActivateBtn = nullptr;
        QPushButton*        m_pSelectBtn = nullptr;
        QPushButton*        m_pPointBtn = nullptr;
        QPushButton*        m_pEllipseBtn = nullptr;
        QPushButton*        m_pRectBtn = nullptr;
        QPushButton*        m_pPolygonBtn = nullptr;
        QPushButton*        m_pFreePolygonBtn = nullptr;
        QPushButton*        m_pLineBtn = nullptr;
        QPushButton*        m_pPolylineBtn = nullptr;
        QPushButton*        m_pFreeLineBtn = nullptr;
        QPushButton*        m_pTextBtn = nullptr;
        QPushButton*        m_pPropertyBtn = nullptr;
        GraphicsContextPtr  m_contextPtr = nullptr;
};

#endif // CGRAPHICSTOOLBAR_H
