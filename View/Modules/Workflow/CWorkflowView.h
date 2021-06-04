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

#ifndef CWORKFLOWVIEW_H
#define CWORKFLOWVIEW_H

#include <QGraphicsView>
#include "Core/CWorkflow.h"
#include "Model/Workflow/CWorkflowManager.h"
#include "Model/User/CUser.h"

class CWorkflowScene;
class CWorkflowItem;
class CWorkflowConnection;

class CWorkflowView : public QGraphicsView
{
    Q_OBJECT

    public:

        CWorkflowView(QWidget* parent = Q_NULLPTR);

        void                    setModel(CWorkflowManager* pModel);

        CWorkflowScene*         getScene();

        void                    manageWorkflowCreated();

        void                    addTask(const WorkflowTaskPtr &pTask, const WorkflowVertex& id, const WorkflowVertex& parentId);
        void                    addTask(const WorkflowTaskPtr &pTask, const WorkflowVertex& id);

        void                    clear();

        void                    zoomInit();
        void                    zoomIn();
        void                    zoomOut();

    public slots:

        void                    updateSceneRect(const QRectF &rect); // override QGraphicsView::updateSceneRect()

        void                    onInputChanged(int index);
        void                    onInputAssigned(const WorkflowVertex& id, int portIndex, bool bAssigned);
        void                    onInputRemoved(int index);
        void                    onInputsCleared();

        void                    onAddConnection(const WorkflowEdge& id, const WorkflowVertex& srcId, size_t srcIndex, const WorkflowVertex& dstId, size_t dstIndex);

        void                    onDeleteItem(const WorkflowVertex& id);
        void                    onDeleteConnectionFromScene(const WorkflowEdge& id, bool bNotifyModel);
        void                    onDeleteConnectionFromModel(const WorkflowEdge& id);

        void                    onSetActiveTask(const WorkflowVertex& id);
        void                    onSetTaskState(const WorkflowVertex& id, CWorkflowTask::State status, const QString& msg = QString());
        void                    onSetIOInfo(const CDataInfoPtr& info, const WorkflowVertex& taskId, int index, bool bInput);

        void                    onUpdateCandidateTask();
        void                    onUpdateTaskItem(const WorkflowTaskPtr &pTask, const WorkflowVertex& id);

    protected:

        virtual void            wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
        void                    resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

        void                    mousePressEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                    mouseMoveEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                    mouseReleaseEvent(QMouseEvent *mouseEvent) Q_DECL_OVERRIDE;

        void                    drawBackground(QPainter* painter, const QRectF& r) override;

    private:

        void                    initConnections();
        void                    initWorkflowScene();

        CWorkflowItem*          createItem(std::shared_ptr<CWorkflowTask> pTask, const WorkflowVertex& id);

        CWorkflowConnection*    connectItems(const WorkflowEdge& id, CWorkflowItem* pSrcItem, size_t srcIndex, CWorkflowItem *pDstItem, size_t dstIndex);

    private:

        CWorkflowManager*                               m_pModel = nullptr;
        CWorkflowScene*                                 m_pScene = nullptr;
        std::map<WorkflowVertex, CWorkflowItem*>        m_idToItem;
        std::map<WorkflowEdge, CWorkflowConnection*>    m_idToConnection;
        QPointF                                         m_dragStartPos = QPointF(0, 0);
        const double                                    m_scaleFactor = 1.15;
        // Item and grid display parameters
        QSize                                           m_cellSize = QSize(150,120);
        QSize                                           m_itemSize = QSize(90,90);
        float                                           m_portRadius = 4;
};

#endif // CWORKFLOWVIEW_H
