#include "CGraphicsPropertiesWidget.h"
#include <QMap>
#include "Main/AppDefine.hpp"
#include "CGraphicsPointPropWidget.h"
#include "CGraphicsEllipsePropWidget.h"
#include "CGraphicsRectPropWidget.h"
#include "CGraphicsPolygonPropWidget.h"
#include "CGraphicsPolylinePropWidget.h"
#include "CGraphicsTextPropWidget.h"
#include "Graphics/CGraphicsContext.h"
#include "Graphics/CGraphicsPoint.h"

CGraphicsPropertiesWidget::CGraphicsPropertiesWidget(QWidget *parent) : QStackedWidget(parent)
{
    init();
}

void CGraphicsPropertiesWidget::setContext(GraphicsContextPtr &contextPtr)
{
    if(contextPtr)
    {
        m_pPointProp->setProperties(contextPtr->getPointPropertyPtr());
        m_pEllipseProp->setProperties(contextPtr->getEllipsePropertyPtr());
        m_pRectProp->setProperties(contextPtr->getRectPropertyPtr());
        m_pPolygonProp->setProperties(contextPtr->getPolygonPropertyPtr());
        m_pPolylineProp->setProperties(contextPtr->getPolylinePropertyPtr());
        m_pTextProp->setProperties(contextPtr->getTextPropertyPtr());
    }
}

void CGraphicsPropertiesWidget::setTool(GraphicsShape tool)
{
    auto it = m_widgetMap.find(tool);
    if(it == m_widgetMap.end())
        setCurrentWidget(m_pEmpty);
    else
        setCurrentWidget(it.value());
}

void CGraphicsPropertiesWidget::contextChanged()
{
    m_pPointProp->propertyChanged();
    m_pEllipseProp->propertyChanged();
    m_pRectProp->propertyChanged();
    m_pPolygonProp->propertyChanged();
    m_pPolylineProp->propertyChanged();
    m_pTextProp->propertyChanged();
}

void CGraphicsPropertiesWidget::init()
{
    setFixedWidth(175);

    m_pEmpty = new QWidget();
    m_pPointProp = new CGraphicsPointPropWidget(this);
    m_widgetMap.insert(Ikomia::GraphicsShape::POINT, m_pPointProp);

    m_pEllipseProp = new CGraphicsEllipsePropWidget(this);
    m_widgetMap.insert(GraphicsShape::ELLIPSE, m_pEllipseProp);

    m_pRectProp = new CGraphicsRectPropWidget(this);
    m_widgetMap.insert(GraphicsShape::RECTANGLE, m_pRectProp);

    m_pPolygonProp = new CGraphicsPolygonPropWidget(this);
    m_widgetMap.insert(GraphicsShape::POLYGON, m_pPolygonProp);
    m_widgetMap.insert(GraphicsShape::FREEHAND_POLYGON, m_pPolygonProp);

    m_pPolylineProp = new CGraphicsPolylinePropWidget(this);
    m_widgetMap.insert(GraphicsShape::LINE, m_pPolylineProp);
    m_widgetMap.insert(GraphicsShape::POLYLINE, m_pPolylineProp);
    m_widgetMap.insert(GraphicsShape::FREEHAND_POLYLINE, m_pPolylineProp);

    m_pTextProp = new CGraphicsTextPropWidget(this);
    m_widgetMap.insert(GraphicsShape::TEXT, m_pTextProp);

    addWidget(m_pEmpty);
    addWidget(m_pPointProp);
    addWidget(m_pEllipseProp);
    addWidget(m_pRectProp);
    addWidget(m_pPolygonProp);
    addWidget(m_pPolylineProp);
    addWidget(m_pTextProp);
}
