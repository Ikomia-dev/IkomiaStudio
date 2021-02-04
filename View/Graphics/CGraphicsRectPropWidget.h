#ifndef CGRAPHICSRECTPROPWIDGET_H
#define CGRAPHICSRECTPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsRectProperty;

class CGraphicsRectPropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsRectPropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsRectProperty *pProp);

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
        GraphicsRectProperty*   m_pProperty = nullptr;
};

#endif // CGRAPHICSRECTPROPWIDGET_H
