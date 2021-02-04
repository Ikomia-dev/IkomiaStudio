#ifndef CGRAPHICSTOOLS_HPP
#define CGRAPHICSTOOLS_HPP

class CGraphicsInactiveSimpleTextItem : public QGraphicsSimpleTextItem
{
    public:

        CGraphicsInactiveSimpleTextItem(QGraphicsItem* parent = Q_NULLPTR) : QGraphicsSimpleTextItem(parent)
        {
        }

        bool contains(const QPointF &point) const override
        {
            Q_UNUSED(point);
            return false;
        }
};

class CGraphicsInactiveTextItem : public QGraphicsTextItem
{
    public:

        CGraphicsInactiveTextItem(QGraphicsItem* parent = Q_NULLPTR) : QGraphicsTextItem(parent)
        {
        }

        bool contains(const QPointF &point) const override
        {
            Q_UNUSED(point);
            return false;
        }
};

#endif // CGRAPHICSTOOLS_HPP
