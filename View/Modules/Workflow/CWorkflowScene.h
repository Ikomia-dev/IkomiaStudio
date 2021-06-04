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

#ifndef CWORKFLOWSCENE_H
#define CWORKFLOWSCENE_H

#include "Main/forwards.hpp"
#include <QGraphicsScene>
#include "Core/CWorkflow.h"
#include "../../Process/CProcessPopupDlg.h"
#include "View/Graphics/CGraphicsLayerChoiceDlg.h"

class CWorkflowItem;
class CWorkflowPortItem;
class CWorkflowConnection;
class CWorkflowIOArea;
class CWorkflowDummyItem;

class CWorkflowScene : public QGraphicsScene
{
    Q_OBJECT

    public:

        CWorkflowScene(QWidget* pParent = nullptr);

        //Getters
        QSize               getCellSize() const;
        CWorkflowIOArea*    getInputArea() const;

        //Setters
        void                setSelectedItem(QGraphicsItem* pItem);
        void                setRootId(const WorkflowVertex& id);
        void                setWorkflowStarted(bool bStarted);
        void                setTaskActionFlag(const WorkflowVertex& id, CWorkflowTask::ActionFlag action, bool bEnable);
        void                setCellSize(QSize cellSize);
        void                setItemSize(QSize itemSize);
        void                setInputInfo(const CDataInfoPtr& info, int index);

        //Methods
        void                initInputs();

        void                updateInput(int index, const WorkflowTaskIOPtr &inputPtr, bool isBatch);

        void                addTaskItem(CWorkflowItem* pItem);
        void                addTaskItem(CWorkflowItem *pItem, CWorkflowItem* pParent);

        void                deleteConnection(CWorkflowConnection* pConnection, bool bLater, bool bNotifyView, bool bNotifyModel);
        void                deleteInput(int index);

        QRectF              resize(const QRectF& viewRect);

        void                updateCandidateTask();

        void                clearAll();

        void                queryProjectDataProxyModel(const TreeItemType &inputType, const std::vector<DataDimension>& filters);

        void                retrieveIOInfo(const WorkflowVertex& taskId, size_t index, bool bInput);

    signals:

        void                doSelectionChange(const WorkflowVertex& id);

        void                doItemDeleted(const WorkflowVertex& id);
        void                doItemConnection(const WorkflowVertex& srcId, size_t srcIndex, const WorkflowVertex& dstId, size_t dstIndex);

        void                doDeleteConnection(const WorkflowEdge& id, bool bNotifyModel);
        void                doDeleteInput(int index);

        void                doSetTaskActionFlag(const WorkflowVertex& id, CWorkflowTask::ActionFlag action, bool bEnable);
        void                doSetGraphicsLayerInput(const WorkflowVertex& id, int portIndex, const QModelIndex& layerIndex);
        void                doSetInput(int inputIndex, const CWorkflowInput& input);
        void                doSetFolderPathInput(const WorkflowVertex& id, size_t index, const QString& path);

        void                doQueryGraphicsProxyModel();
        void                doQueryProjectDataProxyModel(const std::vector<TreeItemType>& dataTypes, const std::vector<DataDimension>& filters);

        void                doAddInput(const CWorkflowInput& input);

        void                doShowProcessPopup();

        void                doQueryIOInfo(const WorkflowVertex& taskId, int index, bool bInput);

    public slots:

        void                onSetGraphicsProxyModel(CProjectGraphicsProxyModel* pModel);
        void                onSetProjectDataProxyModel(CProjectDataProxyModel *pModel);
        void                onSelectionChanged();
        void                onIOAreaChanged();

    protected:

        void                mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
        void                keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    private:

        void                setGraphGridItem(QGraphicsItem* pItem, QPair<int,int>& cell);
        void                setPosition(QGraphicsItem *pItem, int row, int column);
        void                setDialogPosition(QDialog* pDialog, const QPointF& pos);

        QRectF              getItemsBoundingRect() const;
        QRectF              getViewRect() const;
        int                 getTaskItemCount() const;
        QPair<int, int>     getCandidateCell(CWorkflowItem* pItem) const;
        QPair<int, int>     getCellFromParent(CWorkflowItem* pItem, CWorkflowItem* pParent, int childIndex) const;
        size_t              getGraphGridIndex(const QPair<int,int>& cell) const;

        bool                isGraphGridRowEmpty(int row, int firstColumn, int lastColumn) const;
        bool                isValidDropCell(const QPair<int,int>& cell) const;

        void                createIOAreas();

        void                addItemToGraphGrid(QGraphicsItem* pItem, QGraphicsItem* pParent);

        void                deleteSelectedItems();
        void                deleteTaskItem(CWorkflowItem* pItem);

        void                removeGraphGridEmptyRows();

        void                finalizeCurrentConnection(QPointF pos);

        void                updateNearestPorts(QPointF pos);
        void                updatePortZoom(QPointF pos);
        void                updateDropCellRubber(QPointF pos);
        void                updateGraphGridSize(const QPair<int,int>& cell);
        void                updateSceneFromGraphGrid();
        void                updateAllConnections();

        void                endItemDrag(QPointF pos);

        QPoint              mapToScreen(QPointF scenePos);

        QPair<int,int>      positionToGrid(const QPointF& pos) const;
        QPointF             gridToPosition(int row, int column) const;

        bool                isWidthReducible(QRectF viewRect);
        bool                isHeightReducible(QRectF viewRect);

        void                checkSceneRectChange();
        void                centerInputArea();

        void                onPortClicked(CWorkflowPortItem *pPort, const QPointF& pos);

        void                openProcessDlg();
        void                openGraphicsLayerChoiceDlg(CWorkflowPortItem *pPort, const QPointF &pos);
        void                openFolderPathChoiceDlg(CWorkflowPortItem *pPort, const QPointF &pos);

    private:

        //Memory structure of the graph for the view
        CndArray<QGraphicsItem*> m_graphGrid;
        //Size of the cells in the grid.
        QSize                   m_cellSize = QSize(120, 120);
        QSize                   m_itemSize = QSize(90, 90);
        QPointF                 m_dragStartPos = QPointF(0, 0);
        bool                    m_bWorkflowStarted = false;
        bool                    m_bItemDragged = false;
        bool                    m_bNewItemClicked = false;
        WorkflowVertex          m_rootId;
        QGraphicsItem*          m_pCurWorkflowItem = nullptr;
        CWorkflowConnection*    m_pWorkflowConnectionTmp = nullptr;
        QGraphicsProxyWidget*   m_pDropCellRubber = nullptr;
        CWorkflowIOArea*        m_pInputArea = nullptr;
        CWorkflowIOArea*        m_pOutputArea = nullptr;
        CWorkflowDummyItem*     m_pDummyItem = nullptr;
        QGraphicsItem*          m_pLastItem = nullptr;
        CGraphicsLayerChoiceDlg m_layerChoiceDlg;
};

#endif // CWORKFLOWSCENE_H
