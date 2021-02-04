#include "CProtocolLabelArea.h"
#include <QApplication>

CProtocolLabelArea::CProtocolLabelArea(QString label, QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    m_label = label;
    initArea();
    initText();
}

int CProtocolLabelArea::type() const
{
    return Type;
}

QSize CProtocolLabelArea::getSize() const
{
    return m_size;
}

void CProtocolLabelArea::setSize(QSize size)
{
    m_size = size;
}

void CProtocolLabelArea::setTextColor(QColor color)
{
    m_textColor = color;
}

void CProtocolLabelArea::setColorBg(QColor color)
{
    m_bckColor = color;
}

void CProtocolLabelArea::setLineColor(QColor color)
{
    m_lineColor = color;
}

void CProtocolLabelArea::initText()
{
    QFont font;
    font.setStyleHint(QFont::System, QFont::PreferAntialias);
    font.setPointSize(10);
    QFontMetrics fm(font);
    m_topContent = m_topMargin + fm.height();
    QPointF position(m_size.width()/2 - fm.horizontalAdvance(m_label)/2, m_topContent);
    QPainterPath p;
    p.addText(position, font, m_label);
    m_textPath = p;
}

void CProtocolLabelArea::initArea()
{
    m_areaGradient.setStart(QPointF(m_size.width()/2, 0));
    m_areaGradient.setFinalStop(QPointF(m_size.width()/2, m_size.height()));

    m_areaGradient.setColorAt(0.00, QColor(64,64,64));
    m_areaGradient.setColorAt(0.01, QColor(50,50,50));
    m_areaGradient.setColorAt(0.05, QColor(32,32,32));
    m_areaGradient.setColorAt(0.10, QColor(24,24,24));
    m_areaGradient.setColorAt(0.95, QColor(24,24,24));
    m_areaGradient.setColorAt(0.97, QColor(32,32,32));
    m_areaGradient.setColorAt(0.99, QColor(50,50,50));
    m_areaGradient.setColorAt(1.00, QColor(64,64,64));

    auto pal = qApp->palette();
    m_lineSelected = pal.highlight().color().darker();

    QPainterPath p;
    p.setFillRule(Qt::WindingFill);
    p.addRoundedRect(0, 0, m_size.width(), m_size.height(), m_size.width()*0.1, m_size.width()*0.1, Qt::AbsoluteSize);
    m_areaPath = p;
    setPath(m_areaPath);
}

void CProtocolLabelArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //----- Draw area -----//
    QPen mainPen;
    mainPen.setWidth(m_lineSize);
    mainPen.setStyle(Qt::SolidLine);

    if(isSelected())
        mainPen.setColor(m_lineSelected);
    else
        mainPen.setColor(m_lineColor);

    QBrush mainBrush;
    mainBrush.setStyle(Qt::SolidPattern);

    if(isSelected())
        mainBrush.setColor(m_bckColor.lighter());
    else
        mainBrush.setColor(m_bckColor);

    painter->setPen(mainPen);
    painter->setBrush(m_areaGradient);
    painter->setOpacity(opacity());
    painter->drawPath(m_areaPath);

    //----- Draw text -----//
    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->setOpacity(1.0);
    painter->drawPath(m_textPath);
}

void CProtocolLabelArea::update()
{
    initText();
    initArea();
}
