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
#include "CWorkflowView.h"
#include "CWorkflowScene.h"
#include "CWorkflowItem.h"
#include "CWorkflowPortItem.h"
#include "CWorkflowConnection.h"
#include "CWorkflowIOArea.h"

CWorkflowView::CWorkflowView(QWidget* parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setInteractive(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);
    //setDragMode(QGraphicsView::ScrollHandDrag);

    m_pScene = new CWorkflowScene(this);
    m_pScene->setCellSize(m_cellSize);
    m_pScene->setItemSize(m_itemSize);
    setScene(m_pScene);
    setMinimumHeight(m_cellSize.height() + 4*frameWidth());
    setSceneRect(0, 0, m_cellSize.width(), m_cellSize.height());
}

void CWorkflowView::setModel(CWorkflowManager *pModel)
{
    m_pModel = pModel;
    initConnections();
}

void CWorkflowView::manageWorkflowCreated()
{
    initWorkflowScene();
}

//Add task as child of the given task
void CWorkflowView::addTask(const WorkflowTaskPtr &pTask, const WorkflowVertex &id, const WorkflowVertex &parentId)
{
    assert(pTask);

    //Création de l'item pour la nouvelle tache
    auto pItem = createItem(pTask, id);
    if(!pItem)
        throw CException(CoreExCode::CREATE_FAILED, "Workflow item creation failed", __func__, __FILE__, __LINE__);

    //Get parent item
    CWorkflowItem* pParent = nullptr;
    auto it = m_idToItem.find(parentId);

    if(it != m_idToItem.end())
        pParent = it->second;

    m_pScene->addTaskItem(pItem, pParent);
    m_idToItem.insert(std::make_pair(id, pItem));
}

//Add task at the current candidate position
void CWorkflowView::addTask(const WorkflowTaskPtr& pTask, const WorkflowVertex &id)
{
    assert(pTask);

    //Création de l'item pour la nouvelle tache
    auto pItem = createItem(pTask, id);
    if(!pItem)
        throw CException(CoreExCode::CREATE_FAILED, "Workflow item creation failed", __func__, __FILE__, __LINE__);

    m_pScene->addTaskItem(pItem);
    m_idToItem.insert(std::make_pair(id, pItem));
}

void CWorkflowView::clear()
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

void CWorkflowView::zoomInit()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setTransform(QTransform());
}

void CWorkflowView::zoomIn()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scale(m_scaleFactor, m_scaleFactor);
}

void CWorkflowView::zoomOut()
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scale(1.0 / m_scaleFactor, 1.0 / m_scaleFactor);
}

CWorkflowScene*CWorkflowView::getScene()
{
    return m_pScene;
}

void CWorkflowView::updateSceneRect(const QRectF &rect)
{
    setSceneRect(rect);
    viewport()->update();
}

void CWorkflowView::onInputChanged(int index)
{
    assert(m_pModel);
    assert(m_pScene);
    m_pScene->updateInput(index, m_pModel->getInput(index), m_pModel->isBatchInput(index));
    m_pScene->update();
}

void CWorkflowView::onInputAssigned(const WorkflowVertex &id, int portIndex, bool bAssigned)
{
    auto it = m_idToItem.find(id);
    if(it != m_idToItem.end())
        it->second->setInputPortAssigned(portIndex, bAssigned);
}

void CWorkflowView::onInputRemoved(int index)
{
    assert(m_pScene);
    m_pScene->deleteInput(index);
}

void CWorkflowView::onInputsCleared()
{
    assert(m_pScene);
    m_pScene->initInputs();
}

void CWorkflowView::onAddConnection(const WorkflowEdge &id, const WorkflowVertex &srcId, size_t srcIndex, const WorkflowVertex &dstId, size_t dstIndex)
{
    CWorkflowItem* pSrcItem = nullptr;

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

void CWorkflowView::onDeleteItem(const WorkflowVertex& id)
{
    assert(m_pModel);

    //Remove item from internal map
    auto it = m_idToItem.find(id);
    if(it != m_idToItem.end())
        m_idToItem.erase(it);

    m_pModel->onDeleteTask(id);
}

void CWorkflowView::onDeleteConnectionFromScene(const WorkflowEdge &id, bool bNotifyModel)
{
    assert(m_pModel);

    if(bNotifyModel == true)
        m_pModel->onDeleteEdge(id);

    auto it = m_idToConnection.find(id);
    if(it != m_idToConnection.end())
        m_idToConnection.erase(it);
}

void CWorkflowView::onDeleteConnectionFromModel(const WorkflowEdge &id)
{
    auto it = m_idToConnection.find(id);
    if(it != m_idToConnection.end())
    {
        m_pScene->deleteConnection(it->second, true, false, false);
        m_idToConnection.erase(it);
    }
}

void CWorkflowView::onSetActiveTask(const WorkflowVertex &id)
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

void CWorkflowView::onUpdateCandidateTask()
{
    m_pScene->updateCandidateTask();
}

void CWorkflowView::onSetTaskState(const WorkflowVertex& id, CWorkflowTask::State status, const QString& msg)
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

void CWorkflowView::onSetIOInfo(const CDataInfoPtr &info, const WorkflowVertex &taskId, int index, bool bInput)
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

void CWorkflowView::onUpdateTaskItem(const WorkflowTaskPtr &pTask, const WorkflowVertex &id)
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

void CWorkflowView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    //Scale the view / do the zoom
    if(event->angleDelta().y() > 0)
        scale(m_scaleFactor, m_scaleFactor);                // Zoom in
    else
        scale(1.0 / m_scaleFactor, 1.0 / m_scaleFactor);    // Zooming out
}

void CWorkflowView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    QRectF viewportRect = viewport()->rect();
    QRectF sceneRect = m_pScene->resize(viewportRect);
    setSceneRect(sceneRect);
}

void CWorkflowView::mousePressEvent(QMouseEvent *mouseEvent)
{
    QGraphicsView::mousePressEvent(mouseEvent);

    if(mouseEvent->buttons() & Qt::LeftButton)
    {
        m_dragStartPos = mouseEvent->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
}

void CWorkflowView::mouseMoveEvent(QMouseEvent *mouseEvent)
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

void CWorkflowView::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    QGraphicsView::mouseReleaseEvent(mouseEvent);

    auto pItem = itemAt(mouseEvent->pos());
    if(pItem == nullptr)
        viewport()->setCursor(Qt::OpenHandCursor);
}

void CWorkflowView::drawBackground(QPainter* painter, const QRectF& r)
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

void CWorkflowView::initConnections()
{
    //Scene -> view
    connect(m_pScene, &CWorkflowScene::doItemDeleted, this, &CWorkflowView::onDeleteItem);
    connect(m_pScene, &CWorkflowScene::doDeleteConnection, this, &CWorkflowView::onDeleteConnectionFromScene);
}

void CWorkflowView::initWorkflowScene()
{
    m_pScene->setWorkflowStarted(true);
    m_pScene->setRootId(m_pModel->getRootId());
    m_pScene->updateCandidateTask();
}

CWorkflowItem *CWorkflowView::createItem(std::shared_ptr<CWorkflowTask> pTask, const WorkflowVertex &id)
{
    assert(pTask);
    auto pItem = new CWorkflowItem(QString::fromStdString(pTask->getName()), id);
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

CWorkflowConnection* CWorkflowView::connectItems(const WorkflowEdge &id, CWorkflowItem *pSrcItem, size_t srcIndex, CWorkflowItem *pDstItem, size_t dstIndex)
{
    assert(pDstItem);

    CWorkflowConnection* pConnection = new CWorkflowConnection(id);
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



