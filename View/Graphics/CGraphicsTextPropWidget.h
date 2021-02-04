#ifndef CGRAPHICSTEXTPROPWIDGET_H
#define CGRAPHICSTEXTPROPWIDGET_H

#include <QWidget>
#include "Widgets/CColorPushButton.h"

struct GraphicsTextProperty;

class CGraphicsTextPropWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CGraphicsTextPropWidget(QWidget *parent = nullptr);

        void            setProperties(GraphicsTextProperty* pProp);

        void            propertyChanged();

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createToolButton(const QString& title, const QIcon& icon, bool bCheckable);

        void            updateUI();

    private:

        CColorPushButton*       m_pColorBtn = nullptr;
        QPushButton*            m_pBoldBtn = nullptr;
        QPushButton*            m_pItalicBtn = nullptr;
        QPushButton*            m_pUnderlineBtn = nullptr;
        QPushButton*            m_pStrikeOutBtn = nullptr;
        QFontComboBox*          m_pComboFont = nullptr;
        QSpinBox*               m_pSpinFontSize = nullptr;
        GraphicsTextProperty*   m_pProperty = nullptr;
};

#endif // CGRAPHICSTEXTPROPWIDGET_H
