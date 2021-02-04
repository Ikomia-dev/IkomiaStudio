#ifndef CGRAPHICSELLIPSEPROPWIDGET_H
#define CGRAPHICSELLIPSEPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsEllipseProperty;

class CGraphicsEllipsePropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsEllipsePropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsEllipseProperty* pProp);

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
        GraphicsEllipseProperty*    m_pProperty = nullptr;
};

#endif // CGRAPHICSELLIPSEPROPWIDGET_H
