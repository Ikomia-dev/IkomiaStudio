#ifndef CPROTOCOLDUMMYITEM_H
#define CPROTOCOLDUMMYITEM_H

#include <QGraphicsPathItem>
#include "Main/forwards.hpp"
#include "Main/AppDefine.hpp"
#include "View/Common/CGraphicsItemTools.hpp"

class CProtocolDummyItem : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + ProtocolGraphicsItem::DUMMY };

        CProtocolDummyItem(QGraphicsItem* parent = Q_NULLPTR);

        int     type() const override;

        void    setColor(QColor color);
        void    setTextMargin(int margin);
        void    setLineSize(int size);
        void    setSize(QSize itemSize);

        void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:

        void    hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    private:

        void    init();

    private:

        int                         m_textMargin = 5;
        int                         m_lineSize = 2;
        QSize                       m_size = QSize(80, 80);
        QColor                      m_color = Qt::white;
        CGraphicsInactiveTextItem   m_textItem;
        QPainterPath                m_painterPath;
};

#endif // CPROTOCOLDUMMYITEM_H
