/**
 * @file      CImageView.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CImageView
 *
 * @details   Details
 */

#include "CImageView.h"
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QTimeLine>
#include "Graphics/CGraphicsLayer.h"
#include <chrono>
#include <cmath>

CImageView::CImageView (QWidget* parent) : QGraphicsView(parent)
{
    setAcceptDrops(true);
    setCursor(Qt::OpenHandCursor);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setInteractive(true);
    setMouseTracking(true);
    m_pScene = new CImageScene;
    setScene(m_pScene);

    m_pTimeLine = new QTimeLine(350, this);
    m_pTimeLine->setUpdateInterval(20);

    initConnections();
}

CImageView::~CImageView()
{
    if(m_bInternalScene)
    {
        scene()->clear();
        scene()->deleteLater();
    }
}

float CImageView::getScale()
{
    return transform().m11();
}

QImage CImageView::getImage()
{
    assert(m_pScene);
    return m_pScene->getImage();
}

CImageScene *CImageView::getScene()
{
    return m_pScene;
}

int CImageView::getNumScheduledScalings()
{
    return m_numScheduledScalings;
}

QPointF CImageView::getTargetScenePos()
{
    return m_target_scene_pos;
}

QPointF CImageView::getCenter()
{
    return m_center_pos;
}

bool CImageView::isZoomFit() const
{
    return m_bZoomFit;
}

void CImageView::setScale(float scaleFactor)
{
    m_bZoomFit = false;
    scale(scaleFactor/transform().m11(), scaleFactor/transform().m11());
}

void CImageView::setImageScene(CImageScene* pScene)
{
    if(m_pScene)
    {
        if(m_bInternalScene)
        {
            delete m_pScene;
            m_pScene = nullptr;
        }
        else
            m_pScene->removePixmap();
    }

    m_pScene = pScene;
    setScene(m_pScene);
    m_bInternalScene = false;
    m_pScene->activateGraphics(m_bGraphicsActivated);
}

void CImageView::setImage(const QImage &image, bool bZoomFit)
{
    if(image.isNull())
        throw CException(CoreExCode::INVALID_IMAGE, "Invalid image", __func__, __FILE__, __LINE__);

    if(m_pScene == nullptr)
        throw CException(CoreExCode::NULL_POINTER, "Invalid image scene object", __func__, __FILE__, __LINE__);

    m_pScene->setImage(image);
    QRectF rect = sceneRect();
    rect.setBottomRight(QPointF(image.width(), image.height()));
    setSceneRect(rect);
    viewport()->update();

    if(bZoomFit == true)
        zoomFit();
}

void CImageView::setCurrentGraphicsLayer(CGraphicsLayer *pLayer)
{
    if(m_pScene)
        m_pScene->setCurrentGraphicsLayer(pLayer);
}

void CImageView::setNumScheduledScalings(int numScheduledScalings)
{
    m_numScheduledScalings = numScheduledScalings;
}

void CImageView::setTargetScenePos(const QPointF& pos)
{
    m_target_scene_pos = pos;
}

void CImageView::addLayerToModel(CGraphicsLayer *pLayer)
{
    if(pLayer)
        emit doAddGraphicsLayer(pLayer);
}

void CImageView::addGraphicsItem(QGraphicsItem *pItem, bool bForceParent)
{
    assert(m_pScene);
    if(pItem)
    {
        if(bForceParent)
            pItem->setParentItem(m_pScene->getCurrentGraphicsLayer());

        m_pScene->addItem(pItem);
    }
}

void CImageView::addGraphicsLayer(CGraphicsLayer *pLayer, bool bTopMost)
{
    if(m_pScene)
        m_pScene->addGraphicsLayer(pLayer, bTopMost);
}

void CImageView::removeGraphicsLayer(CGraphicsLayer *pLayer, bool bDelete)
{
    if(m_pScene)
        m_pScene->removeGraphicsLayer(pLayer, bDelete);
}

void CImageView::notifyGraphicsChanged()
{
    emit doGraphicsChanged();
}

void CImageView::notifyGraphicsRemoved(const QSet<CGraphicsLayer*>& layers)
{
    emit doGraphicsRemoved(layers);
}

void CImageView::notifySceneDeleted()
{
    m_pScene = new CImageScene;
    setScene(m_pScene);
    m_bInternalScene = true;
}

void CImageView::setOverlayImage(const QImage& image)
{
    if(m_pScene)
        m_pScene->setOverlayImage(image);
}

void CImageView::clearOverlay()
{
    if(m_pScene)
        m_pScene->clearOverlay();
}

void CImageView::clear()
{
    if(m_pScene)
        m_pScene->clearAll();
}

void CImageView::updateCenter()
{
    m_center_pos = mapToScene(viewport()->rect().center());
    emit doUpdateCenter(m_center_pos);
}

void CImageView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
}

void CImageView::mousePressEvent(QMouseEvent* event)
{
    switch (event->buttons())
    {
        case Qt::MiddleButton:
            zoomFit();
            emit doZoomFit();
            break;

        case Qt::LeftButton:
            if(m_bGraphicsActivated == false)
                beginDrag(event);
            break;

        default:
            break;
    }
    QGraphicsView::mousePressEvent(event);
}

void CImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_bGraphicsActivated == false)
        setCursor(Qt::OpenHandCursor);

    QGraphicsView::mouseReleaseEvent(event);
}

void CImageView::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() == Qt::LeftButton && m_bGraphicsActivated == false)
        dragImage(event);

    QGraphicsView::mouseMoveEvent(event);
}

void CImageView::wheelEvent(QWheelEvent* event)
{
    //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    // Scale the view / do the zoom
    /*SMOOTH WAY*/
    smoothZoom(event);
    emit doZoomEvent(event);
}

void CImageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
}

void CImageView::showEvent(QShowEvent* event)
{
    QGraphicsView::showEvent(event);
}

void CImageView::keyPressEvent(QKeyEvent *event)
{
    if(event->key()== Qt::Key_Shift)
    {
        activateGraphics(!m_bGraphicsActivated);
        emit doChangeGraphicsActivationState(m_bGraphicsActivated);
    }
    else if(!m_bMoveByKey &&
            (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
             event->key() == Qt::Key_Left || event->key() == Qt::Key_Right))
    {
        event->ignore();
    }
    else
        QGraphicsView::keyPressEvent(event);
}

void CImageView::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsView::keyReleaseEvent(event);
}

void CImageView::dragEnterEvent(QDragEnterEvent* event)
{
    // Just for propagating event to parent
    event->ignore();
}

void CImageView::drawBackground(QPainter *painter, const QRectF &r)
{
    QGraphicsView::drawBackground(painter, r);

    auto pPixmapItem = m_pScene->getPixmapItem();
    if(pPixmapItem == nullptr)
        return;

    if(pPixmapItem->pixmap().hasAlphaChannel() == false)
        return;

    //Draw transparency pattern (alternance of two-colors squares)
    const QRectF rcImage = pPixmapItem->boundingRect();
    const QRectF rcImageVisible = rcImage.intersected(r);

    const QColor color1(80, 80, 80);
    const QColor color2(110, 110, 110);

    //Scale invariance
    qreal size = std::floor((m_bckGridSize / transform().m11()) + 0.5);
    if(size < 1)
        size = 1;

    bool bColor1 = true;
    bool bEvenRow = true;

    for(qreal i=rcImageVisible.top(); i<rcImageVisible.bottom(); i+=size)
    {
        for(qreal j=rcImageVisible.left(); j<rcImageVisible.right(); j+=size)
        {
            QColor color;
            bColor1 ? color = color1 : color = color2;
            drawSquare(painter, j, i, size, color);
            bColor1 = !bColor1;
        }
        bEvenRow = !bEvenRow;
        bColor1 = bEvenRow;
    }
}

void CImageView::initConnections()
{
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &CImageView::onScrollChanged);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CImageView::onScrollChanged);

    connect(m_pTimeLine, &QTimeLine::valueChanged, this, &CImageView::onScalingTime);
    connect(m_pTimeLine, &QTimeLine::finished, this, &CImageView::onAnimFinished);
}

void CImageView::onScalingTime()
{
     qreal factor = 1.0+ qreal(m_numScheduledScalings) / 300.0;
     scale(factor, factor);

     // Keep zoom under mouse using projection in newly zoomed scene and calculation of translation
     QPointF newScenePos = mapToScene(m_target_viewport_pos);
     QPointF delta = newScenePos - m_target_scene_pos;
     translate(delta.x(), delta.y());
}

void CImageView::onAnimFinished()
{
}

void CImageView::smoothZoom(QWheelEvent* event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15; // see QWheelEvent documentation
    m_numScheduledScalings += numSteps;
    m_bZoomFit = false;

    if (m_numScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
       m_numScheduledScalings = numSteps;

    if(m_pTimeLine->state() == QTimeLine::Running)
        return;

    m_pTimeLine->start();
}

void CImageView::zoom(float scaleFactor)
{
    if(m_pScene == nullptr)
        return;

    m_bZoomFit = false;
    scale(scaleFactor, scaleFactor);
    m_numScheduledScalings = 0;
}

void CImageView::onZoomIn()
{
    zoom(1 + m_zoomDelta);
}

void CImageView::onZoomOut()
{
    zoom(1 - m_zoomDelta);
}

void CImageView::onZoomOriginal()
{
    zoomOriginal();
}

void CImageView::onZoomOriginalEvent()
{
    zoomOriginal();
    emit doZoomOriginal();
}

void CImageView::onUpdateTargetPos(const QPointF& scenePos)
{
    // Update target position using only position in scene because, viewport can
    // be different from src image and result image (when multiple video display)
    m_target_viewport_pos = mapFromScene(scenePos);
    m_target_scene_pos = scenePos;
}

void CImageView::zoomFit()
{
    assert(m_pScene != nullptr);

    if(m_pScene->getPixmapItem() == nullptr)
        return;

    fitInView(m_pScene->getPixmapItem()->boundingRect(), Qt::KeepAspectRatio);
    m_numScheduledScalings = 0;
    m_bZoomFit = true;
}

void CImageView::zoomOriginal()
{
    resetMatrix();
    m_numScheduledScalings = 0;
    m_bZoomFit = false;
}

void CImageView::beginDrag(QMouseEvent *pEvent)
{
    m_firstDragPt = pEvent->pos();
    setCursor(Qt::ClosedHandCursor);
}

void CImageView::dragImage(QMouseEvent* pEvent)
{
    QPointF firstPt = mapToScene(m_firstDragPt);
    QPointF lastPt = mapToScene(pEvent->pos());
    QPointF translation = lastPt - firstPt;
    translate(translation.x(), translation.y());
    m_firstDragPt = pEvent->pos();
    updateCenter();

    m_target_viewport_pos = pEvent->pos();
    m_target_scene_pos = lastPt;
    emit doUpdateTargetPos(m_target_scene_pos);
}

void CImageView::drawSquare(QPainter* painter, qreal x, qreal y, qreal size, const QColor &color)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));
    painter->drawRect(x, y, size, size);
}

void CImageView::activateGraphics(bool bActivate)
{
    m_bGraphicsActivated = bActivate;
    m_pScene->activateGraphics(bActivate);
    m_bGraphicsActivated ? setCursor(Qt::ArrowCursor) : setCursor(Qt::OpenHandCursor);
}

void CImageView::enableMoveByKey(bool bEnable)
{
    m_bMoveByKey = bEnable;
}

void CImageView::onDispatchZoomIn()
{
    onZoomIn();
    emit doZoomIn();
}

void CImageView::onDispatchZoomOut()
{
    onZoomOut();
    emit doZoomOut();
}

void CImageView::onUpdateZoom(QWheelEvent *event)
{
    //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    smoothZoom(event);
}

void CImageView::onZoomFit()
{
    zoomFit();
}

void CImageView::onZoomFitEvent()
{
    zoomFit();
    emit doZoomFit();
}

void CImageView::onUpdateCenter(const QPointF& center)
{
    blockSignals(true);
    centerOn(center);
    m_center_pos = center;
    blockSignals(false);
}

void CImageView::onScrollChanged()
{
    updateCenter();
}

#include "moc_CImageView.cpp"
