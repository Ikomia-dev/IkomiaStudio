#ifndef CGRAPHICSPOINTPROPERTYWIDGET_H
#define CGRAPHICSPOINTPROPERTYWIDGET_H

#include "Widgets/CColorPushButton.h"

struct GraphicsPointProperty;

class CGraphicsPointPropWidget: public QWidget
{
    public:

        CGraphicsPointPropWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            setProperties(GraphicsPointProperty* pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        void            updateUI();

    private:

        CColorPushButton*       m_pPenColorBtn = nullptr;
        CColorPushButton*       m_pBrushColorBtn = nullptr;
        QSpinBox*               m_pSpinSize = nullptr;
        QSpinBox*               m_pSpinOpacity = nullptr;
        QLineEdit*              m_pEditCategory = nullptr;
        GraphicsPointProperty*  m_pProperty = nullptr;
};

#endif // CGRAPHICSPOINTPROPERTYWIDGET_H
