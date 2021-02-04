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
