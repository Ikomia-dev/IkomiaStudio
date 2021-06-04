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

#ifndef CWORKFLOWPORTITEM_H
#define CWORKFLOWPORTITEM_H

#include <QGraphicsPathItem>
#include "Main/AppDefine.hpp"
#include "Core/CWorkflow.h"

class CWorkflowItem;
class CWorkflowConnection;

class CWorkflowPortItem : public QGraphicsPathItem
{
    public:

        enum { Type = UserType + WorkflowGraphicsItem::PORT };

        CWorkflowPortItem(size_t index, bool bInput, WorkflowVertex taskId, IODataType dataType, QGraphicsItem* parent = Q_NULLPTR);
        CWorkflowPortItem(size_t index, bool bInput, WorkflowVertex taskId, IODataType dataType, QColor color, QGraphicsItem* parent = Q_NULLPTR);
        virtual ~CWorkflowPortItem();

        //Getters
        int                             type() const override;
        size_t                          getIndex() const;
        float                           getRadius() const;
        WorkflowVertex                  getTaskId() const;
        int                             getConnectionCount() const;
        QVector<CWorkflowConnection*>   getConnections() const;
        QColor                          getColor() const;
        IODataType                      getDataType() const;

        bool                            isInput() const;
        bool                            isConnectedTo(const WorkflowVertex& id) const;
        bool                            isClickable() const;

        //Setters
        void                            setIndex(size_t index);
        void                            setColor(QColor color);
        void                            setAssigned(bool bAssign);
        void                            setRadius(float radius);
        void                            setInfo(const CDataInfoPtr& info);
        void                            setDataType(IODataType type);

        bool                            connectionExists(CWorkflowPortItem* pPort) const;

        void                            addInputConnection(CWorkflowConnection* pConnection);
        void                            addOutputConnection(CWorkflowConnection* pConnection);

        void                            updateConnections();
        void                            updateShape(QPointF pos);

        void                            removeConnection(CWorkflowConnection* pConnection);

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
        WorkflowVertex                  m_taskId;
        QVector<CWorkflowConnection*>   m_connections;
        IODataType                      m_dataType = IODataType::NONE;
        float                           m_baseRadius = 4.0;
        float                           m_radius = 4.0;
        QColor                          m_color = Qt::white;
        QColor                          m_borderColor = m_color.darker();
        QColor                          m_highlightColor;
        QPainterPath                    m_mainPath;
        QPainterPath                    m_assignedPath;
};

#endif // CWORKFLOWPORTITEM_H
