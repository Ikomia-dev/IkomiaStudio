#include <QRubberBand>
#include "Main/AppTools.hpp"
#include "CProtocolScene.h"
#include "CProtocolItem.h"
#include "CProtocolPortItem.h"
#include "CProtocolConnection.h"
#include "CProtocolIOArea.h"
#include "CProtocolDummyItem.h"

CProtocolScene::CProtocolScene(QWidget* pParent)
{
    // Do not call QGraphicsScene(pParent) to avoid automatic deletion of child items
    // -> cause double delete of CProtocolConnection instead...
    Q_UNUSED(pParent);
    Dimensions dims = {{DataDimension::Y, 1}, {DataDimension::X, 1}};
    m_graphGrid = CndArray<QGraphicsItem*>(dims);
    m_rootId = boost::graph_traits<ProtocolGraph>::null_vertex();
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
    createIOAreas();

    connect(this, &CProtocolScene::selectionChanged, this, &CProtocolScene::onSelectionChanged);
}

QSize CProtocolScene::getCellSize() const
{
    return m_cellSize;
}

CProtocolIOArea *CProtocolScene::getInputArea() const
{
    return m_pInputArea;
}

void CProtocolScene::setSelectedItem(QGraphicsItem *pItem)
{
    if(m_pCurProtocolItem != nullptr)
        m_pCurProtocolItem->setSelected(false);

    pItem->setSelected(true);
    m_pCurProtocolItem = pItem;
}

void CProtocolScene::setRootId(const ProtocolVertex &id)
{
    m_rootId = id;
    m_pInputArea->setTaskId(id);
}

void CProtocolScene::setProtocolStarted(bool bStarted)
{
    m_bProtocolStarted = bStarted;
    m_pInputArea->activateActions(bStarted);
}

void CProtocolScene::setTaskActionFlag(const ProtocolVertex &id, CProtocolTask::ActionFlag action, bool bEnable)
{
    emit doSetTaskActionFlag(id, action, bEnable);
}

void CProtocolScene::setCellSize(QSize cellSize)
{
    m_cellSize = cellSize;
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
}

void CProtocolScene::setItemSize(QSize itemSize)
{
    m_itemSize = itemSize;
}

void CProtocolScene::setInputInfo(const CDataInfoPtr &info, int index)
{
    if(m_pInputArea)
        m_pInputArea->setPortInfo(info, index);
}

void CProtocolScene::initInputs()
{
    for(int i=0; i<m_pInputArea->getPortCount(); ++i)
        m_pInputArea->updatePort(i, nullptr, false);
}

void CProtocolScene::updateInput(int index, const ProtocolTaskIOPtr& inputPtr, bool isBatch)
{
    if(index < m_pInputArea->getPortCount())
        m_pInputArea->updatePort(index, inputPtr, isBatch);
    else
        m_pInputArea->addPort(inputPtr, isBatch);
}

//Add item at candidate position
void CProtocolScene::addTaskItem(CProtocolItem *pItem)
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
void CProtocolScene::addTaskItem(CProtocolItem *pItem, CProtocolItem *pParent)
{
    //Clear old position of dummy item
    auto dummyCell = positionToGrid(m_pDummyItem->pos());
    auto index = getGraphGridIndex(dummyCell);

    if(index != SIZE_MAX &&  m_graphGrid[index] && m_graphGrid[index]->type() == CProtocolDummyItem::Type)
        m_graphGrid[index] = nullptr;

    //Insertion dans la scene
    addItem(pItem);
    //Mise à jour de la structure du graphe
    addItemToGraphGrid(pItem, pParent);
    updateSceneFromGraphGrid();
}

QPair<int,int> CProtocolScene::getCandidateCell(CProtocolItem *pItem) const
{
    assert(pItem);
    assert(m_pDummyItem);
    return positionToGrid(m_pDummyItem->pos());
}

QPair<int,int> CProtocolScene::getCellFromParent(CProtocolItem *pItem, CProtocolItem *pParent, int childIndex) const
{
    assert(pItem);
    QPair<int,int> parentCell(0, 0);

    if(pParent != nullptr)
        parentCell = positionToGrid(pParent->pos());

    return QPair<int,int>(parentCell.first + childIndex, parentCell.second + 1);
}

size_t CProtocolScene::getGraphGridIndex(const QPair<int, int> &cell) const
{
    DimensionIndices indices = {{DataDimension::Y, cell.first}, {DataDimension::X, cell.second}};
    return m_graphGrid.index(indices);
}

bool CProtocolScene::isGraphGridRowEmpty(int row, int firstColumn, int lastColumn) const
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

bool CProtocolScene::isValidDropCell(const QPair<int,int>& cell) const
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

void CProtocolScene::deleteInput(int index)
{
    assert(m_pInputArea);
    m_pInputArea->removePort(index);
}

void CProtocolScene::deleteConnection(CProtocolConnection *pConnection, bool bLater, bool bNotifyView, bool bNotifyModel)
{
    assert(pConnection);

    if(bNotifyView)
        emit doDeleteConnection(pConnection->getId(), bNotifyModel);

    if(bLater)
        pConnection->deleteLater();
    else
        delete pConnection;
}

QRectF CProtocolScene::resize(const QRectF &viewRect)
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

void CProtocolScene::updateCandidateTask()
{
    if(m_pDummyItem == nullptr)
    {
        m_pDummyItem = new CProtocolDummyItem;
        m_pDummyItem->setSize(m_itemSize);
        m_pDummyItem->setColor(qApp->palette().text().color());
        addItem(m_pDummyItem);
    }

    //Clear old position of dummy item
    auto dummyCell = positionToGrid(m_pDummyItem->pos());
    auto index = getGraphGridIndex(dummyCell);

    if(index != SIZE_MAX &&  m_graphGrid[index] && m_graphGrid[index]->type() == CProtocolDummyItem::Type)
        m_graphGrid[index] = nullptr;

    addItemToGraphGrid(m_pDummyItem, m_pCurProtocolItem);
    removeGraphGridEmptyRows();
    updateSceneFromGraphGrid();
}

void CProtocolScene::clearAll()
{
    m_graphGrid.clear();
    Dimensions dims = {{DataDimension::Y, 1}, {DataDimension::X, 1}};
    m_graphGrid.setDimensions(dims);

    m_rootId = boost::graph_traits<ProtocolGraph>::null_vertex();
    m_pCurProtocolItem = nullptr;
    m_pProtocolConnectionTmp = nullptr;
    m_pDropCellRubber = nullptr;
    m_pInputArea = nullptr;
    m_pOutputArea = nullptr;
    m_pDummyItem = nullptr;
    clear();
    createIOAreas();
}

void CProtocolScene::queryProjectDataProxyModel(const TreeItemType& inputType, const std::vector<DataDimension> &filters)
{
    std::vector<TreeItemType> inputTypes;
    inputTypes.push_back(inputType);
    emit doQueryProjectDataProxyModel(inputTypes, filters);
}

void CProtocolScene::retrieveIOInfo(const ProtocolVertex &taskId, size_t index, bool bInput)
{
    emit doQueryIOInfo(taskId, static_cast<int>(index), bInput);
}

void CProtocolScene::onSetGraphicsProxyModel(CProjectGraphicsProxyModel *pModel)
{
    m_layerChoiceDlg.setModel(pModel);
}

void CProtocolScene::onSetProjectDataProxyModel(CProjectDataProxyModel *pModel)
{
    if(m_pInputArea)
        m_pInputArea->setProjectDataProxyModel(pModel);
}

void CProtocolScene::onSelectionChanged()
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

void CProtocolScene::onIOAreaChanged()
{
    // This slot is used when we add input data to the input area
    m_pInputArea->update();
    checkSceneRectChange();
    centerInputArea();
}

void CProtocolScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() & Qt::LeftButton)
    {
        auto pItem = itemAt(mouseEvent->scenePos(), QTransform());
        m_bNewItemClicked = pItem != m_pCurProtocolItem;
        m_pCurProtocolItem = pItem;

        if(m_pCurProtocolItem)
        {
            switch(m_pCurProtocolItem->type())
            {
                case CProtocolItem::Type:
                    m_dragStartPos = mouseEvent->scenePos();
                    break;

                case CProtocolPortItem::Type:
                    m_dragStartPos = mouseEvent->scenePos();
                    updateNearestPorts(mouseEvent->scenePos());
                    return;

                case CProtocolIOArea::Type:
                    if(m_pCurProtocolItem == m_pInputArea)
                        emit doSelectionChange(boost::graph_traits<ProtocolGraph>::null_vertex());
                    break;

                case CProtocolDummyItem::Type:
                    return;
            }
        }
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void CProtocolScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->buttons() & Qt::LeftButton)
    {        
        if(m_pCurProtocolItem)
        {
            qreal moveDistance = (mouseEvent->scenePos() - m_dragStartPos).manhattanLength();
            bool bMinDistance = moveDistance >= QApplication::startDragDistance();

            if(m_pCurProtocolItem->type() == CProtocolItem::Type && bMinDistance)
            {
                m_bItemDragged = true;
                updateDropCellRubber(mouseEvent->scenePos());
                static_cast<CProtocolItem*>(m_pCurProtocolItem)->updateConnections();
            }
            else if(m_pCurProtocolItem->type() == CProtocolPortItem::Type)
            {
                if(m_pProtocolConnectionTmp == nullptr)
                {
                    CProtocolPortItem* pPort = static_cast<CProtocolPortItem*>(m_pCurProtocolItem);
                    CProtocolPortItem* pPortTmp = pPort;
                    if(pPort->isInput())
                    {
                        QVector<CProtocolConnection*> connections = pPort->getConnections();
                        if(connections.size() > 0)
                        {
                            CProtocolConnection* pConnect = connections[0];
                            pPortTmp = pConnect->getSourcePort();
                            pConnect->onDelete();
                        }
                        else
                            return;
                    }
                    m_pProtocolConnectionTmp = new CProtocolConnection;
                    m_pProtocolConnectionTmp->setSourcePort(pPortTmp);
                    addItem(m_pProtocolConnectionTmp);
                }
                m_pProtocolConnectionTmp->updatePath(mouseEvent->scenePos());
                updateNearestPorts(mouseEvent->scenePos());
                return;
            }
        }
    }
    updatePortZoom(mouseEvent->scenePos());
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void CProtocolScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() & Qt::LeftButton)
    {
        if(m_pCurProtocolItem != nullptr)
        {
            if(m_pCurProtocolItem->type() == CProtocolItem::Type)
            {
                if(m_bItemDragged == true)
                    endItemDrag(mouseEvent->scenePos());
                else if(m_bNewItemClicked == true)
                    emit doSelectionChange(static_cast<CProtocolItem*>(m_pCurProtocolItem)->getId());
            }
            else if(m_pCurProtocolItem->type() == CProtocolPortItem::Type)
            {
                if(m_pProtocolConnectionTmp)
                    finalizeCurrentConnection(mouseEvent->scenePos());
                else
                {
                    auto pPortItem = static_cast<CProtocolPortItem*>(m_pCurProtocolItem);
                    if(pPortItem->isClickable())
                        onPortClicked(pPortItem, mouseEvent->screenPos());
                }
                updateNearestPorts(QPointF());
            }
            else if(m_pCurProtocolItem->type() == CProtocolDummyItem::Type)
                openProcessDlg();
        }
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void CProtocolScene::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        if(m_bItemDragged == false)
            deleteSelectedItems();
    }
    QGraphicsScene::keyPressEvent(event);
}

void CProtocolScene::setGraphGridItem(QGraphicsItem *pItem, QPair<int, int> &cell)
{
    auto index = getGraphGridIndex(cell);
    if(index != SIZE_MAX)
        m_graphGrid[index] = pItem;
}

void CProtocolScene::setPosition(QGraphicsItem *pItem, int row, int column)
{
    assert(pItem);
    QRectF itemRect = pItem->boundingRect();
    auto cellCenter = gridToPosition(row, column);
    pItem->setPos(cellCenter.x() - itemRect.width()/2, cellCenter.y() - itemRect.height()/2);

    if(pItem->type() == CProtocolItem::Type)
    {
        auto pProtocolItem = static_cast<CProtocolItem*>(pItem);
        pProtocolItem->updateConnections();
    }
    checkSceneRectChange();
}

void CProtocolScene::setDialogPosition(QDialog *pDialog, const QPointF &pos)
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

QRectF CProtocolScene::getItemsBoundingRect() const
{
    QRectF boundRect;
    auto itemList = items();

    for(int i=0; i<itemList.size(); ++i)
    {
        if(itemList[i]->type() == CProtocolItem::Type || itemList[i]->type() == CProtocolDummyItem::Type)
        {
            QRectF itemBoundRect = itemList[i]->boundingRect();
            QRectF itemRect = QRectF(itemList[i]->x(), itemList[i]->y(), itemBoundRect.width(), itemBoundRect.height());
            boundRect = boundRect.united(itemRect);
        }
    }
    return boundRect;
}

QRectF CProtocolScene::getViewRect() const
{
    assert(views().size() > 0);
    QGraphicsView* pView = views().first();
    return pView->viewport()->rect();
}

int CProtocolScene::getTaskItemCount() const
{
    int inputNb = m_pInputArea->getPortCount();
    int outputNb = m_pOutputArea->getPortCount();
    return items().size() - inputNb - outputNb - 2;
}

void CProtocolScene::createIOAreas()
{
    QPalette p = qApp->palette();

    m_pInputArea = new CProtocolIOArea(tr("Inputs"), true);
    m_pInputArea->setColorBg(p.dark().color());
    m_pInputArea->setLineColor(p.dark().color());
    addItem(m_pInputArea);

    centerInputArea();

    connect(m_pInputArea, &CProtocolIOArea::doIOAreaChanged, this, &CProtocolScene::onIOAreaChanged);

    /*QRectF rc = sceneRect();
    m_pOutputArea = new CProtocolIOArea(tr("Outputs"), false);
    m_pOutputArea->setPos(rc.right() - m_pOutputArea->getSize().width(), 0);
    m_pOutputArea->setColorBg(p.dark().color());
    m_pOutputArea->setLineColor(p.dark().color());
    addItem(m_pOutputArea);*/
}

void CProtocolScene::addItemToGraphGrid(QGraphicsItem *pItem, QGraphicsItem *pParent)
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

void CProtocolScene::deleteSelectedItems()
{
    auto items = selectedItems();
    for(int i=0; i<items.size(); ++i)
    {
        if(items[i]->type() == CProtocolItem::Type)
        {
            auto pItem = static_cast<CProtocolItem*>(items[i]);
            deleteTaskItem(pItem);
        }
    }
    checkSceneRectChange();
}

void CProtocolScene::deleteTaskItem(CProtocolItem *pItem)
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

    //Remove task and notify CProtocolView
    if(pItem == m_pCurProtocolItem)
        m_pCurProtocolItem = nullptr;

    auto taskId = pItem->getId();
    delete pItem;
    emit doItemDeleted(taskId);
}

void CProtocolScene::removeGraphGridEmptyRows()
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

void CProtocolScene::finalizeCurrentConnection(QPointF pos)
{
    auto itemList = items(pos);
    if(itemList.size() > 0)
    {
        auto pSrcPort = m_pProtocolConnectionTmp->getSourcePort();
        CProtocolPortItem* pDstPort = nullptr;

        for(int i=0; i<itemList.size(); ++i)
        {
            if(itemList[i]->type() == CProtocolPortItem::Type)
            {
                pDstPort = static_cast<CProtocolPortItem*>(itemList[i]);
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

    if(m_pProtocolConnectionTmp)
    {
        removeItem(m_pProtocolConnectionTmp);
        delete m_pProtocolConnectionTmp;
        m_pProtocolConnectionTmp = nullptr;
    }
}

void CProtocolScene::updateNearestPorts(QPointF pos)
{
    CProtocolPortItem* pCurrPort = static_cast<CProtocolPortItem*>(m_pCurProtocolItem);
    assert(pCurrPort != nullptr);
    bool bIn = pCurrPort->isInput();
    auto allItems = items();
    for(auto it : allItems)
    {
        if(it->type() == CProtocolPortItem::Type) // Get only ports
        {
            auto pPort = static_cast<CProtocolPortItem*>(it);
            bool bTest1 = bIn && pPort->isInput(); // Current port is input and can be input again
            bool bTest2 = !bIn && // Current port is an output
                    pPort->isInput() && // Port is an input
                    it->parentItem() != m_pCurProtocolItem->parentItem(); // Test if same protocol task

            if(bTest1 || bTest2)
                pPort->updateShape(pos);
        }
    }
}

void CProtocolScene::updatePortZoom(QPointF pos)
{
    auto allItems = items();
    for(auto it : allItems)
        if(it->type() == CProtocolPortItem::Type)
        {
            auto pPortItem = static_cast<CProtocolPortItem*>(it);
            pPortItem->updateShape(pos);
        }
}

void CProtocolScene::updateDropCellRubber(QPointF pos)
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

void CProtocolScene::updateGraphGridSize(const QPair<int, int> &cell)
{
    size_t nbRow = m_graphGrid.size(DataDimension::Y);
    size_t nbCol = m_graphGrid.size(DataDimension::X);

    if(cell.first >= (int)nbRow)
        m_graphGrid.setDimension(DataDimension::Y, cell.first + 1);

    if(cell.second >= (int)nbCol)
        m_graphGrid.setDimension(DataDimension::X, cell.second + 1);
}

void CProtocolScene::updateSceneFromGraphGrid()
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

void CProtocolScene::updateAllConnections()
{
    auto itemList = items();

    for(int i=0; i<itemList.size(); ++i)
        if(itemList[i]->type() == CProtocolConnection::Type)
        {
            auto pConnItem = static_cast<CProtocolConnection*>(itemList[i]);
            pConnItem->updatePath();
        }
}

void CProtocolScene::endItemDrag(QPointF pos)
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
            setGraphGridItem(m_pCurProtocolItem, newCell);
        }

        //Set position in the scene
        setPosition(m_pCurProtocolItem, newCell.first, newCell.second);
        updateCandidateTask();
    }
    else
        setPosition(m_pCurProtocolItem, oldCell.first, oldCell.second);

    if(m_pDropCellRubber)
    {
        delete m_pDropCellRubber;
        m_pDropCellRubber = nullptr;
    }
    m_bItemDragged = false;
}

QPoint CProtocolScene::mapToScreen(QPointF scenePos)
{
    assert(views().size() > 0);
    QGraphicsView* pView = views().first();
    QPoint viewPos = pView->mapFromScene(scenePos);
    QPoint screenPos = pView->mapToGlobal(viewPos);
    return screenPos;
}

QPair<int, int> CProtocolScene::positionToGrid(const QPointF &pos) const
{
    int row = (int)pos.y() / m_cellSize.height();
    int col = (int)pos.x() / m_cellSize.width();
    return QPair<int, int>(row, col);
}

QPointF CProtocolScene::gridToPosition(int row, int column) const
{
    QPointF center;
    center.setX(column * m_cellSize.width() + m_cellSize.width()/2);
    center.setY(row * m_cellSize.height() + m_cellSize.height()/2);
    return center;
}

bool CProtocolScene::isWidthReducible(QRectF viewRect)
{
    QRectF boundRect = getItemsBoundingRect();
    return boundRect.width() < viewRect.width();
}

bool CProtocolScene::isHeightReducible(QRectF viewRect)
{
    QRectF boundRect = getItemsBoundingRect();
    return boundRect.height() < viewRect.height();
}

void CProtocolScene::checkSceneRectChange()
{
    QRectF rcScene = sceneRect();
    QRectF viewRect = getViewRect();
    QRectF displayRect = getItemsBoundingRect();

    displayRect = displayRect.united(viewRect);
    if(displayRect != rcScene)
        setSceneRect(displayRect);
}

void CProtocolScene::centerInputArea()
{
    auto rect = sceneRect();
    auto centerPos = m_pInputArea->scenePos();
    centerPos.setY(rect.center().ry() - m_pInputArea->getSize().height()/2);
    m_pInputArea->setPos(centerPos);

    updateAllConnections();
}

void CProtocolScene::onPortClicked(CProtocolPortItem *pPort, const QPointF &pos)
{
    assert(pPort);
    IODataType type = pPort->getDataType();
    if(type == IODataType::INPUT_GRAPHICS)
        openGraphicsLayerChoiceDlg(pPort, pos);
    else if(type == IODataType::FOLDER_PATH)
        openFolderPathChoiceDlg(pPort, pos);
}

void CProtocolScene::openProcessDlg()
{
    emit doShowProcessPopup();
}

void CProtocolScene::openGraphicsLayerChoiceDlg(CProtocolPortItem* pPort, const QPointF& pos)
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

void CProtocolScene::openFolderPathChoiceDlg(CProtocolPortItem *pPort, const QPointF &pos)
{
    Q_UNUSED(pos);
    QSettings ikomiaSettings;
    auto pParent = dynamic_cast<QWidget*>(parent());

    QString dir = QFileDialog::getExistingDirectory(pParent, tr("Choose folder"), ikomiaSettings.value(_DefaultDirImg).toString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | CSettingsManager::dialogOptions());
    if(!dir.isEmpty())
        emit doSetFolderPathInput(pPort->getTaskId(), pPort->getIndex(), dir);
}

