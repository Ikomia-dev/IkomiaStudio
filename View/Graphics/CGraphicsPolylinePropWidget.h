#ifndef CGRAPHICSPOLYLINEPROPWIDGET_H
#define CGRAPHICSPOLYLINEPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsPolylineProperty;

class CGraphicsPolylinePropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsPolylinePropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsPolylineProperty* pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        void            updateUI();

    private:

        CColorPushButton*           m_pPenColorBtn = nullptr;
        QSpinBox*                   m_pSpinSize = nullptr;
        QSpinBox*                   m_pSpinOpacity = nullptr;
        QLineEdit*                  m_pEditCategory = nullptr;
        GraphicsPolylineProperty*   m_pProperty = nullptr;
};

#endif // CGRAPHICSPOLYLINEPROPWIDGET_H
