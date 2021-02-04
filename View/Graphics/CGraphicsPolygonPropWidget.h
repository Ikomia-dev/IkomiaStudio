#ifndef CGRAPHICSPOLYGONPROPWIDGET_H
#define CGRAPHICSPOLYGONPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsPolygonProperty;

class CGraphicsPolygonPropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsPolygonPropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsPolygonProperty *pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        void            updateUI();

    private:

        CColorPushButton*           m_pPenColorBtn = nullptr;
        CColorPushButton*           m_pBrushColorBtn = nullptr;
        QSpinBox*                   m_pSpinSize = nullptr;
        QSpinBox*                   m_pSpinOpacity = nullptr;
        QLineEdit*                  m_pEditCategory = nullptr;
        GraphicsPolygonProperty*    m_pProperty = nullptr;
};

#endif // CGRAPHICSPOLYGONPROPWIDGET_H
