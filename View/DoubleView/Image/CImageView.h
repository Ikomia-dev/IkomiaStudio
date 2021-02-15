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

#ifndef CIMAGEVIEW_H
#define CIMAGEVIEW_H

/**
 * @file      CImageView.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CImageView definition
 *
 * @details   Details
 */

#include <QGraphicsView>
#include <memory>
#include "CImageScene.h"

class QTimeLine;

class CImageView : public QGraphicsView
{
    Q_OBJECT

    public:

        CImageView(QWidget* parent = 0);
        ~CImageView();

        float               getScale();
        QImage              getImage();
        CImageScene*        getScene();
        int                 getNumScheduledScalings();
        QPointF             getTargetScenePos();
        QPointF             getCenter();

        bool                isZoomFit() const;

        void                setScale(float scaleFactor);
        void                setImageScene(CImageScene *pScene);
        void                setImage(const QImage& image, bool bZoomFit = false);
        void                setCurrentGraphicsLayer(CGraphicsLayer* pLayer);
        void                setNumScheduledScalings(int numScheduledScalings);
        void                setTargetScenePos(const QPointF& pos);

        void                addLayerToModel(CGraphicsLayer* pLayer);
        void                addGraphicsItem(QGraphicsItem* pItem, bool bForceParent);
        void                addGraphicsLayer(CGraphicsLayer* pLayer, bool bTopMost);

        void                removeGraphicsLayer(CGraphicsLayer* pLayer, bool bDelete);

        void                activateGraphics(bool bActivate);

        void                enableMoveByKey(bool bEnable);

        void                notifyGraphicsChanged();
        void                notifyGraphicsRemoved(const QSet<CGraphicsLayer *> &layers);
        void                notifySceneDeleted();

        void                setOverlayImage(const QImage &image);

        void                clear();
        void                clearOverlay();

        void                updateCenter();

        void                zoomFit();

    signals:

        void                doZoomEvent(QWheelEvent* event);
        void                doZoomFit();
        void                doZoomIn();
        void                doZoomOut();
        void                doZoomOriginal();
        void                doUpdateCenter(const QPointF& center);
        void                doUpdateTargetPos(const QPointF& scenePos);
        //Graphics
        void                doChangeGraphicsActivationState(bool bActivated);
        void                doAddGraphicsLayer(CGraphicsLayer* pLayer);
        void                doGraphicsChanged();
        void                doGraphicsRemoved(const QSet<CGraphicsLayer*>& layers);

    public slots:

        void                onScalingTime();
        void                onAnimFinished();
        void                onUpdateZoom(QWheelEvent* event);
        void                onDispatchZoomIn();
        void                onDispatchZoomOut();
        void                onZoomFit();
        void                onZoomFitEvent();
        void                onUpdateCenter(const QPointF& center);
        void                onScrollChanged();
        void                onZoomIn();
        void                onZoomOut();
        void                onZoomOriginal();
        void                onZoomOriginalEvent();
        void                onUpdateTargetPos(const QPointF& scenePos);

    protected:

        virtual void        mouseMoveEvent(QMouseEvent *event);
        virtual void        mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;       
        virtual void        mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        virtual void        mouseReleaseEvent(QMouseEvent* event) override;

        virtual void        wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

        virtual void        resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

        virtual void        showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

        virtual void        keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;        
        virtual void        keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

        virtual void        dragEnterEvent(QDragEnterEvent* event) override;

        virtual void        drawBackground(QPainter* painter, const QRectF& r) override;

    private:

        void                initConnections();

        void                smoothZoom(QWheelEvent* event);
        void                zoom(float scaleFactor);
        void                zoomOriginal();

        void                beginDrag(QMouseEvent* pEvent);
        void                dragImage(QMouseEvent *pEvent);

        void                drawSquare(QPainter *painter, qreal x, qreal y, qreal size, const QColor& color);

    private:

        CImageScene*        m_pScene = nullptr;
        QTimeLine*          m_pTimeLine = nullptr;
        QPointF             m_center_pos;
        QPointF             m_target_scene_pos;
        QPoint              m_target_viewport_pos;
        QPoint              m_firstDragPt;
        bool                m_bInternalScene = true;
        bool                m_bGraphicsActivated = false;
        bool                m_bZoomFit = false;
        bool                m_bMoveByKey = true;
        float               m_zoomDelta = 0.15f;
        int                 m_numScheduledScalings = 0;
        const int           m_bckGridSize = 10;
};

#endif // CIMAGEVIEW_H
