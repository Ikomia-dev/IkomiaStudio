// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QRubberBand>
#include "Main/AppTools.hpp"
#include "CWorkflowScene.h"
#include "CWorkflowItem.h"
#include "CWorkflowPortItem.h"
#include "CWorkflowConnection.h"
#include "CWorkflowIOArea.h"
#include "CWorkflowDummyItem.h"

CWorkflowScene::CWorkflowScene(QWidget* pParent)
{
    // Do not call QGraphicsScene(pParent) to avoid automatic deletion of child items
    // -> cause double delete of CWorkflowConnection instead...
    Q_UNUSED(pParent);
    Dimensions dims = {{DataDimension::Y, 1}, {DataDimension::X, 1}};
    m_graphGrid = CndArray<QGraphicsItem*>(dims);
    m_rootId = boost::graph_traits<WorkflowGraph>::null_vertex();
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
    createIOAreas();

    connect(this, &CWorkflowScene::selectionChanged, this, &CWorkflowScene::onSelectionChanged);
}

QSize CWorkflowScene::getCellSize() const
{
    return m_cellSize;
}

CWorkflowIOArea *CWorkflowScene::getInputArea() const
{
    return m_pInputArea;
}

void CWorkflowScene::setSelectedItem(QGraphicsItem *pItem)
{
    if(m_pCurWorkflowItem != nullptr)
        m_pCurWorkflowItem->setSelected(false);

    pItem->setSelected(true);
    m_pCurWorkflowItem = pItem;
}

void CWorkflowScene::setRootId(const WorkflowVertex &id)
{
    m_rootId = id;
    m_pInputArea->setTaskId(id);
}

void CWorkflowScene::setWorkflowStarted(bool bStarted)
{
    m_bWorkflowStarted = bStarted;
    m_pInputArea->activateActions(bStarted);
}

void CWorkflowScene::setTaskActionFlag(const WorkflowVertex &id, CWorkflowTask::ActionFlag action, bool bEnable)
{
    emit doSetTaskActionFlag(id, action, bEnable);
}

void CWorkflowScene::setCellSize(QSize cellSize)
{
    m_cellSize = cellSize;
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
}

void CWorkflowScene::setItemSize(QSize itemSize)
{
    m_itemSize = itemSize;
}

void CWorkflowScene::setInputInfo(const CDataInfoPtr &info, int index)
{
    if(m_pInputArea)
        m_pInputArea->setPortInfo(info, index);
}

void CWorkflowScene::initInputs()
{
    for(int i=0; i<m_pInputArea->getPortCount(); ++i)
        m_pInputArea->updatePort(i, nullptr, false);
}

void CWorkflowScene::updateInput(int index, const WorkflowTaskIOPtr& inputPtr, bool isBatch)
{
    if(index < m_pInputArea->getPortCount())
        m_pInputArea->updatePort(index, inputPtr, isBatch);
    else
        m_pInputArea->addPort(inputPtr, isBatch);
}

//Add item at candidate position
void CWorkflowScene::addTaskItem(CWorkflowItem *pItem)
{
    //Positionnement
    auto cell = getCandidateCell(pItem);
    setPosition(pItem, cell.first, cell.second);
    //Mise à jour de la structure du graphe
    updateGraphGridSize(cell);
    setGraphGridItem(pItem, cell);
    //Insertion dans la scene
    addItem(pItem);
}

//Add item according to parent position
void CWorkflowScene::addTaskItem(CWorkflowItem *pItem, CWorkflowItem *pParent)
{
    //Clear old position of dummy item
    auto dummyCell = positionToGrid(m_pDummyItem->pos());
    auto index = getGraphGridIndex(dummyCell);

    if(index != SIZE_MAX &&  m_graphGrid[index] && m_graphGrid[index]->type() == CWorkflowDummyItem::Type)
        m_graphGrid[index] = nullptr;

    //Insertion dans la scene
    addItem(pItem);
    //Mise à jour de la structure du graphe
    addItemToGraphGrid(pItem, pParent);
    updateSceneFromGraphGrid();
}

QPair<int,int> CWorkflowScene::getCandidateCell(CWorkflowItem *pItem) const
{
    assert(pItem);
    assert(m_pDummyItem);
    return positionToGrid(m_pDummyItem->pos());
}

QPair<int,int> CWorkflowScene::getCellFromParent(CWorkflowItem *pItem, CWorkflowItem *pParent, int childIndex) const
{
    assert(pItem);
    QPair<int,int> parentCell(0, 0);

    if(pParent != nullptr)
        parentCell = positionToGrid(pParent->pos());

    return QPair<int,int>(parentCell.first + childIndex, parentCell.second + 1);
}

size_t CWorkflowScene::getGraphGridIndex(const QPair<int, int> &cell) const
{
    DimensionIndices indices = {{DataDimension::Y, cell.first}, {DataDimension::X, cell.second}};
    return m_graphGrid.index(indices);
}

bool CWorkflowScene::isGraphGridRowEmpty(int row, int firstColumn, int lastColumn) const
{
    int nbRows = (int)m_graphGrid.size(DataDimension::Y);
    if(row >= nbRows)
        return true;

    int nbColumns = (int)m_graphGrid.size(DataDimension::X);
    if(firstColumn >= nbColumns)
        return true;

    for(int i=firstColumn; i<=lastColumn; ++i)
    {
        auto index = getGraphGridIndex(QPair<int, int>(row, i));
        assert(index != SIZE_MAX);

        if(m_graphGrid[index] !=  nullptr)
            return false;
    }
    return true;
}

bool CWorkflowScene::isValidDropCell(const QPair<int,int>& cell) const
{
    size_t index = getGraphGridIndex(cell);
    //int outputCol = m_pOutputArea->pos().x() / m_cellSize.width();

    auto dims = m_graphGrid.dimensions();
    int rowCount = (int)(dims[0].second);
    int colCount = (int)(dims[1].second);

    return ((cell.second > 0 /*&& cell.second < outputCol*/) && //Not first or last column
            ((cell.first >= rowCount || cell.second >= colCount) || //Outside the grid
            (index < m_graphGrid.size() && (m_graphGrid[index] == nullptr || m_graphGrid[index] == m_pDummyItem)))); //Empty cell inside grid
}

void CWorkflowScene::deleteInput(int index)
{
    assert(m_pInputArea);
    m_pInputArea->removePort(index);
}

void CWorkflowScene::deleteConnection(CWorkflowConnection *pConnection, bool bLater, bool bNotifyView, bool bNotifyModel)
{
    assert(pConnection);

    if(bNotifyView)
        emit doDeleteConnection(pConnection->getId(), bNotifyModel);

    if(bLater)
        pConnection->deleteLater();
    else
        delete pConnection;
}

QRectF CWorkflowScene::resize(const QRectF &viewRect)
{
    QRectF rect = sceneRect();
    QRectF newRect = rect;
    QRectF boundRect = getItemsBoundingRect();

    if(viewRect.width() > rect.width() || isWidthReducible(viewRect))
        newRect.setWidth(viewRect.width());

    if(viewRect.height() > rect.height() || isHeightReducible(viewRect))
        newRect.setHeight(viewRect.height());
    else if(boundRect.height() > viewRect.height())
        newRect.setHeight(boundRect.height());

    if(newRect != rect)
        setSceneRect(newRect);

    centerInputArea();

    return newRect;
}

void CWorkflowScene::updateCandidateTask()
{
    if(m_pDummyItem == nullptr)
    {
        m_pDummyItem = new CWorkflowDummyItem;
        m_pDummyItem->setSize(m_itemSize);
        m_pDummyItem->setColor(qApp->palette().text().color());
        addItem(m_pDummyItem);
    }

    //Clear old position of dummy item
    auto dummyCell = positionToGrid(m_pDummyItem->pos());
    auto index = getGraphGridIndex(dummyCell);

    if(index != SIZE_MAX &&  m_graphGrid[index] && m_graphGrid[index]->type() == CWorkflowDummyItem::Type)
        m_graphGrid[index] = nullptr;

    addItemToGraphGrid(m_pDummyItem, m_pCurWorkflowItem);
    removeGraphGridEmptyRows();
    updateSceneFromGraphGrid();
}

void CWorkflowScene::clearAll()
{
    m_graphGrid.clear();
    Dimensions dims = {{DataDimension::Y, 1}, {DataDimension::X, 1}};
    m_graphGrid.setDimensions(dims);

    m_rootId = boost::graph_traits<WorkflowGraph>::null_vertex();
    m_pCurWorkflowItem = nullptr;
    m_pWorkflowConnectionTmp = nullptr;
    m_pDropCellRubber = nullptr;
    m_pInputArea = nullptr;
    m_pOutputArea = nullptr;
    m_pDummyItem = nullptr;
    clear();
    createIOAreas();
}

void CWorkflowScene::queryProjectDataProxyModel(const TreeItemType& inputType, const std::vector<DataDimension> &filters)
{
    std::vector<TreeItemType> inputTypes;
    inputTypes.push_back(inputType);
    emit doQueryProjectDataProxyModel(inputTypes, filters);
}

void CWorkflowScene::retrieveIOInfo(const WorkflowVertex &taskId, size_t index, bool bInput)
{
    emit doQueryIOInfo(taskId, static_cast<int>(index), bInput);
}

void CWorkflowScene::onSetGraphicsProxyModel(CProjectGraphicsProxyModel *pModel)
{
    m_layerChoiceDlg.setModel(pModel);
}

void CWorkflowScene::onSetProjectDataProxyModel(CProjectDataProxyModel *pModel)
{
    if(m_pInputArea)
        m_pInputArea->setProjectDataProxyModel(pModel);
}

void CWorkflowScene::onSelectionChanged()
{
    auto items = selectedItems();

    if(items.size() == 1)
    {
        if(items[0] != m_pLastItem)
            m_pLastItem = items[0];
    }
    else if(items.size() > 1)
    {
        blockSignals(true);
        m_pLastItem->setSelected(false);
        items.removeOne(m_pLastItem);
        m_pLastItem = items.back();
        blockSignals(false);
    }

}

void CWorkflowScene::onIOAreaChanged()
{
    // This slot is used when we add input data to the input area
    m_pInputArea->update();
    checkSceneRectChange();
    centerInputArea();
}

void CWorkflowScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() & Qt::LeftButton)
    {
        auto pItem = itemAt(mouseEvent->scenePos(), QTransform());
        m_bNewItemClicked = pItem != m_pCurWorkflowItem;
        m_pCurWorkflowItem = pItem;

        if(m_pCurWorkflowItem)
        {
            switch(m_pCurWorkflowItem->type())
            {
                case CWorkflowItem::Type:
                    m_dragStartPos = mouseEvent->scenePos();
                    break;

                case CWorkflowPortItem::Type:
                    m_dragStartPos = mouseEvent->scenePos();
                    updateNearestPorts(mouseEvent->scenePos());
                    return;

                case CWorkflowIOArea::Type:
                    if(m_pCurWorkflowItem == m_pInputArea)
                        emit doSelectionChange(boost::graph_traits<WorkflowGraph>::null_vertex());
                    break;

                case CWorkflowDummyItem::Type:
                    return;
            }
        }
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void CWorkflowScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->buttons() & Qt::LeftButton)
    {        
        if(m_pCurWorkflowItem)
        {
            qreal moveDistance = (mouseEvent->scenePos() - m_dragStartPos).manhattanLength();
            bool bMinDistance = moveDistance >= QApplication::startDragDistance();

            if(m_pCurWorkflowItem->type() == CWorkflowItem::Type && bMinDistance)
            {
                m_bItemDragged = true;
                updateDropCellRubber(mouseEvent->scenePos());
                static_cast<CWorkflowItem*>(m_pCurWorkflowItem)->updateConnections();
            }
            else if(m_pCurWorkflowItem->type() == CWorkflowPortItem::Type)
            {
                if(m_pWorkflowConnectionTmp == nullptr)
                {
                    CWorkflowPortItem* pPort = static_cast<CWorkflowPortItem*>(m_pCurWorkflowItem);
                    CWorkflowPortItem* pPortTmp = pPort;
                    if(pPort->isInput())
                    {
                        QVector<CWorkflowConnection*> connections = pPort->getConnections();
                        if(connections.size() > 0)
                        {
                            CWorkflowConnection* pConnect = connections[0];
                            pPortTmp = pConnect->getSourcePort();
                            pConnect->onDelete();
                        }
                        else
                            return;
                    }
                    m_pWorkflowConnectionTmp = new CWorkflowConnection;
                    m_pWorkflowConnectionTmp->setSourcePort(pPortTmp);
                    addItem(m_pWorkflowConnectionTmp);
                }
                m_pWorkflowConnectionTmp->updatePath(mouseEvent->scenePos());
                updateNearestPorts(mouseEvent->scenePos());
                return;
            }
        }
    }
    updatePortZoom(mouseEvent->scenePos());
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void CWorkflowScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() & Qt::LeftButton)
    {
        if(m_pCurWorkflowItem != nullptr)
        {
            if(m_pCurWorkflowItem->type() == CWorkflowItem::Type)
            {
                if(m_bItemDragged == true)
                    endItemDrag(mouseEvent->scenePos());
                else if(m_bNewItemClicked == true)
                    emit doSelectionChange(static_cast<CWorkflowItem*>(m_pCurWorkflowItem)->getId());
            }
            else if(m_pCurWorkflowItem->type() == CWorkflowPortItem::Type)
            {
                if(m_pWorkflowConnectionTmp)
                    finalizeCurrentConnection(mouseEvent->scenePos());
                else
                {
                    auto pPortItem = static_cast<CWorkflowPortItem*>(m_pCurWorkflowItem);
                    if(pPortItem->isClickable())
                        onPortClicked(pPortItem, mouseEvent->screenPos());
                }
                updateNearestPorts(QPointF());
            }
            else if(m_pCurWorkflowItem->type() == CWorkflowDummyItem::Type)
                openProcessDlg();
        }
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void CWorkflowScene::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        if(m_bItemDragged == false)
            deleteSelectedItems();
    }
    QGraphicsScene::keyPressEvent(event);
}

void CWorkflowScene::setGraphGridItem(QGraphicsItem *pItem, QPair<int, int> &cell)
{
    auto index = getGraphGridIndex(cell);
    if(index != SIZE_MAX)
        m_graphGrid[index] = pItem;
}

void CWorkflowScene::setPosition(QGraphicsItem *pItem, int row, int column)
{
    assert(pItem);
    QRectF itemRect = pItem->boundingRect();
    auto cellCenter = gridToPosition(row, column);
    pItem->setPos(cellCenter.x() - itemRect.width()/2, cellCenter.y() - itemRect.height()/2);

    if(pItem->type() == CWorkflowItem::Type)
    {
        auto pWorkflowItem = static_cast<CWorkflowItem*>(pItem);
        pWorkflowItem->updateConnections();
    }
    checkSceneRectChange();
}

void CWorkflowScene::setDialogPosition(QDialog *pDialog, const QPointF &pos)
{
    assert(pDialog);

    // pos is current screen position
    // get Screen size
    auto newPos = pos.toPoint();
    auto pScreen = QGuiApplication::screenAt(newPos);
    assert(pScreen);
    auto screenRect = pScreen->availableGeometry();

    int w = pDialog->width();
    int h = pDialog->height();
    newPos.setY(newPos.y() - h);

    if (newPos.x() < 0)
        newPos.setX(0);
    else if(newPos.x() + w > screenRect.right())
        newPos.setX(screenRect.right() - w);

    if (newPos.y() < 0)
        newPos.setY(0);
    else if(newPos.y() + h > screenRect.bottom())
        newPos.setY(screenRect.bottom() - h);

    pDialog->move(newPos);
}

QRectF CWorkflowScene::getItemsBoundingRect() const
{
    QRectF boundRect;
    auto itemList = items();

    for(int i=0; i<itemList.size(); ++i)
    {
        if(itemList[i]->type() == CWorkflowItem::Type || itemList[i]->type() == CWorkflowDummyItem::Type)
        {
            QRectF itemBoundRect = itemList[i]->boundingRect();
            QRectF itemRect = QRectF(itemList[i]->x(), itemList[i]->y(), itemBoundRect.width(), itemBoundRect.height());
            boundRect = boundRect.united(itemRect);
        }
    }
    return boundRect;
}

QRectF CWorkflowScene::getViewRect() const
{
    assert(views().size() > 0);
    QGraphicsView* pView = views().first();
    return pView->viewport()->rect();
}

int CWorkflowScene::getTaskItemCount() const
{
    int inputNb = m_pInputArea->getPortCount();
    int outputNb = m_pOutputArea->getPortCount();
    return items().size() - inputNb - outputNb - 2;
}

void CWorkflowScene::createIOAreas()
{
    QPalette p = qApp->palette();

    m_pInputArea = new CWorkflowIOArea(tr("Inputs"), true);
    m_pInputArea->setColorBg(p.dark().color());
    m_pInputArea->setLineColor(p.dark().color());
    addItem(m_pInputArea);

    centerInputArea();

    connect(m_pInputArea, &CWorkflowIOArea::doIOAreaChanged, this, &CWorkflowScene::onIOAreaChanged);

    /*QRectF rc = sceneRect();
    m_pOutputArea = new CWorkflowIOArea(tr("Outputs"), false);
    m_pOutputArea->setPos(rc.right() - m_pOutputArea->getSize().width(), 0);
    m_pOutputArea->setColorBg(p.dark().color());
    m_pOutputArea->setLineColor(p.dark().color());
    addItem(m_pOutputArea);*/
}

void CWorkflowScene::addItemToGraphGrid(QGraphicsItem *pItem, QGraphicsItem *pParent)
{
    bool bFindPosition = false;
    QPair<int,int> cell = QPair<int, int>(0, 0);

    if(pParent)
    {
        auto parentIndex = m_graphGrid.find(pParent);
        if(parentIndex != SIZE_MAX)
        {
            cell.first = static_cast<int>(parentIndex / m_graphGrid.size(DataDimension::X));
            cell.second = static_cast<int>(parentIndex % m_graphGrid.size(DataDimension::X));
        }
    }

    while(bFindPosition == false)
    {
        if(isGraphGridRowEmpty(cell.first, cell.second+1, (int)m_graphGrid.size(DataDimension::X)-1) == true)
        {
            cell.second++;
            bFindPosition = true;
        }
        else
        {
            cell.first++;
            if(isGraphGridRowEmpty(cell.first, 1, cell.second) == false)
            {
                cell.second++;
                m_graphGrid.insertDimension(DataDimension::Y, cell.first);
                bFindPosition = true;
            }
        }
    }
    updateGraphGridSize(cell);
    setGraphGridItem(pItem, cell);
}

void CWorkflowScene::deleteSelectedItems()
{
    auto items = selectedItems();
    for(int i=0; i<items.size(); ++i)
    {
        if(items[i]->type() == CWorkflowItem::Type)
        {
            auto pItem = static_cast<CWorkflowItem*>(items[i]);
            deleteTaskItem(pItem);
        }
    }
    checkSceneRectChange();
}

void CWorkflowScene::deleteTaskItem(CWorkflowItem *pItem)
{
    assert(pItem);

    //Update graph grid
    size_t index = m_graphGrid.find(pItem);
    if(index == SIZE_MAX)
        return;

    m_graphGrid[index] = nullptr;

    auto connections = pItem->getConnections();
    for(int i=0; i<connections.size(); ++i)
        deleteConnection(connections[i], false, true, false);

    //Remove task and notify CWorkflowView
    if(pItem == m_pCurWorkflowItem)
        m_pCurWorkflowItem = nullptr;

    auto taskId = pItem->getId();
    delete pItem;
    emit doItemDeleted(taskId);
}

void CWorkflowScene::removeGraphGridEmptyRows()
{
    auto dims = m_graphGrid.dimensions();
    auto rowCount = dims[0].second;

    for(int i=(int)rowCount-1; i>=0; --i)
    {
        if(isGraphGridRowEmpty(i, 0, (int)m_graphGrid.size(DataDimension::X)-1))
        {
            DimensionIndices row = {{DataDimension::Y, i}, {DataDimension::X, 0}};
            m_graphGrid.removeDimension(DataDimension::Y, row);
        }
    }
}

void CWorkflowScene::finalizeCurrentConnection(QPointF pos)
{
    auto itemList = items(pos);
    if(itemList.size() > 0)
    {
        auto pSrcPort = m_pWorkflowConnectionTmp->getSourcePort();
        CWorkflowPortItem* pDstPort = nullptr;

        for(int i=0; i<itemList.size(); ++i)
        {
            if(itemList[i]->type() == CWorkflowPortItem::Type)
            {
                pDstPort = static_cast<CWorkflowPortItem*>(itemList[i]);
                break;
            }
        }

        //Check connection validity
        if( pDstPort != nullptr &&
            pSrcPort != pDstPort &&
            pSrcPort->isInput() != pDstPort->isInput() &&
            pSrcPort->getTaskId() != pDstPort->getTaskId() &&
            pSrcPort->connectionExists(pDstPort) == false)
        {
            if(pSrcPort->isInput())
                std::swap(pSrcPort, pDstPort);

            emit doItemConnection(pSrcPort->getTaskId(), pSrcPort->getIndex(), pDstPort->getTaskId(), pDstPort->getIndex());
        }
    }

    if(m_pWorkflowConnectionTmp)
    {
        removeItem(m_pWorkflowConnectionTmp);
        delete m_pWorkflowConnectionTmp;
        m_pWorkflowConnectionTmp = nullptr;
    }
}

void CWorkflowScene::updateNearestPorts(QPointF pos)
{
    CWorkflowPortItem* pCurrPort = static_cast<CWorkflowPortItem*>(m_pCurWorkflowItem);
    assert(pCurrPort != nullptr);
    bool bIn = pCurrPort->isInput();
    auto allItems = items();
    for(auto it : allItems)
    {
        if(it->type() == CWorkflowPortItem::Type) // Get only ports
        {
            auto pPort = static_cast<CWorkflowPortItem*>(it);
            bool bTest1 = bIn && pPort->isInput(); // Current port is input and can be input again
            bool bTest2 = !bIn && // Current port is an output
                    pPort->isInput() && // Port is an input
                    it->parentItem() != m_pCurWorkflowItem->parentItem(); // Test if same protocol task

            if(bTest1 || bTest2)
                pPort->updateShape(pos);
        }
    }
}

void CWorkflowScene::updatePortZoom(QPointF pos)
{
    auto allItems = items();
    for(auto it : allItems)
        if(it->type() == CWorkflowPortItem::Type)
        {
            auto pPortItem = static_cast<CWorkflowPortItem*>(it);
            pPortItem->updateShape(pos);
        }
}

void CWorkflowScene::updateDropCellRubber(QPointF pos)
{
    auto cell = positionToGrid(pos);
    if(isValidDropCell(cell) == false)
        return;

    if(m_pDropCellRubber == nullptr)
    {
        auto pRubber = new QRubberBand(QRubberBand::Rectangle);
        pRubber->resize(m_cellSize);
        pRubber->show();
        m_pDropCellRubber = new QGraphicsProxyWidget;
        m_pDropCellRubber->setWidget(pRubber);
        m_pDropCellRubber->setOpacity(0.3);
        addItem(m_pDropCellRubber);
    }

    QPointF scenePos(cell.second * m_cellSize.width(), cell.first * m_cellSize.height());
    m_pDropCellRubber->setPos(scenePos);
}

void CWorkflowScene::updateGraphGridSize(const QPair<int, int> &cell)
{
    size_t nbRow = m_graphGrid.size(DataDimension::Y);
    size_t nbCol = m_graphGrid.size(DataDimension::X);

    if(cell.first >= (int)nbRow)
        m_graphGrid.setDimension(DataDimension::Y, cell.first + 1);

    if(cell.second >= (int)nbCol)
        m_graphGrid.setDimension(DataDimension::X, cell.second + 1);
}

void CWorkflowScene::updateSceneFromGraphGrid()
{
    size_t index = 0;
    for(int i=0; i<(int)m_graphGrid.size(DataDimension::Y); ++i)
    {
        for(int j=0; j<(int)m_graphGrid.size(DataDimension::X); ++j)
        {
            if(m_graphGrid[index] != nullptr)
                setPosition(m_graphGrid[index], i, j);

            index++;
        }
    }
}

void CWorkflowScene::updateAllConnections()
{
    auto itemList = items();

    for(int i=0; i<itemList.size(); ++i)
        if(itemList[i]->type() == CWorkflowConnection::Type)
        {
            auto pConnItem = static_cast<CWorkflowConnection*>(itemList[i]);
            pConnItem->updatePath();
        }
}

void CWorkflowScene::endItemDrag(QPointF pos)
{
    auto newCell = positionToGrid(pos);
    auto oldCell = positionToGrid(m_dragStartPos);

    if(newCell != oldCell)
    {
        if(isValidDropCell(newCell) == false)
            newCell = positionToGrid(m_dragStartPos);
        else
        {
            //Update graph grid structure
            setGraphGridItem(nullptr, oldCell);
            updateGraphGridSize(newCell);
            setGraphGridItem(m_pCurWorkflowItem, newCell);
        }

        //Set position in the scene
        setPosition(m_pCurWorkflowItem, newCell.first, newCell.second);
        updateCandidateTask();
    }
    else
        setPosition(m_pCurWorkflowItem, oldCell.first, oldCell.second);

    if(m_pDropCellRubber)
    {
        delete m_pDropCellRubber;
        m_pDropCellRubber = nullptr;
    }
    m_bItemDragged = false;
}

QPoint CWorkflowScene::mapToScreen(QPointF scenePos)
{
    assert(views().size() > 0);
    QGraphicsView* pView = views().first();
    QPoint viewPos = pView->mapFromScene(scenePos);
    QPoint screenPos = pView->mapToGlobal(viewPos);
    return screenPos;
}

QPair<int, int> CWorkflowScene::positionToGrid(const QPointF &pos) const
{
    int row = (int)pos.y() / m_cellSize.height();
    int col = (int)pos.x() / m_cellSize.width();
    return QPair<int, int>(row, col);
}

QPointF CWorkflowScene::gridToPosition(int row, int column) const
{
    QPointF center;
    center.setX(column * m_cellSize.width() + m_cellSize.width()/2);
    center.setY(row * m_cellSize.height() + m_cellSize.height()/2);
    return center;
}

bool CWorkflowScene::isWidthReducible(QRectF viewRect)
{
    QRectF boundRect = getItemsBoundingRect();
    return boundRect.width() < viewRect.width();
}

bool CWorkflowScene::isHeightReducible(QRectF viewRect)
{
    QRectF boundRect = getItemsBoundingRect();
    return boundRect.height() < viewRect.height();
}

void CWorkflowScene::checkSceneRectChange()
{
    QRectF rcScene = sceneRect();
    QRectF viewRect = getViewRect();
    QRectF displayRect = getItemsBoundingRect();

    displayRect = displayRect.united(viewRect);
    if(displayRect != rcScene)
        setSceneRect(displayRect);
}

void CWorkflowScene::centerInputArea()
{
    auto rect = sceneRect();
    auto centerPos = m_pInputArea->scenePos();
    centerPos.setY(rect.center().ry() - m_pInputArea->getSize().height()/2);
    m_pInputArea->setPos(centerPos);

    updateAllConnections();
}

void CWorkflowScene::onPortClicked(CWorkflowPortItem *pPort, const QPointF &pos)
{
    assert(pPort);
    IODataType type = pPort->getDataType();
    if(type == IODataType::INPUT_GRAPHICS)
        openGraphicsLayerChoiceDlg(pPort, pos);
    else if(type == IODataType::FOLDER_PATH)
        openFolderPathChoiceDlg(pPort, pos);
}

void CWorkflowScene::openProcessDlg()
{
    emit doShowProcessPopup();
}

void CWorkflowScene::openGraphicsLayerChoiceDlg(CWorkflowPortItem* pPort, const QPointF& pos)
{
    emit doQueryGraphicsProxyModel();

    //Set position of the dialog
    setDialogPosition(&m_layerChoiceDlg, pos);

    int ret = m_layerChoiceDlg.exec();
    if(ret == QDialog::Accepted)
    {
        auto layerIndex = m_layerChoiceDlg.getSelectedLayer();
        emit doSetGraphicsLayerInput(pPort->getTaskId(), (int)pPort->getIndex(), layerIndex);
    }
}

void CWorkflowScene::openFolderPathChoiceDlg(CWorkflowPortItem *pPort, const QPointF &pos)
{
    Q_UNUSED(pos);
    QSettings ikomiaSettings;
    auto pParent = dynamic_cast<QWidget*>(parent());

    QString dir = QFileDialog::getExistingDirectory(pParent, tr("Choose folder"), ikomiaSettings.value(_DefaultDirImg).toString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | CSettingsManager::dialogOptions());
    if(!dir.isEmpty())
        emit doSetFolderPathInput(pPort->getTaskId(), pPort->getIndex(), dir);
}

