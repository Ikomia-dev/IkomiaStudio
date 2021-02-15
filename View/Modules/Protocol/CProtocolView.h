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

#ifndef CPROTOCOLVIEW_H
#define CPROTOCOLVIEW_H

#include <QGraphicsView>
#include "Core/CProtocol.h"
#include "Model/Protocol/CProtocolManager.h"
#include "Model/User/CUser.h"

class CProtocolScene;
class CProtocolItem;
class CProtocolConnection;

class CProtocolView : public QGraphicsView
{
    Q_OBJECT

    public:

        CProtocolView(QWidget* parent = Q_NULLPTR);

        void                    setModel(CProtocolManager* pModel);

        CProtocolScene*         getScene();

        void                    manageProtocolCreated();

        void                    addTask(const ProtocolTaskPtr &pTask, const ProtocolVertex& id, const ProtocolVertex& parentId);
        void                    addTask(const ProtocolTaskPtr &pTask, const ProtocolVertex& id);

        void                    clear();

        void                    zoomInit();
        void                    zoomIn();
        void                    zoomOut();

    public slots:

        void                    updateSceneRect(const QRectF &rect); // override QGraphicsView::updateSceneRect()

        void                    onInputChanged(int index);
        void                    onInputAssigned(const ProtocolVertex& id, int portIndex, bool bAssigned);
        void                    onInputRemoved(int index);
        void                    onInputsCleared();

        void                    onAddConnection(const ProtocolEdge& id, const ProtocolVertex& srcId, size_t srcIndex, const ProtocolVertex& dstId, size_t dstIndex);

        void                    onDeleteItem(const ProtocolVertex& id);
        void                    onDeleteConnectionFromScene(const ProtocolEdge& id, bool bNotifyModel);
        void                    onDeleteConnectionFromModel(const ProtocolEdge& id);

        void                    onSetActiveTask(const ProtocolVertex& id);
        void                    onSetTaskState(const ProtocolVertex& id, CProtocolTask::State status, const QString& msg = QString());
        void                    onSetIOInfo(const CDataInfoPtr& info, const ProtocolVertex& taskId, int index, bool bInput);

        void                    onUpdateCandidateTask();
        void                    onUpdateTaskItem(const ProtocolTaskPtr &pTask, const ProtocolVertex& id);

    protected:

        virtual void            wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
        void                    resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

        void                    mousePressEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                    mouseMoveEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                    mouseReleaseEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;

        void                    drawBackground(QPainter* painter, const QRectF& r) override;

    private:

        void                    initConnections();
        void                    initProtocolScene();

        CProtocolItem*          createItem(std::shared_ptr<CProtocolTask> pTask, const ProtocolVertex& id);

        CProtocolConnection*    connectItems(const ProtocolEdge& id, CProtocolItem* pSrcItem, size_t srcIndex, CProtocolItem *pDstItem, size_t dstIndex);

    private:

        CProtocolManager*                               m_pModel = nullptr;
        CProtocolScene*                                 m_pScene = nullptr;
        std::map<ProtocolVertex, CProtocolItem*>        m_idToItem;
        std::map<ProtocolEdge, CProtocolConnection*>    m_idToConnection;
        QPointF                                         m_dragStartPos = QPointF(0, 0);
        const double                                    m_scaleFactor = 1.15;
        // Item and grid display parameters
        QSize                                           m_cellSize = QSize(150,120);
        QSize                                           m_itemSize = QSize(90,90);
        float                                           m_portRadius = 4;
};

#endif // CPROTOCOLVIEW_H
