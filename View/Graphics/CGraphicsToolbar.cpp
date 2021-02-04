#include "CGraphicsToolbar.h"
#include <QPushButton>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QFrame>
#include "Graphics/CGraphicsContext.h"

CGraphicsToolbar::CGraphicsToolbar(int size, QWidget *parent, Qt::WindowFlags f) : QFrame(parent, f)
{
    m_size = size;
    initLayout();
    initConnections();
}

void CGraphicsToolbar::setContext(GraphicsContextPtr &contextPtr)
{
    m_contextPtr = contextPtr;
}

void CGraphicsToolbar::contextChanged()
{
    switch(m_contextPtr->getTool())
    {
        case GraphicsShape::SELECTION: m_pSelectBtn->setChecked(true); break;
        case GraphicsShape::POINT: m_pPointBtn->setChecked(true); break;
        case GraphicsShape::ELLIPSE: m_pEllipseBtn->setChecked(true); break;
        case GraphicsShape::RECTANGLE: m_pRectBtn->setChecked(true); break;
        case GraphicsShape::POLYGON: m_pPolygonBtn->setChecked(true); break;
        case GraphicsShape::FREEHAND_POLYGON: m_pFreePolygonBtn->setChecked(true); break;
        case GraphicsShape::LINE: m_pLineBtn->setChecked(true); break;
        case GraphicsShape::POLYLINE: m_pPolylineBtn->setChecked(true); break;
        case GraphicsShape::FREEHAND_POLYLINE: m_pFreeLineBtn->setChecked(true); break;
        case GraphicsShape::TEXT: m_pTextBtn->setChecked(true); break;
    }
}

void CGraphicsToolbar::show()
{
    m_pActivateBtn->setChecked(true);
    m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-on.png"));
    emit doActivateGraphics(true);
    QFrame::show();
}

void CGraphicsToolbar::hide()
{
    m_pActivateBtn->setChecked(false);
    m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-off.png"));
    emit doActivateGraphics(false);
    QFrame::hide();
}

void CGraphicsToolbar::onActivateClicked()
{
    if(m_pActivateBtn->isChecked())
    {
        m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-on.png"));
        emit doActivateGraphics(true);
    }
    else
    {
        m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-off.png"));
        emit doActivateGraphics(false);
    }
}

void CGraphicsToolbar::onChangeActivationState(bool bActive)
{
    m_pActivateBtn->setChecked(bActive);
    if(bActive)
        m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-on.png"));
    else
        m_pActivateBtn->setIcon(QIcon(":Images/draw-checkbox-off.png"));
}

void CGraphicsToolbar::initLayout()
{
    setContentsMargins(5,1,1,1);
    setMinimumWidth(m_size);
    setMaximumWidth(m_size);

    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameShape(QFrame::VLine);
    auto pal = qApp->palette();
    pLine->setStyleSheet(QString("color:%1;").arg(pal.window().color().name()));

    m_pActivateBtn = createToolButton(tr("Graphics edition on/off"), QIcon(":Images/draw-checkbox-on.png"));
    m_pSelectBtn = createToolButton(tr("Selection"), QIcon(":Images/tool-pointer.png"));
    m_pSelectBtn->setChecked(true);
    m_pPointBtn = createToolButton(tr("Point"), QIcon(":Images/tool-point.png"));
    m_pEllipseBtn = createToolButton(tr("Circle/Ellipse"), QIcon(":Images/draw-ellipse.png"));
    m_pRectBtn = createToolButton(tr("Square/Rectangle"), QIcon(":Images/draw-rectangle.png"));
    m_pPolygonBtn = createToolButton(tr("Polygon"), QIcon(":Images/draw-polygon.png"));
    m_pFreePolygonBtn = createToolButton(tr("Free hand polygon"), QIcon(":Images/draw-hand-polygon.png"));
    m_pLineBtn = createToolButton(tr("Line"), QIcon(":Images/draw-line.png"));
    m_pPolylineBtn = createToolButton(tr("Polyline"), QIcon(":Images/draw-polyline.png"));
    m_pFreeLineBtn = createToolButton(tr("Free hand polyline"), QIcon(":Images/draw-freehand.png"));
    m_pTextBtn = createToolButton(tr("Text"), QIcon(":Images/draw-text.png"));
    m_pPropertyBtn = createToolButton(tr("Properties"), QIcon(":Images/properties_white.png"), false);

    auto pBtnGroup = new QButtonGroup(this);
    pBtnGroup->addButton(m_pSelectBtn);
    pBtnGroup->addButton(m_pPointBtn);
    pBtnGroup->addButton(m_pEllipseBtn);
    pBtnGroup->addButton(m_pRectBtn);
    pBtnGroup->addButton(m_pPolygonBtn);
    pBtnGroup->addButton(m_pFreePolygonBtn);
    pBtnGroup->addButton(m_pLineBtn);
    pBtnGroup->addButton(m_pPolylineBtn);
    pBtnGroup->addButton(m_pFreeLineBtn);
    pBtnGroup->addButton(m_pTextBtn);

    auto pLayout = new QVBoxLayout;
    pLayout->addSpacing(40);
    pLayout->addWidget(m_pActivateBtn);
    pLayout->addWidget(m_pSelectBtn);
    pLayout->addWidget(m_pPointBtn);
    pLayout->addWidget(m_pEllipseBtn);
    pLayout->addWidget(m_pRectBtn);
    pLayout->addWidget(m_pPolygonBtn);
    pLayout->addWidget(m_pFreePolygonBtn);
    pLayout->addWidget(m_pLineBtn);
    pLayout->addWidget(m_pPolylineBtn);
    pLayout->addWidget(m_pFreeLineBtn);
    pLayout->addWidget(m_pTextBtn);
    pLayout->addWidget(createSeparator());
    pLayout->addWidget(m_pPropertyBtn);
    pLayout->addStretch(1);
    pLayout->setSpacing(2);
    pLayout->setContentsMargins(0, 0, 0, 0);

    auto pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(pLine);
    pMainLayout->addLayout(pLayout);
    pMainLayout->setContentsMargins(0,0,0,0);
    pMainLayout->setSpacing(1);

    setLayout(pMainLayout);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
}

void CGraphicsToolbar::initConnections()
{
    connect(m_pActivateBtn, &QPushButton::clicked, this, &CGraphicsToolbar::onActivateClicked);
    connect(m_pSelectBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::SELECTION);});
    connect(m_pPointBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::POINT);});
    connect(m_pEllipseBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::ELLIPSE);});
    connect(m_pRectBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::RECTANGLE);});
    connect(m_pPolygonBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::POLYGON);});
    connect(m_pFreePolygonBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::FREEHAND_POLYGON);});
    connect(m_pLineBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::LINE);});
    connect(m_pPolylineBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::POLYLINE);});
    connect(m_pFreeLineBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::FREEHAND_POLYLINE);});
    connect(m_pTextBtn, &QPushButton::clicked, [&]{ emit doSetGraphicsTool(GraphicsShape::TEXT);});
    connect(m_pPropertyBtn, &QPushButton::clicked, [&]{ emit doToggleGraphicsProperties();});
}

QPushButton *CGraphicsToolbar::createToolButton(const QString& title, const QIcon& icon, bool bCheckable)
{
    auto pal = qApp->palette();
    auto hoverBorderColor = pal.highlight().color();
    auto highLightColor = pal.highlight().color();
    QColor checkedColor(highLightColor.red(), highLightColor.green(), highLightColor.blue(), 128);

    int marginL, marginT, marginR, marginB;
    getContentsMargins(&marginL, &marginT, &marginR, &marginB);

    auto pBtn = new QPushButton(icon, "");
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none; } QPushButton:hover {border: 1px solid %1;} QPushButton:checked { background-color: rgba(%2,%3,%4,%5); }")
                        .arg(hoverBorderColor.name())
                        .arg(checkedColor.red()).arg(checkedColor.green()).arg(checkedColor.blue()).arg(checkedColor.alpha()));
    pBtn->setFlat(true);
    pBtn->setToolTip(title);
    pBtn->setIconSize(QSize(m_size*m_iconSizeRate/100, m_size*m_iconSizeRate/100));
    pBtn->setCheckable(bCheckable);
    return pBtn;
}

QFrame *CGraphicsToolbar::createSeparator()
{
    auto pal = qApp->palette();
    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameShape(QFrame::HLine);
    pLine->setStyleSheet(QString("color:%1;").arg(pal.window().color().name()));
    return pLine;
}
