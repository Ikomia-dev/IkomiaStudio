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

#ifndef CIMAGESCENE_H
#define CIMAGESCENE_H

/**
 * @file      CImageScene.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CImageScene definition
 *
 * @details   Details
 */

#include <memory>
#include <QGraphicsScene>
#include "Main/AppDefine.hpp"
#include "Main/forwards.hpp"
#include "Graphics/CGraphicsContext.h"

class QAbstractGraphicsShapeItem;
class CGraphicsLayer;

/**
 * @brief
 *
 */
class CImageScene : public QGraphicsScene
{
    Q_OBJECT

    public:

        CImageScene(QObject* parent = 0);
        ~CImageScene();


        QImage                  getImage() const;
        QGraphicsPixmapItem*    getPixmapItem() const;
        CGraphicsLayer*         getCurrentGraphicsLayer() const;

        void                    setImage(const QImage& image);
        void                    setOverlayImage(const QImage& image);
        void                    setGraphicsContext(GraphicsContextPtr &graphicsContextPtr);
        void                    setCurrentGraphicsLayer(CGraphicsLayer* pLayer);

        void                    activateGraphics(bool bActivate);

        void                    addGraphicsLayer(CGraphicsLayer* pLayer, bool bTopMost);

        void                    removePixmap();
        void                    removeGraphicsLayer(CGraphicsLayer* pLayer, bool bDelete);
        void                    removeSelectedItems();

        void                    clearOverlay();
        void                    clearAll();

    protected:

        virtual void            mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void            mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void            mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        virtual void            mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

        virtual void            keyPressEvent(QKeyEvent * event) override;

        virtual void            dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

    private:

        GraphicsShape           getGraphicsTool() const;

        bool                    isGraphicsItemAt(QPointF pt);

        void                    initSelectionBox();

        void                    mousePressGraphics(QGraphicsSceneMouseEvent* pMouseEvent);
        void                    mouseReleaseGraphics(QGraphicsSceneMouseEvent* pMouseEvent);
        void                    mouseMoveGraphics(QGraphicsSceneMouseEvent* pMouseEvent);
        void                    mouseDoubleClickGraphics(QGraphicsSceneMouseEvent* pMouseEvent);

        void                    createPointItem();
        void                    createTempRectItem();
        void                    createTempEllipsetItem();
        void                    createTextItem();
        void                    createRootLayer();
        void                    createInputLayer();

        void                    updateSelectionBox(QPointF currentPt);
        void                    updateTempRectItem(QPointF currentPt, Qt::KeyboardModifiers keyModifiers);
        void                    updateTempEllipseItem(QPointF currentPt, Qt::KeyboardModifiers keyModifiers);
        void                    updateTempPolygonItem(QPointF currentPt);
        void                    updateTempPolylineItem(QPointF currentPt);

        void                    addPtToPolygonItem();
        void                    addPtToPolylineItem();

        void                    checkLineItem();
        void                    checkCurrentLayer();

        void                    validateSelection();
        void                    validateItem();

        void                    removeTempItem();

        void                    notifyGraphicsChanged() const;
        void                    notifyGraphicsRemoved(const QSet<CGraphicsLayer *> &layers) const;

    protected:

        //Image
        QGraphicsPixmapItem*        m_pPixmapItem = nullptr;
        QGraphicsPixmapItem*        m_pOverlayPixmapItem = nullptr;

        //Graphics
        GraphicsContextPtr          m_graphicsContextPtr = nullptr;
        CGraphicsLayer*             m_pCurrentLayer = nullptr;
        QAbstractGraphicsShapeItem* m_pTemporaryItem = nullptr;
        QRubberBand*                m_pSelectionBox = nullptr;
        QPointF                     m_clickPtScene;
        QPointF                     m_clickPtScreen;
        bool                        m_bGraphicsActivated = false;
        bool                        m_bSelectObject = false;
        bool                        m_bMoveObject = false;
};

#endif // CIMAGESCENE_H
