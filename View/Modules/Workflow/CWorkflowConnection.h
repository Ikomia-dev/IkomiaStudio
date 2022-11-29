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

#ifndef CWORKFLOWCONNECTION_H
#define CWORKFLOWCONNECTION_H

#include <QGraphicsPathItem>
#include "Main/forwards.hpp"
#include "Main/AppDefine.hpp"
#include "Core/CWorkflow.h"

class CWorkflowPortItem;

class CWorkflowConnection : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

    public:

        enum { Type = UserType + WorkflowGraphicsItem::CONNECTION };

        CWorkflowConnection(QGraphicsItem* parent = Q_NULLPTR);
        CWorkflowConnection(WorkflowEdge getId, QGraphicsItem* parent = Q_NULLPTR);
        ~CWorkflowConnection();

        //Getters
        int                 type() const override;
        WorkflowEdge        getId() const;
        CWorkflowPortItem*  getSourcePort() const;
        CWorkflowPortItem*  getTargetPort() const;

        bool                isConnectionItem(QGraphicsItem* pItem) const;

        //Setters
        void                setSourcePort(CWorkflowPortItem* pPort);
        void                setTargetPort(CWorkflowPortItem* pPort);

        void                updatePath();
        void                updatePath(QPointF currentPos);

        using QGraphicsPathItem::contains;
        bool                contains(const WorkflowVertex& id1, const WorkflowVertex& id2, bool bDirected);

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

        WorkflowEdge            m_id;
        CWorkflowPortItem*      m_pSrcPort = nullptr;
        CWorkflowPortItem*      m_pDstPort = nullptr;
        QGraphicsProxyWidget*   m_pDeleteWidget = nullptr;
        float                   m_portRadius = 6.0f;
        int                     m_penSize = 2;
};

#endif // CWORKFLOWCONNECTION_H
