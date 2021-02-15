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

#include "Main/AppTools.hpp"
#include "CProtocolView.h"
#include "CProtocolScene.h"
#include "CProtocolItem.h"
#include "CProtocolPortItem.h"
#include "CProtocolConnection.h"
#include "CProtocolIOArea.h"

CProtocolView::CProtocolView(QWidget* parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setInteractive(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    m_pScene = new CProtocolScene(this);
    m_pScene->setCellSize(m_cellSize);
    m_pScene->setItemSize(m_itemSize);
    setScene(m_pScene);
    setMinimumHeight(m_cellSize.height() + 4*frameWidth());
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
}

void CProtocolView::setModel(CProtocolManager *pModel)
{
    m_pModel = pModel;
    initConnections();
}

void CProtocolView::manageProtocolCreated()
{
    initProtocolScene();
}

//Add task as child of the given task
void CProtocolView::addTask(const ProtocolTaskPtr &pTask, const ProtocolVertex &id, const ProtocolVertex &parentId)
{
    assert(pTask);

    //Création de l'item pour la nouvelle tache
    auto pItem = createItem(pTask, id);
    if(!pItem)
        throw CException(CoreExCode::CREATE_FAILED, "Workflow item creation failed", __func__, __FILE__, __LINE__);

    //Get parent item
    CProtocolItem* pParent = nullptr;
    auto it = m_idToItem.find(parentId);

    if(it != m_idToItem.end())
        pParent = it->second;

    m_pScene->addTaskItem(pItem, pParent);
    m_idToItem.insert(std::make_pair(id, pItem));
}

//Add task at the current candidate position
void CProtocolView::addTask(const ProtocolTaskPtr& pTask, const ProtocolVertex &id)
{
    assert(pTask);

    //Création de l'item pour la nouvelle tache
    auto pItem = createItem(pTask, id);
    if(!pItem)
        throw CException(CoreExCode::CREATE_FAILED, "Workflow item creation failed", __func__, __FILE__, __LINE__);

    m_pScene->addTaskItem(pItem);
    m_idToItem.insert(std::make_pair(id, pItem));
}

void CProtocolView::clear()
{
    if(m_pScene)
    {
        //Delete connections first to respect port <-> connection association
        for(auto it=m_idToConnection.begin(); it!=m_idToConnection.end(); ++it)
            m_pScene->deleteConnection(it->second, false, false, false);

        m_pScene->clearAll();
    }
    m_idToItem.clear();
    m_idToConnection.clear();
}

void CProtocolView::zoomInit()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setTransform(QTransform());
}

void CProtocolView::zoomIn()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scale(m_scaleFactor, m_scaleFactor);
}

void CProtocolView::zoomOut()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scale(1.0 / m_scaleFactor, 1.0 / m_scaleFactor);
}

CProtocolScene*CProtocolView::getScene()
{
    return m_pScene;
}

void CProtocolView::updateSceneRect(const QRectF &rect)
{
    setSceneRect(rect);
    viewport()->update();
}

void CProtocolView::onInputChanged(int index)
{
    assert(m_pModel);
    assert(m_pScene);
    m_pScene->updateInput(index, m_pModel->getInput(index), m_pModel->isBatchInput(index));
    m_pScene->update();
}

void CProtocolView::onInputAssigned(const ProtocolVertex &id, int portIndex, bool bAssigned)
{
    auto it = m_idToItem.find(id);
    if(it != m_idToItem.end())
        it->second->setInputPortAssigned(portIndex, bAssigned);
}

void CProtocolView::onInputRemoved(int index)
{
    assert(m_pScene);
    m_pScene->deleteInput(index);
}

void CProtocolView::onInputsCleared()
{
    assert(m_pScene);
    m_pScene->initInputs();
}

void CProtocolView::onAddConnection(const ProtocolEdge &id, const ProtocolVertex &srcId, size_t srcIndex, const ProtocolVertex &dstId, size_t dstIndex)
{
    CProtocolItem* pSrcItem = nullptr;

    if(m_pModel->isRoot(srcId) == false)
    {
        auto srcIt = m_idToItem.find(srcId);
        if(srcIt == m_idToItem.end())
            return;
        else
            pSrcItem = srcIt->second;
    }

    auto dstIt = m_idToItem.find(dstId);
    if(dstIt == m_idToItem.end())
        return;

    auto pConnection = connectItems(id, pSrcItem, srcIndex, dstIt->second, dstIndex);
    m_idToConnection.insert(std::make_pair(id, pConnection));
}

void CProtocolView::onDeleteItem(const ProtocolVertex& id)
{
    assert(m_pModel);

    //Remove item from internal map
    auto it = m_idToItem.find(id);
    if(it != m_idToItem.end())
        m_idToItem.erase(it);

    m_pModel->onDeleteTask(id);
}

void CProtocolView::onDeleteConnectionFromScene(const ProtocolEdge &id, bool bNotifyModel)
{
    assert(m_pModel);

    if(bNotifyModel == true)
        m_pModel->onDeleteEdge(id);

    auto it = m_idToConnection.find(id);
    if(it != m_idToConnection.end())
        m_idToConnection.erase(it);
}

void CProtocolView::onDeleteConnectionFromModel(const ProtocolEdge &id)
{
    auto it = m_idToConnection.find(id);
    if(it != m_idToConnection.end())
    {
        m_pScene->deleteConnection(it->second, true, false, false);
        m_idToConnection.erase(it);
    }
}

void CProtocolView::onSetActiveTask(const ProtocolVertex &id)
{
    if(m_pModel->isRoot(id))
        m_pScene->setSelectedItem(m_pScene->getInputArea());
    else
    {
        auto it = m_idToItem.find(id);
        if(it != m_idToItem.end())
            m_pScene->setSelectedItem(it->second);
    }
}

void CProtocolView::onUpdateCandidateTask()
{
    m_pScene->updateCandidateTask();
}

void CProtocolView::onSetTaskState(const ProtocolVertex& id, CProtocolTask::State status, const QString& msg)
{
    if(m_pModel->isRoot(id) == false)
    {
        auto srcItem = m_idToItem.find(id);
        if(srcItem == m_idToItem.end())
            return;

        srcItem->second->setStatus(status);
        if(!msg.isEmpty())
            srcItem->second->setStatusMsg(msg);
    }
}

void CProtocolView::onSetIOInfo(const CDataInfoPtr &info, const ProtocolVertex &taskId, int index, bool bInput)
{
    if(m_pModel->isRoot(taskId))
        m_pScene->setInputInfo(info, index);
    else
    {
        auto srcItem = m_idToItem.find(taskId);
        if(srcItem == m_idToItem.end())
            return;

        srcItem->second->setIOInfo(info, index, bInput);
    }
}

void CProtocolView::onUpdateTaskItem(const ProtocolTaskPtr &pTask, const ProtocolVertex &id)
{
    if(m_pModel->isRoot(id) == false)
    {
        auto it = m_idToItem.find(id);
        if(it != m_idToItem.end())
        {
            it->second->updatePorts(pTask);
            it->second->updateActions(pTask->getActionFlags());
        }
    }
}

void CProtocolView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    //Scale the view / do the zoom
    if(event->delta() > 0)
        scale(m_scaleFactor, m_scaleFactor);                // Zoom in
    else
        scale(1.0 / m_scaleFactor, 1.0 / m_scaleFactor);    // Zooming out
}

void CProtocolView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    QRectF viewportRect = viewport()->rect();
    QRectF sceneRect = m_pScene->resize(viewportRect);
    setSceneRect(sceneRect);
}

void CProtocolView::mousePressEvent(QMouseEvent *mouseEvent)
{
    QGraphicsView::mousePressEvent(mouseEvent);

    if(mouseEvent->buttons() & Qt::LeftButton)
    {
        m_dragStartPos = mouseEvent->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
}

void CProtocolView::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    QGraphicsView::mouseMoveEvent(mouseEvent);

    auto pItem = itemAt(mouseEvent->pos());
    if(pItem == nullptr)
    {
        if(mouseEvent->buttons() & Qt::LeftButton)
        {
            auto hPos = horizontalScrollBar()->value();
            auto vPos = verticalScrollBar()->value();
            auto scroll = m_dragStartPos - mouseEvent->pos();
            horizontalScrollBar()->setValue(hPos + scroll.x());
            verticalScrollBar()->setValue(vPos + scroll.y());
            viewport()->setCursor(Qt::ClosedHandCursor);
            m_dragStartPos = mouseEvent->pos();
        }
        else
            viewport()->setCursor(Qt::OpenHandCursor);
    }
    else
        viewport()->setCursor(Qt::ArrowCursor);
}

void CProtocolView::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    QGraphicsView::mouseReleaseEvent(mouseEvent);

    auto pItem = itemAt(mouseEvent->pos());
    if(pItem == nullptr)
        viewport()->setCursor(Qt::OpenHandCursor);
}

void CProtocolView::drawBackground(QPainter* painter, const QRectF& r)
{
    QGraphicsView::drawBackground(painter, r);

    auto drawGrid =
      [&](double gridStepW, double gridStepH)
      {
        QRect   windowRect = rect();
        QPointF tl = mapToScene(windowRect.topLeft());
        QPointF br = mapToScene(windowRect.bottomRight());

        double left   = std::floor(tl.x() / gridStepW - 0.5);
        double right  = std::floor(br.x() / gridStepW + 1.0);
        double bottom = std::floor(tl.y() / gridStepH - 0.5);
        double top    = std::floor (br.y() / gridStepH + 1.0);

        // vertical lines
        for (int xi = int(left); xi <= int(right); ++xi)
        {
          QLineF line(xi * gridStepW, bottom * gridStepH,
                      xi * gridStepW, top * gridStepH );

          painter->drawLine(line);
        }

        // horizontal lines
        for (int yi = int(bottom); yi <= int(top); ++yi)
        {
          QLineF line(left * gridStepW, yi * gridStepH,
                      right * gridStepW, yi * gridStepH );
          painter->drawLine(line);
        }
      };

    QPen pfine(QColor(60,60,60), 1.0);

    painter->setPen(pfine);
    drawGrid(m_cellSize.width()/10, m_cellSize.height()/10);

    QPen p(QColor(25,25,25), 1.0);

    painter->setPen(p);
    drawGrid(m_cellSize.width(), m_cellSize.height());
}

void CProtocolView::initConnections()
{
    //Scene -> view
    connect(m_pScene, &CProtocolScene::doItemDeleted, this, &CProtocolView::onDeleteItem);
    connect(m_pScene, &CProtocolScene::doDeleteConnection, this, &CProtocolView::onDeleteConnectionFromScene);
}

void CProtocolView::initProtocolScene()
{
    m_pScene->setProtocolStarted(true);
    m_pScene->setRootId(m_pModel->getRootId());
    m_pScene->updateCandidateTask();
}

CProtocolItem *CProtocolView::createItem(std::shared_ptr<CProtocolTask> pTask, const ProtocolVertex &id)
{
    assert(pTask);
    auto pItem = new CProtocolItem(QString::fromStdString(pTask->getName()), id);
    QPalette p = qApp->palette();
    pItem->setSize(m_itemSize);
    pItem->setPortRadius(m_portRadius);
    pItem->setHeaderColor(p.dark().color(), p.text().color());
    pItem->setBodyColor(p.window().color().lighter());
    pItem->setLineColor(p.dark().color(), p.highlight().color().darker());
    pItem->updatePorts(pTask);
    pItem->updateActions(pTask->getActionFlags());
    pItem->updateItem();
    return pItem;
}

CProtocolConnection* CProtocolView::connectItems(const ProtocolEdge &id, CProtocolItem *pSrcItem, size_t srcIndex, CProtocolItem *pDstItem, size_t dstIndex)
{
    assert(pDstItem);

    CProtocolConnection* pConnection = new CProtocolConnection(id);
    if(pSrcItem == nullptr)
    {
        //root task -> connection to input area
        m_pScene->updateInput((int)srcIndex, m_pModel->getInput(srcIndex), m_pModel->isBatchInput(srcIndex));
        auto pInputAreaItem = m_pScene->getInputArea();
        pConnection->setSourcePort(pInputAreaItem->getPort((int)srcIndex));
    }
    else
        pConnection->setSourcePort(pSrcItem->getOutputPort((int)srcIndex));

    pConnection->setTargetPort(pDstItem->getInputPort((int)dstIndex));
    pConnection->updatePath();
    m_pScene->addItem(pConnection);
    return pConnection;
}



