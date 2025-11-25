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

/**
 * @file      CImageScene.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CImageScene
 *
 * @details   Details
 */

#include "CImageScene.h"
#include <QGraphicsPixmapItem>
#include "CImageView.h"
#include "Graphics/CGraphicsLayer.h"
#include "Graphics/CGraphicsPoint.h"
#include "Graphics/CGraphicsEllipse.h"
#include "Graphics/CGraphicsRectangle.h"
#include "Graphics/CGraphicsPolygon.h"
#include "Graphics/CGraphicsPolyline.h"
#include "Graphics/CGraphicsText.h"

CImageScene::CImageScene (QObject* parent) : QGraphicsScene(parent)
{
    initSelectionBox();
}

CImageScene::~CImageScene()
{
    delete m_pSelectionBox;
}

QImage CImageScene::getImage() const
{
    return m_pPixmapItem->pixmap().toImage();
}

QGraphicsPixmapItem* CImageScene::getPixmapItem() const
{
    return m_pPixmapItem;
}

CGraphicsLayer* CImageScene::getCurrentGraphicsLayer() const
{
    return m_pCurrentLayer;
}

void CImageScene::setImage(const QImage& image)
{
    if(image.isNull())
        return;

    if(m_pPixmapItem)
        delete m_pPixmapItem;

    m_pPixmapItem = addPixmap(QPixmap::fromImage(image));
    m_pPixmapItem->setTransformationMode(Qt::SmoothTransformation);
    m_pPixmapItem->setZValue(-2);
}

void CImageScene::setOverlayImage(const QImage &image)
{
    if(image.isNull())
        return;

    if(m_pOverlayPixmapItem)
        m_pOverlayPixmapItem->setPixmap(QPixmap::fromImage(image));
    else
    {
        m_pOverlayPixmapItem = addPixmap(QPixmap::fromImage(image));
        m_pOverlayPixmapItem->setOpacity(0.7);
        m_pOverlayPixmapItem->setTransformationMode(Qt::FastTransformation);
        m_pOverlayPixmapItem->setZValue(-1);
    }
}

void CImageScene::setGraphicsContext(GraphicsContextPtr &graphicsContextPtr)
{
    m_graphicsContextPtr = graphicsContextPtr;
}

void CImageScene::setCurrentGraphicsLayer(CGraphicsLayer *pLayer)
{
    m_pCurrentLayer = pLayer;
}

void CImageScene::activateGraphics(bool bActivate)
{
    m_bGraphicsActivated = bActivate;
}

void CImageScene::addGraphicsLayer(CGraphicsLayer *pLayer, bool bTopMost)
{
    if(pLayer)
    {
        if(m_pCurrentLayer && bTopMost == false)
           pLayer->setParentItem(m_pCurrentLayer);
        else
            addItem(pLayer);
    }
}

void CImageScene::removePixmap()
{
    if(m_pPixmapItem != nullptr)
    {
        removeItem(m_pPixmapItem);
        delete m_pPixmapItem;
        m_pPixmapItem = nullptr;
    }
}

void CImageScene::removeGraphicsLayer(CGraphicsLayer *pLayer, bool bDelete)
{
    if(pLayer)
    {
        if(pLayer->isRoot() || pLayer == m_pCurrentLayer)
            m_pCurrentLayer = nullptr;

        removeItem(pLayer);

        if(bDelete == true)
            delete pLayer;
    }
}

void CImageScene::removeSelectedItems()
{
    auto itemList = this->selectedItems();
    if(itemList.size() > 0)
    {
        QSet<CGraphicsLayer*> parentLayers;
        for(auto it : itemList)
        {
            auto pParentLayer = dynamic_cast<CGraphicsLayer*>(it->parentItem());
            if(pParentLayer)
                parentLayers.insert(pParentLayer);

            removeItem(it);
            delete it;
        }
        notifyGraphicsRemoved(parentLayers);
    }
}

void CImageScene::clearOverlay()
{
    if(m_pOverlayPixmapItem)
    {
        removeItem(m_pOverlayPixmapItem);
        delete m_pOverlayPixmapItem;
        m_pOverlayPixmapItem = nullptr;
    }
}

void CImageScene::clearAll()
{
    clearOverlay();
    clear();
    m_pPixmapItem = nullptr;
    m_pCurrentLayer = nullptr;
    m_pTemporaryItem = nullptr;
    m_pSelectionBox = nullptr;
}

void CImageScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->buttons() & Qt::LeftButton)
    {
        if(m_bGraphicsActivated)
        {
            m_clickPtScene = mouseEvent->scenePos();
            m_clickPtScreen = mouseEvent->screenPos();
            mousePressGraphics(mouseEvent);
            return;
        }
    }
    else if(mouseEvent->buttons() & Qt::RightButton)
        removeTempItem();

    QGraphicsScene::mousePressEvent(mouseEvent);
}

void CImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton && m_bGraphicsActivated)
    {
        mouseReleaseGraphics(mouseEvent);
        return;
    }
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void CImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(m_bGraphicsActivated && (mouseEvent->buttons() & Qt::LeftButton || (mouseEvent->buttons() == Qt::NoButton && m_pTemporaryItem)))
    {
        mouseMoveGraphics(mouseEvent);
        return;
    }
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void CImageScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(m_bGraphicsActivated && mouseEvent->buttons() & Qt::LeftButton)
        mouseDoubleClickGraphics(mouseEvent);
}

void CImageScene::keyPressEvent(QKeyEvent *event)
{
    if(event->key()== Qt::Key_Delete)
        removeSelectedItems();
    else
        QGraphicsScene::keyPressEvent(event);
}

void CImageScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    // Just for cancelling QGraphicsScene behavior on drop (without overloading, we can't drop in scene)
    event->ignore();
}

GraphicsShape CImageScene::getGraphicsTool() const
{
    if(m_graphicsContextPtr)
        return m_graphicsContextPtr->getTool();
    else
        return GraphicsShape::SELECTION;
}

bool CImageScene::isGraphicsItemAt(QPointF pt)
{
    auto pItem = itemAt(pt, QTransform());
    auto pImgItem = dynamic_cast<QGraphicsPixmapItem*>(pItem);
    return (pItem != nullptr && pImgItem == nullptr);
}

void CImageScene::initSelectionBox()
{
    m_pSelectionBox = new QRubberBand(QRubberBand::Rectangle);
    m_pSelectionBox->hide();
}

void CImageScene::mousePressGraphics(QGraphicsSceneMouseEvent *pMouseEvent)
{
    switch(getGraphicsTool())
    {
        case GraphicsShape::SELECTION:
            m_bSelectObject = isGraphicsItemAt(m_clickPtScene);
            updateSelectionBox(m_clickPtScreen);
            QGraphicsScene::mousePressEvent(pMouseEvent);
            break;

        case GraphicsShape::POINT:
            createPointItem();
            break;

        case GraphicsShape::RECTANGLE:
            createTempRectItem();
            break;

        case GraphicsShape::ELLIPSE:
            createTempEllipsetItem();
            break;

        case GraphicsShape::POLYGON:
        case GraphicsShape::FREEHAND_POLYGON:
            addPtToPolygonItem();
            break;

        case GraphicsShape::LINE:
        case GraphicsShape::POLYLINE:
        case GraphicsShape::FREEHAND_POLYLINE:
            addPtToPolylineItem();
            break;

        case GraphicsShape::TEXT:
            createTextItem();
            break;

        default:
            QGraphicsScene::mousePressEvent(pMouseEvent);
            break;
    }
}

void CImageScene::mouseReleaseGraphics(QGraphicsSceneMouseEvent *pMouseEvent)
{
    switch(getGraphicsTool())
    {
        case GraphicsShape::SELECTION:
            QGraphicsScene::mouseReleaseEvent(pMouseEvent);
            validateSelection();

            if(m_bMoveObject == true)
                notifyGraphicsChanged();
            break;

        case GraphicsShape::POINT:
        case GraphicsShape::ELLIPSE:
        case GraphicsShape::RECTANGLE:
        case GraphicsShape::FREEHAND_POLYGON:
        case GraphicsShape::FREEHAND_POLYLINE:
        case GraphicsShape::TEXT:
            validateItem();
            break;

        case GraphicsShape::LINE:
            checkLineItem();
            break;

        case GraphicsShape::POLYGON:
        case GraphicsShape::POLYLINE:
            break;
    }
    m_bMoveObject = false;
}

void CImageScene::mouseMoveGraphics(QGraphicsSceneMouseEvent *pMouseEvent)
{
    if(pMouseEvent->buttons() & Qt::LeftButton)
    {
        switch(getGraphicsTool())
        {
            case GraphicsShape::SELECTION:
                if(m_bSelectObject == false)
                    updateSelectionBox(pMouseEvent->screenPos());
                else
                {
                    m_bMoveObject = true;
                    QGraphicsScene::mouseMoveEvent(pMouseEvent);
                }
                break;

            case GraphicsShape::RECTANGLE:
                updateTempRectItem(pMouseEvent->scenePos(), pMouseEvent->modifiers());
                break;

            case GraphicsShape::ELLIPSE:
                updateTempEllipseItem(pMouseEvent->scenePos(), pMouseEvent->modifiers());
                break;

            case GraphicsShape::FREEHAND_POLYGON:
                m_clickPtScene = pMouseEvent->scenePos();
                addPtToPolygonItem();
                break;

            case GraphicsShape::FREEHAND_POLYLINE:
                m_clickPtScene = pMouseEvent->scenePos();
                addPtToPolylineItem();
                break;

            case GraphicsShape::POINT:
            case GraphicsShape::POLYGON:
            case GraphicsShape::LINE:
            case GraphicsShape::POLYLINE:
            case GraphicsShape::TEXT:
                QGraphicsScene::mouseMoveEvent(pMouseEvent);
                break;
        }
    }
    else if(pMouseEvent->buttons() == Qt::NoButton && m_pTemporaryItem)
    {
        switch(getGraphicsTool())
        {
            case GraphicsShape::POLYGON:
                updateTempPolygonItem(pMouseEvent->scenePos());
                break;

            case GraphicsShape::LINE:
            case GraphicsShape::POLYLINE:
                updateTempPolylineItem(pMouseEvent->scenePos());
                break;

            case GraphicsShape::SELECTION:
            case GraphicsShape::POINT:
            case GraphicsShape::ELLIPSE:
            case GraphicsShape::RECTANGLE:
            case GraphicsShape::FREEHAND_POLYGON:
            case GraphicsShape::FREEHAND_POLYLINE:
            case GraphicsShape::TEXT:
                QGraphicsScene::mouseMoveEvent(pMouseEvent);
                break;
        }
    }
}

void CImageScene::mouseDoubleClickGraphics(QGraphicsSceneMouseEvent *pMouseEvent)
{
    Q_UNUSED(pMouseEvent);

    auto tool = getGraphicsTool();
    if(tool == GraphicsShape::POLYGON || tool == GraphicsShape::POLYLINE)
    {
        validateItem();
        pMouseEvent->accept();
    }
}

void CImageScene::createPointItem()
{
    assert(m_graphicsContextPtr);
    checkCurrentLayer();
    auto pProperty = m_graphicsContextPtr->getPointPropertyPtr();
    auto pPointItem = new CGraphicsPoint(m_clickPtScene, *pProperty, m_pCurrentLayer);
    Q_UNUSED(pPointItem);
}

void CImageScene::createTempRectItem()
{
    assert(m_graphicsContextPtr);
    auto pProperty = m_graphicsContextPtr->getRectPropertyPtr();
    m_pTemporaryItem = new CGraphicsRectangle(*pProperty, m_pCurrentLayer);

    if(m_pCurrentLayer == nullptr)
        addItem(m_pTemporaryItem);
}

void CImageScene::createTempEllipsetItem()
{
    assert(m_graphicsContextPtr);
    auto pProperty = m_graphicsContextPtr->getEllipsePropertyPtr();
    m_pTemporaryItem = new CGraphicsEllipse(*pProperty, m_pCurrentLayer);

    if(m_pCurrentLayer == nullptr)
        addItem(m_pTemporaryItem);
}

void CImageScene::createTextItem()
{
    assert(m_graphicsContextPtr);
    checkCurrentLayer();
    auto pProperty = m_graphicsContextPtr->getTextPropertyPtr();
    CGraphicsText* pTextItem = new CGraphicsText("Text Area", *pProperty, m_clickPtScene, m_pCurrentLayer);
    Q_UNUSED(pTextItem);
}

void CImageScene::createRootLayer()
{
    //Create root layer
    m_pCurrentLayer = new CGraphicsLayer(tr("Graphics"));
    m_pCurrentLayer->setRoot(true);
    addItem(m_pCurrentLayer);
    static_cast<CImageView*>(views().first())->addLayerToModel(m_pCurrentLayer);
}

void CImageScene::createInputLayer()
{
    m_pCurrentLayer = new CGraphicsLayer(tr("Input layer"), m_pCurrentLayer);
    static_cast<CImageView*>(views().first())->addLayerToModel(m_pCurrentLayer);
}

void CImageScene::updateSelectionBox(QPointF currentPt)
{
    int x = std::min(m_clickPtScreen.x(), currentPt.x());
    int y = std::min(m_clickPtScreen.y(), currentPt.y());
    int width = qFabs(m_clickPtScreen.x() - currentPt.x());
    int height = qFabs(m_clickPtScreen.y() - currentPt.y());
    m_pSelectionBox->setGeometry(x, y, width, height);
    m_pSelectionBox->show();
}

void CImageScene::updateTempRectItem(QPointF currentPt, Qt::KeyboardModifiers keyModifiers)
{
    auto pRectItem = dynamic_cast<CGraphicsRectangle*>(m_pTemporaryItem);
    if(pRectItem)
    {
        qreal x = std::min(m_clickPtScene.x(), currentPt.x());
        qreal y = std::min(m_clickPtScene.y(), currentPt.y());

        if(keyModifiers == Qt::ControlModifier)
        {
            //Square
            qreal width = std::min(qFabs(m_clickPtScene.x() - currentPt.x()), qFabs(m_clickPtScene.y() - currentPt.y()));
            pRectItem->setRect(x, y, width, width);
        }
        else
        {
            //Rectangle
            qreal width = qFabs(m_clickPtScene.x() - currentPt.x());
            qreal height = qFabs(m_clickPtScene.y() - currentPt.y());
            pRectItem->setRect(x, y, width, height);
        }
    }
}

void CImageScene::updateTempEllipseItem(QPointF currentPt, Qt::KeyboardModifiers keyModifiers)
{
    CGraphicsEllipse* pItem = dynamic_cast<CGraphicsEllipse*>(m_pTemporaryItem);
    if(pItem)
    {
        qreal x = std::min(m_clickPtScene.x(), currentPt.x());
        qreal y = std::min(m_clickPtScene.y(), currentPt.y());

        if(keyModifiers == Qt::ControlModifier)
        {
            //Circle
            qreal width = std::min(qFabs(m_clickPtScene.x() - currentPt.x()), qFabs(m_clickPtScene.y() - currentPt.y()));
            pItem->setRect(x, y, width, width);
        }
        else
        {
            //Ellipse
            qreal width = qFabs(m_clickPtScene.x() - currentPt.x());
            qreal height = qFabs(m_clickPtScene.y() - currentPt.y());
            pItem->setRect(x, y, width, height);
        }
    }
}

void CImageScene::updateTempPolygonItem(QPointF currentPt)
{
    CGraphicsPolygon* pItem = dynamic_cast<CGraphicsPolygon*>(m_pTemporaryItem);
    if(pItem)
        pItem->updateLastPoint(currentPt);
}

void CImageScene::updateTempPolylineItem(QPointF currentPt)
{
    CGraphicsPolyline* pItem = dynamic_cast<CGraphicsPolyline*>(m_pTemporaryItem);
    if(pItem)
        pItem->updateLastPoint(currentPt);
}

void CImageScene::addPtToPolygonItem()
{
    if(m_pTemporaryItem == Q_NULLPTR)
    {
        assert(m_graphicsContextPtr);
        auto pProperty = m_graphicsContextPtr->getPolygonPropertyPtr();
        m_pTemporaryItem = new CGraphicsPolygon(*pProperty, m_pCurrentLayer);

        if(m_pCurrentLayer == nullptr)
            addItem(m_pTemporaryItem);
    }

    CGraphicsPolygon* pItem = dynamic_cast<CGraphicsPolygon*>(m_pTemporaryItem);
    if(pItem)
        pItem->addPoint(m_clickPtScene);
}

void CImageScene::addPtToPolylineItem()
{
    if(m_pTemporaryItem == Q_NULLPTR)
    {
        assert(m_graphicsContextPtr);
        auto pProperty = m_graphicsContextPtr->getPolylinePropertyPtr();
        m_pTemporaryItem = new CGraphicsPolyline(*pProperty, m_pCurrentLayer);

        if(m_pCurrentLayer == nullptr)
            addItem(m_pTemporaryItem);
    }

    CGraphicsPolyline* pItem = dynamic_cast<CGraphicsPolyline*>(m_pTemporaryItem);
    if(pItem)
        pItem->addPoint(m_clickPtScene);
}

void CImageScene::checkLineItem()
{
    if(m_pTemporaryItem != Q_NULLPTR)
    {
        CGraphicsPolyline* pItem = dynamic_cast<CGraphicsPolyline*>(m_pTemporaryItem);
        if(pItem && pItem->getPolygon().size() > 2)
            validateItem();
    }
}

void CImageScene::checkCurrentLayer()
{
    if(m_pCurrentLayer == nullptr)
    {
        createRootLayer();
        createInputLayer();
    }
    else if(m_pCurrentLayer->isRoot())
        createInputLayer();
}

void CImageScene::validateSelection()
{
    int maxGeometricParent = 0;
    QPainterPath selectionPath;
    QGraphicsView* pView = views().first();
    QRect rc = m_pSelectionBox->geometry();

    if(rc.width() == 0 && rc.height() == 0)
    {
        int nbCollidingItems;
        QGraphicsItem* pItemToSelect = nullptr;
        clearSelection();
        auto pItemList = items(m_clickPtScene);

        for(auto it : pItemList)
        {
            if((nbCollidingItems = it->collidingItems(Qt::ContainsItemShape).size()) >= maxGeometricParent)
            {
                pItemToSelect = it;
                maxGeometricParent = nbCollidingItems;
            }
        }

        if(pItemToSelect)
            pItemToSelect->setSelected(true);
    }
    else
    {
        QPoint p1 = pView->mapFromGlobal(QPoint(rc.topLeft()));
        QPoint p2 = pView->mapFromGlobal(QPoint(rc.bottomRight()));
        QPointF topLeft = pView->mapToScene(p1);
        QPointF bottomRight = pView->mapToScene(p2);
        selectionPath.addRect(QRectF(topLeft, bottomRight));
        setSelectionArea(selectionPath, Qt::ReplaceSelection, Qt::ContainsItemShape);
    }
    m_pSelectionBox->hide();
}

void CImageScene::validateItem()
{
    if(m_pTemporaryItem)
    {
        checkCurrentLayer();
        m_pTemporaryItem->setParentItem(m_pCurrentLayer);
        m_pTemporaryItem = nullptr;
    }
    notifyGraphicsChanged();
}

void CImageScene::removeTempItem()
{
    if(m_pTemporaryItem)
    {
        removeItem(m_pTemporaryItem);
        delete m_pTemporaryItem;
        m_pTemporaryItem = Q_NULLPTR;
    }
}

void CImageScene::notifyGraphicsChanged() const
{
    auto pView = static_cast<CImageView*>(views().first());
    if(pView)
        pView->notifyGraphicsChanged();
}

void CImageScene::notifyGraphicsRemoved(const QSet<CGraphicsLayer*>& layers) const
{
    auto pView = static_cast<CImageView*>(views().first());
    if(pView)
        pView->notifyGraphicsRemoved(layers);
}

#include "moc_CImageScene.cpp"
