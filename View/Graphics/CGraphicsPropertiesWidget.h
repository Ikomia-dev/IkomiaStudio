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

#ifndef CGRAPHICSPROPERTIESWIDGET_H
#define CGRAPHICSPROPERTIESWIDGET_H

#include "Graphics/CGraphicsItem.hpp"

class CGraphicsPointPropWidget;
class CGraphicsEllipsePropWidget;
class CGraphicsRectPropWidget;
class CGraphicsPolygonPropWidget;
class CGraphicsPolylinePropWidget;
class CGraphicsTextPropWidget;
class CGraphicsContext;
using GraphicsContextPtr = std::shared_ptr<CGraphicsContext>;

class CGraphicsPropertiesWidget : public QStackedWidget
{
    public:

        CGraphicsPropertiesWidget(QWidget* parent=Q_NULLPTR);

        void    setContext(GraphicsContextPtr& contextPtr);
        void    setTool(GraphicsShape tool);

        void    contextChanged();

    private:

        void    init();

    private:

        QWidget*                        m_pEmpty = nullptr;
        CGraphicsPointPropWidget*       m_pPointProp = nullptr;
        CGraphicsEllipsePropWidget*     m_pEllipseProp = nullptr;
        CGraphicsRectPropWidget*        m_pRectProp = nullptr;
        CGraphicsPolygonPropWidget*     m_pPolygonProp = nullptr;
        CGraphicsPolylinePropWidget*    m_pPolylineProp = nullptr;
        CGraphicsTextPropWidget*        m_pTextProp = nullptr;
        QMap<GraphicsShape, QWidget*>   m_widgetMap;
};

#endif // CGRAPHICSPROPERTIESWIDGET_H
