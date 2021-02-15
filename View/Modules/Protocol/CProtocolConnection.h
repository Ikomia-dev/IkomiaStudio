/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CPROTOCOLCONNECTION_H
#define CPROTOCOLCONNECTION_H

#include <QGraphicsPathItem>
#include "Main/forwards.hpp"
#include "Main/AppDefine.hpp"
#include "Core/CProtocol.h"

class CProtocolPortItem;

class CProtocolConnection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

    public:

        enum { Type = UserType + ProtocolGraphicsItem::CONNECTION };

        CProtocolConnection(QGraphicsItem* parent = Q_NULLPTR);
        CProtocolConnection(ProtocolEdge getId, QGraphicsItem* parent = Q_NULLPTR);
        ~CProtocolConnection();

        //Getters
        int                 type() const override;
        ProtocolEdge        getId() const;
        CProtocolPortItem*  getSourcePort() const;
        CProtocolPortItem*  getTargetPort() const;

        //Setters
        void                setSourcePort(CProtocolPortItem* pPort);
        void                setTargetPort(CProtocolPortItem* pPort);

        void                updatePath();
        void                updatePath(QPointF currentPos);

        using QGraphicsPathItem::contains;
        bool                contains(const ProtocolVertex& id1, const ProtocolVertex& id2, bool bDirected);

        void                prepareRemove();

    public slots:

        void                onDelete();

    protected:

        void                hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void                hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        void                init();

        void                showDeleteWidget();
        void                hideDeleteWidget();

        QPen                createPen(const QPointF &pt1, const QPointF &pt2);

    private:

        ProtocolEdge            m_id;
        CProtocolPortItem*      m_pSrcPort = nullptr;
        CProtocolPortItem*      m_pDstPort = nullptr;
        QGraphicsProxyWidget*   m_pDeleteWidget = nullptr;
        float                   m_portRadius = 6.0f;
        int                     m_penSize = 2;
};

#endif // CPROTOCOLCONNECTION_H
