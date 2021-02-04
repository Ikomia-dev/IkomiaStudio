#ifndef CPROTOCOLPORTITEM_H
#define CPROTOCOLPORTITEM_H

#include <QGraphicsPathItem>
#include "Main/AppDefine.hpp"
#include "Core/CProtocol.h"

class CProtocolItem;
class CProtocolConnection;

class CProtocolPortItem : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + ProtocolGraphicsItem::PORT };

        CProtocolPortItem(size_t index, bool bInput, ProtocolVertex taskId, IODataType dataType, QGraphicsItem* parent = Q_NULLPTR);
        CProtocolPortItem(size_t index, bool bInput, ProtocolVertex taskId, IODataType dataType, QColor color, QGraphicsItem* parent = Q_NULLPTR);
        virtual ~CProtocolPortItem();

        //Getters
        int                             type() const override;
        size_t                          getIndex() const;
        float                           getRadius() const;
        ProtocolVertex                  getTaskId() const;
        int                             getConnectionCount() const;
        QVector<CProtocolConnection*>   getConnections() const;
        QColor                          getColor() const;
        IODataType                      getDataType() const;

        bool                            isInput() const;
        bool                            isConnectedTo(const ProtocolVertex& id) const;
        bool                            isClickable() const;

        //Setters
        void                            setIndex(size_t index);
        void                            setColor(QColor color);
        void                            setAssigned(bool bAssign);
        void                            setRadius(float radius);
        void                            setInfo(const CDataInfoPtr& info);
        void                            setDataType(IODataType type);

        bool                            connectionExists(CProtocolPortItem* pPort) const;

        void                            addInputConnection(CProtocolConnection* pConnection);
        void                            addOutputConnection(CProtocolConnection* pConnection);

        void                            updateConnections();
        void                            updateShape(QPointF pos);

        void                            removeConnection(CProtocolConnection* pConnection);

        void                            paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    protected:

        void                            hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void                            hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        void                            hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        void                            initPainter();

        void                            clearConnections();

        void                            retrieveIOInfo();

    private:

        size_t                          m_index = 0;
        bool                            m_bInput = true;
        bool                            m_bAssigned = false;
        ProtocolVertex                  m_taskId;
        QVector<CProtocolConnection*>   m_connections;
        IODataType                      m_dataType = IODataType::NONE;
        float                           m_baseRadius = 4.0;
        float                           m_radius = 4.0;
        QColor                          m_color = Qt::white;
        QColor                          m_borderColor = m_color.darker();
        QColor                          m_highlightColor;
        QPainterPath                    m_mainPath;
        QPainterPath                    m_assignedPath;
};

#endif // CPROTOCOLPORTITEM_H
