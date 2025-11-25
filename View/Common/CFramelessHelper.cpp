/**
 * @file      CFramelessHelper.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CFramelessHelper
 *
 * @details   Details
 */

/*
    FramelessHelper, an easy way to support move/resize on
    frameless toplevel windows.
    Copyright (C) 2011  Nishant Parashar
    Email:- nishcode (at) gmail (dot) com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QRubberBand>
#include <QMouseEvent>
#include <QMutex>
#include <QDebug>
#include "CFramelessHelper.h"

class CCursorPosCalculator
{
    public:

        CCursorPosCalculator();
        void reset();
        void recalculate( const QPoint& globalMousePos, const QRect& frameRect );

    public:

        bool    m_bOnEdges;
        bool    m_bOnLeftEdge;
        bool    m_bOnRightEdge;
        bool    m_bOnTopEdge;
        bool    m_bOnBottomEdge;
        bool    m_bOnTopLeftEdge;
        bool    m_bOnBottomLeftEdge;
        bool    m_bOnTopRightEdge;
        bool    m_OnBottomRightEdge;

        bool    m_bOnBorders;
        bool    m_bOnLeftBorder;
        bool    m_bOnRightBorder;
        bool    m_bOnTopBorder;
        bool    m_bOnBottomBorder;

        int     m_edgeWidth=1;
        int     m_borderWidth=15;
};

class CWidgetData
{
    public:

        CWidgetData( CFramelessHelperImpl* _d, QWidget* topLevelWidget );
        ~CWidgetData();

        //void setWidget( QWidget* topLevelWidget );
        QWidget*    widget();
        void        handleWidgetEvent( QEvent* event );
        void        updateRubberBandStatus();
        void        setBorderWidth(int width);
        void        setEdgeWidth(int width);
        void        setWidgetGeometry(QWidget* pWidget);

    private:

        void        updateCursorShape( const QPoint& globalMousePos );
        void        resizeWidget( const QPoint& globalMousePos );
        void        moveWidget( const QPoint& globalMousePos );

        void        handleMousePressEvent( QMouseEvent* event );
        void        handleMouseReleaseEvent( QMouseEvent* event );
        void        handleMouseMoveEvent( QMouseEvent* event );
        void        handleLeaveEvent( QEvent* event );
        void        handleHoverMoveEvent( QHoverEvent* event );

    private:

        CFramelessHelperImpl*   m_pHelperImpl = nullptr;
        QRubberBand*            m_pRubberBand = nullptr;
        QWidget*                m_pWidget = nullptr;
        QWidget*                m_pWidgetGeometry = nullptr;
        QPoint                  m_dragPos;
        CCursorPosCalculator    m_pressedMousePos;
        CCursorPosCalculator    m_moveMousePos;
        bool                    m_bLeftButtonPressed;
        bool                    m_bCursorShapeChanged;
        Qt::WindowFlags         m_windowFlags;

};

class CFramelessHelperImpl
{
    public:

        QHash< QWidget*, CWidgetData* > m_widgetDataHash;
        bool m_bWidgetMovable;
        bool m_bWidgetResizable;
        bool m_bUseRubberBandOnResize;
        bool m_bUseRubberBandOnMove;
};

CCursorPosCalculator::CCursorPosCalculator()
{
    reset();
}

void CCursorPosCalculator::reset()
{
    m_bOnEdges = false;
    m_bOnLeftEdge = false;
    m_bOnRightEdge = false;
    m_bOnTopEdge = false;
    m_bOnBottomEdge = false;
    m_bOnTopLeftEdge = false;
    m_bOnBottomLeftEdge = false;
    m_bOnTopRightEdge = false;
    m_OnBottomRightEdge = false;
}

void CCursorPosCalculator::recalculate( const QPoint& globalMousePos, const QRect& frameRect )
{
    int globalMouseX = globalMousePos.x();
    int globalMouseY = globalMousePos.y();

    int frameX = frameRect.x();
    int frameY = frameRect.y();

    int frameWidth = frameRect.width();
    int frameHeight = frameRect.height();

    m_bOnLeftEdge = globalMouseX >= frameX && globalMouseX <= frameX + m_edgeWidth;
    m_bOnRightEdge = globalMouseX >= frameX + frameWidth - m_edgeWidth && globalMouseX <= frameX + frameWidth;
    m_bOnTopEdge = globalMouseY >= frameY && globalMouseY <= frameY + m_edgeWidth;
    m_bOnBottomEdge = globalMouseY >= frameY + frameHeight - m_edgeWidth && globalMouseY <= frameY + frameHeight;
    m_bOnTopLeftEdge = m_bOnTopEdge && m_bOnLeftEdge;
    m_bOnBottomLeftEdge = m_bOnBottomEdge && m_bOnLeftEdge;
    m_bOnTopRightEdge = m_bOnTopEdge && m_bOnRightEdge;
    m_OnBottomRightEdge = m_bOnBottomEdge && m_bOnRightEdge;
    //only these checks would be enough
    m_bOnEdges = m_bOnLeftEdge || m_bOnRightEdge || m_bOnTopEdge || m_bOnBottomEdge;

    m_bOnLeftBorder = globalMouseX >= frameX + m_edgeWidth && globalMouseX <= frameX + m_borderWidth;
    m_bOnRightBorder = globalMouseX >= frameX + frameWidth - m_borderWidth && globalMouseX <= frameX + frameWidth - m_edgeWidth;
    m_bOnTopBorder = globalMouseY >= frameY + m_edgeWidth && globalMouseY <= frameY + m_borderWidth;
    m_bOnBottomBorder = globalMouseY >= frameY + frameHeight - m_borderWidth && globalMouseY <= frameY + frameHeight - m_edgeWidth;
    //m_bOnBorders = m_bOnLeftBorder || m_bOnRightBorder || m_bOnTopBorder || m_bOnBottomBorder;
    m_bOnBorders = m_bOnTopBorder;
}

/* WIGDETDATA*/

CWidgetData::CWidgetData( CFramelessHelperImpl* _d, QWidget* topLevelWidget )
{
    m_pHelperImpl = _d;
    m_pWidget = topLevelWidget;
    m_bLeftButtonPressed = false;
    m_pRubberBand = 0;
    m_bCursorShapeChanged = false;

    m_windowFlags = m_pWidget->windowFlags();

    //---from Qt docs of setWindowFlags()----
    //Note: This function calls setParent() when
    //changing the flags for a window, causing the
    //widget to be hidden. You must call show()
    //to make the widget visible again..

    bool visible = m_pWidget->isVisible();

    m_pWidget->setMouseTracking( true );
    m_pWidget->setWindowFlags( m_windowFlags | Qt::CustomizeWindowHint | Qt::FramelessWindowHint );
    //Bug fix, mouse move events does not propagate from child widgets.
    //so need the hover events.
    m_pWidget->setAttribute( Qt::WA_Hover );

    updateRubberBandStatus();

    m_pWidget->setVisible( visible );
}

CWidgetData::~CWidgetData()
{
    //---from Qt docs of setWindowFlags()----
    //Note: This function calls setParent() when
    //changing the flags for a window, causing the
    //widget to be hidden. You must call show()
    //to make the widget visible again..

    bool visible = m_pWidget->isVisible();

    m_pWidget->setMouseTracking( false );
    m_pWidget->setWindowFlags( m_windowFlags );//^  Qt::CustomizeWindowHint ^ Qt::FramelessWindowHint );
    m_pWidget->setAttribute( Qt::WA_Hover, false );

    m_pWidget->setVisible( visible );

    delete m_pRubberBand;
}

void CWidgetData::updateRubberBandStatus()
{
    if ( m_pHelperImpl->m_bUseRubberBandOnMove || m_pHelperImpl->m_bUseRubberBandOnResize )
    {
        if ( !m_pRubberBand )
            m_pRubberBand = new QRubberBand( QRubberBand::Rectangle );
    }
    else
    {
        delete m_pRubberBand;
        m_pRubberBand = 0;
    }
}

void CWidgetData::setBorderWidth(int width)
{
    m_pressedMousePos.m_borderWidth = width;
    m_moveMousePos.m_borderWidth = width;
}

void CWidgetData::setEdgeWidth(int width)
{
    m_pressedMousePos.m_edgeWidth = width;
    m_moveMousePos.m_edgeWidth = width;
}

void CWidgetData::setWidgetGeometry(QWidget* pWidget)
{
    m_pWidgetGeometry = pWidget;
}

QWidget* CWidgetData::widget()
{
    return m_pWidget;
}

void CWidgetData::handleWidgetEvent( QEvent* event )
{
    switch ( event->type() )
    {
        default: //qDebug() << "Event = " << event;
            break;

        case QEvent::MouseButtonPress:
            handleMousePressEvent( static_cast<QMouseEvent*>( event ) );
            break;

        case QEvent::MouseButtonRelease:
            handleMouseReleaseEvent( static_cast<QMouseEvent*>( event ) );
            break;

        case QEvent::MouseMove:
            handleMouseMoveEvent( static_cast<QMouseEvent*>( event ) );
            break;

        case QEvent::Leave:
            handleLeaveEvent( event );
            break;

        //Bug fix, hover event is necessary coz child widget does not
        //propagate mousemove events. so the cursor remains in edge shape
        //even in middle of widget.
        case QEvent::HoverMove:
            handleHoverMoveEvent( static_cast<QHoverEvent*>( event ) );
            break;

        //case QEvent::Enter:
            //qDebug() << "Enter event";//d->handleEnterEvent( event );
            //break;
    }
}

void CWidgetData::updateCursorShape( const QPoint& globalMousePos )
{
    if ( m_pWidget->isFullScreen() || m_pWidget->isMaximized() )
    {
        if ( m_bCursorShapeChanged )
            m_pWidget->unsetCursor();

        return;
    }

    QRect frameRect;
    // Test if we have a custom widget (e.g. dialog widget) in order to adapt mouse interaction to real geometry
    if(m_pWidgetGeometry != nullptr)
    {
        // Get widget geometry
        frameRect = m_pWidgetGeometry->geometry();
        // Set widget position in global coordinate because we test with mouse pos in global position
        frameRect.moveTopLeft(m_pWidget->mapToGlobal(frameRect.topLeft()));
    }
    else // else we get frameGeometry from current widget (e.g. mainwindow)
        frameRect = m_pWidget->frameGeometry();

    // Calculate intersection between widget geometry and mouse pos
    m_moveMousePos.recalculate( globalMousePos, frameRect );

    if( m_moveMousePos.m_bOnTopLeftEdge || m_moveMousePos.m_OnBottomRightEdge )
    {
        m_pWidget->setCursor( Qt::SizeFDiagCursor );
        m_bCursorShapeChanged = true;
    }
    else if( m_moveMousePos.m_bOnTopRightEdge || m_moveMousePos.m_bOnBottomLeftEdge )
    {
        m_pWidget->setCursor( Qt::SizeBDiagCursor );
        m_bCursorShapeChanged = true;
    }
    else if( m_moveMousePos.m_bOnLeftEdge || m_moveMousePos.m_bOnRightEdge )
    {
        m_pWidget->setCursor( Qt::SizeHorCursor );
        m_bCursorShapeChanged = true;
    }
    else if( m_moveMousePos.m_bOnTopEdge || m_moveMousePos.m_bOnBottomEdge )
    {
        m_pWidget->setCursor( Qt::SizeVerCursor );
        m_bCursorShapeChanged = true;
    }
    else if( m_moveMousePos.m_bOnTopBorder)
    {
        m_pWidget->setCursor( Qt::SizeAllCursor );
        m_bCursorShapeChanged = true;
    }
    else
    {
        if ( m_bCursorShapeChanged )
        {
          m_pWidget->unsetCursor();
          m_bCursorShapeChanged = false;
        }
    }
}

void CWidgetData::resizeWidget( const QPoint& globalMousePos )
{
    QRect origRect;

    if ( m_pHelperImpl->m_bUseRubberBandOnResize )
        origRect = m_pRubberBand->frameGeometry();
    else
        origRect = m_pWidget->frameGeometry();

    int left = origRect.left();
    int top = origRect.top();
    int right = origRect.right();
    int bottom = origRect.bottom();

    origRect.getCoords( &left, &top, &right, &bottom );

    int minWidth = m_pWidget->minimumWidth();
    int minHeight = m_pWidget->minimumHeight();

    if ( m_pressedMousePos.m_bOnTopLeftEdge )
    {
        left = globalMousePos.x();
        top = globalMousePos.y();
    }
    else if ( m_pressedMousePos.m_bOnBottomLeftEdge )
    {
        left = globalMousePos.x();
        bottom = globalMousePos.y();
    }
    else if ( m_pressedMousePos.m_bOnTopRightEdge )
    {
        right = globalMousePos.x();
        top = globalMousePos.y();
    }
    else if ( m_pressedMousePos.m_OnBottomRightEdge )
    {
        right = globalMousePos.x();
        bottom = globalMousePos.y();
    }
    else if ( m_pressedMousePos.m_bOnLeftEdge )
        left = globalMousePos.x();
    else if ( m_pressedMousePos.m_bOnRightEdge )
        right = globalMousePos.x();
    else if ( m_pressedMousePos.m_bOnTopEdge )
        top = globalMousePos.y();
    else if ( m_pressedMousePos.m_bOnBottomEdge )
        bottom = globalMousePos.y();

    QRect newRect( QPoint(left, top), QPoint(right, bottom) );

    if ( newRect.isValid() )
    {
        if ( minWidth > newRect.width() )
        {
            //determine what has caused the width change.
            if( left != origRect.left() )
                newRect.setLeft( origRect.left() );
            else
                newRect.setRight( origRect.right() );
        }

        if ( minHeight > newRect.height() )
        {
            //determine what has caused the height change.
            if ( top != origRect.top() )
                newRect.setTop( origRect.top() );
            else
                newRect.setBottom( origRect.bottom() );
        }

        if ( m_pHelperImpl->m_bUseRubberBandOnResize )
            m_pRubberBand->setGeometry( newRect );
        else
            m_pWidget->setGeometry( newRect );
    }
    else
    {
        //qDebug() << "Calculated Rect is not valid" << newRect;
    }
}

void CWidgetData::moveWidget( const QPoint& globalMousePos )
{
    if ( m_pHelperImpl->m_bUseRubberBandOnMove )
        m_pRubberBand->move( globalMousePos - m_dragPos );
    else
        m_pWidget->move( globalMousePos - m_dragPos );
}

void CWidgetData::handleMousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton )
    {
        m_bLeftButtonPressed = true;

        QRect frameRect;
        // Test if we have a custom widget (e.g. dialog widget) in order to adapt mouse interaction to real geometry
        if(m_pWidgetGeometry != nullptr)
        {
            // Get widget geometry
            frameRect = m_pWidgetGeometry->geometry();
            // Set widget position in global coordinate because we test with mouse pos in global position
            frameRect.moveTopLeft(m_pWidget->mapToGlobal(frameRect.topLeft()));
        }
        else // else we get frameGeometry from current widget (e.g. mainwindow)
            frameRect = m_pWidget->frameGeometry();

        // Calculate intersection between widget geometry and mouse pos
        m_pressedMousePos.recalculate( event->globalPosition().toPoint(), frameRect );
        // Set new drag position relative to current widget
        m_dragPos = event->globalPosition().toPoint() - m_pWidget->frameGeometry().topLeft();

        if ( m_pressedMousePos.m_bOnEdges  )
        {
            if ( m_pHelperImpl->m_bUseRubberBandOnResize )
            {
                m_pRubberBand->setGeometry( frameRect );
                m_pRubberBand->show();
            }
        }
        else if ( m_pHelperImpl->m_bUseRubberBandOnMove )
        {
            m_pRubberBand->setGeometry( frameRect );
            m_pRubberBand->show();
        }
    }
}

void CWidgetData::handleMouseReleaseEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton )
    {
        m_bLeftButtonPressed = false;
        m_pressedMousePos.reset();

        if ( m_pRubberBand && m_pRubberBand->isVisible() )
        {
            m_pRubberBand->hide();
            m_pWidget->setGeometry( m_pRubberBand->geometry() );
        }
    }
}

void CWidgetData::handleMouseMoveEvent( QMouseEvent* event )
{
    if ( m_bLeftButtonPressed )
    {
        if ( m_pHelperImpl->m_bWidgetResizable && m_pressedMousePos.m_bOnEdges )
            resizeWidget( event->globalPosition().toPoint() );
        else if ( m_pHelperImpl->m_bWidgetMovable && m_pressedMousePos.m_bOnBorders )
            moveWidget( event->globalPosition().toPoint() );
    }
    else if ( m_pHelperImpl->m_bWidgetResizable )
        updateCursorShape( event->globalPosition().toPoint() );
}

void CWidgetData::handleLeaveEvent( QEvent* /*event*/ )
{
    if ( !m_bLeftButtonPressed )
        m_pWidget->unsetCursor();
}

void CWidgetData::handleHoverMoveEvent( QHoverEvent* event )
{
    if ( m_pHelperImpl->m_bWidgetResizable )
        updateCursorShape( m_pWidget->mapToGlobal( event->position().toPoint() ) );
}


CFramelessHelper::CFramelessHelper( QObject* parent ) : QObject( parent ), m_pHelperImpl( new CFramelessHelperImpl )
{
    m_pHelperImpl->m_bWidgetMovable = true;
    m_pHelperImpl->m_bWidgetResizable = true;
    m_pHelperImpl->m_bUseRubberBandOnResize = false;
    m_pHelperImpl->m_bUseRubberBandOnMove = false;
}

CFramelessHelper::~CFramelessHelper()
{
    QList<QWidget*> keys = m_pHelperImpl->m_widgetDataHash.keys();
    int size = keys.size();

    for ( int i = 0; i < size; ++i )
        delete m_pHelperImpl->m_widgetDataHash.take( keys[i] );

    delete m_pHelperImpl;
}

bool CFramelessHelper::eventFilter( QObject *obj, QEvent *event )
{
  QEvent::Type type = event->type();

  if( type == QEvent::MouseMove ||
       type == QEvent::HoverMove ||
       type == QEvent::MouseButtonPress ||
       type == QEvent::MouseButtonRelease ||
       type == QEvent::Leave
    )
    {
        CWidgetData* data = m_pHelperImpl->m_widgetDataHash.value( static_cast<QWidget*>(obj) );
        if ( data )
            data->handleWidgetEvent( event );
    }
    return false;
}

void CFramelessHelper::activateOn( QWidget* topLevelWidget )
{
    if ( m_pHelperImpl->m_widgetDataHash.contains( topLevelWidget ) )
        return;

    CWidgetData* data = new CWidgetData( m_pHelperImpl, topLevelWidget );
    m_pHelperImpl->m_widgetDataHash.insert( topLevelWidget, data );
    topLevelWidget->installEventFilter( this );
}

void CFramelessHelper::removeFrom( QWidget* topLevelWidget )
{
    CWidgetData* data = m_pHelperImpl->m_widgetDataHash.take( topLevelWidget );
    if ( data )
    {
        topLevelWidget->removeEventFilter( this );
        delete data;
    }
}

void CFramelessHelper::setWidgetMovable( bool movable )
{
    m_pHelperImpl->m_bWidgetMovable = movable;
}

bool CFramelessHelper::isWidgetMovable()
{
    return m_pHelperImpl->m_bWidgetMovable;
}

void CFramelessHelper::setWidgetResizable( bool resizable )
{
    m_pHelperImpl->m_bWidgetResizable = resizable;
}

bool CFramelessHelper::isWidgetResizable()
{
    return m_pHelperImpl->m_bWidgetResizable;
}

void CFramelessHelper::useRubberBandOnMove( bool use )
{
    m_pHelperImpl->m_bUseRubberBandOnMove = use;
    QList<CWidgetData*> list = m_pHelperImpl->m_widgetDataHash.values();
    int size = list.size();

    for ( int i = 0; i < size; ++i )
        list[i]->updateRubberBandStatus();
}

bool CFramelessHelper::isUsingRubberBandOnMove()
{
    return m_pHelperImpl->m_bUseRubberBandOnMove;
}

void CFramelessHelper::useRubberBandOnResize( bool use )
{
    m_pHelperImpl->m_bUseRubberBandOnResize = use;
    QList<CWidgetData*> list = m_pHelperImpl->m_widgetDataHash.values();
    int size = list.size();

    for ( int i = 0; i < size; ++i )
        list[i]->updateRubberBandStatus();
}

bool CFramelessHelper::isUsingRubberBandOnResisze()
{
    return m_pHelperImpl->m_bUseRubberBandOnResize;
}

void CFramelessHelper::setBorderWidth( int newBorderWidth )
{
    if ( newBorderWidth >= 0 )
    {
        for(auto it = m_pHelperImpl->m_widgetDataHash.begin(); it!=m_pHelperImpl->m_widgetDataHash.end(); ++it)
            it.value()->setBorderWidth(newBorderWidth);
    }
}

void CFramelessHelper::setEdgeWidth(int newWidth)
{
    if ( newWidth >= 0 )
    {
        for(auto it = m_pHelperImpl->m_widgetDataHash.begin(); it!=m_pHelperImpl->m_widgetDataHash.end(); ++it)
            it.value()->setEdgeWidth(newWidth);
    }
}

void CFramelessHelper::setWidgetGeometry(QWidget* pWidget)
{
    for(auto it = m_pHelperImpl->m_widgetDataHash.begin(); it!=m_pHelperImpl->m_widgetDataHash.end(); ++it)
        it.value()->setWidgetGeometry(pWidget);
}

