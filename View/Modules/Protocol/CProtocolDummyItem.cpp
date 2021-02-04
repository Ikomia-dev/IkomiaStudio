#include "CProtocolDummyItem.h"

CProtocolDummyItem::CProtocolDummyItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{
    init();
    setAcceptHoverEvents(true);
}

int CProtocolDummyItem::type() const
{
    return Type;
}

void CProtocolDummyItem::setColor(QColor color)
{
    m_color = color;
}

void CProtocolDummyItem::setTextMargin(int margin)
{
    m_textMargin = margin;
    init();
}

void CProtocolDummyItem::setLineSize(int size)
{
    m_lineSize = size;
}

void CProtocolDummyItem::setSize(QSize itemSize)
{
    m_size = itemSize;
}

void CProtocolDummyItem::init()
{
    //Adjust text
    QFont font = m_textItem.font();
    font.setPointSize(8);
    QString text = QObject::tr("Click here to add new process");
    m_textItem.setTextWidth(m_size.width() - 2*m_textMargin);
    m_textItem.setFont(font);
    m_textItem.setPlainText(text);
    m_textItem.setParentItem(this);
    QRectF rcText = m_textItem.boundingRect();
    m_textItem.setPos(m_size.width()/2 - rcText.width()/2, m_size.height()/2 - rcText.height()/2);

    //Rounded rectangle
    QPainterPath p;
    p.addRoundedRect(0, 0, m_size.width(), m_size.height(), m_size.width()*0.1, m_size.width()*0.1, Qt::AbsoluteSize);
    m_painterPath = p;
    setPath(m_painterPath);
}

void CProtocolDummyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QLinearGradient gradientBody(QPointF( m_size.width()/2, 0), QPointF( m_size.width()/2, m_size.height()));

    gradientBody.setColorAt(0.0, QColor(70,70,70,204));
    gradientBody.setColorAt(0.03, QColor(50,50,50,204));
    gradientBody.setColorAt(0.20, QColor(24,24,24,204));
    gradientBody.setColorAt(0.80, QColor(24,24,24,204));
    gradientBody.setColorAt(0.97, QColor(50,50,50,204));
    gradientBody.setColorAt(1.0, QColor(70,70,70,204));

    QPen pen;
    pen.setWidth(m_lineSize);
    pen.setColor(m_color);
    painter->setPen(pen);
    painter->setBrush(gradientBody);
    painter->drawPath(m_painterPath);

    m_textItem.setDefaultTextColor(m_color);
}

void CProtocolDummyItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverEnterEvent(event);
}
