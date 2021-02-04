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
