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

#include "CBubbleTip.h"
#include <QFontMetrics>
#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>
#include <QStyle>
#include <QIcon>

CBubbleTip::CBubbleTip(QWidget* parent, QWidget* pMainApp, int flags) : QWidget( parent )
{
    m_pApp = pMainApp;
    m_title = "";
    m_text = "";
    m_duration = -1;
    m_icon = QPixmap();
    m_flags = flags;
    init();
}

CBubbleTip::CBubbleTip( QStyle::StandardPixmap icon, QString title, QString text, int duration, QWidget* parent ) : QWidget( parent )
{
    m_pCloseBtn = new CBubbleTipBtn( CBubbleTipBtn::Close, this );
    m_title = title;
    m_text = text;
    m_duration = duration;
    m_icon = qApp->style()->standardIcon( icon ).pixmap( QSize( 15, 15 ) );
    init();
}

CBubbleTip::CBubbleTip( QPixmap icon, QString title, QString text, int duration, QWidget* parent ) : QWidget( parent )
{
    m_pCloseBtn = new CBubbleTipBtn( CBubbleTipBtn::Close, this );
    m_title = title;
    m_text = text;
    m_duration = duration;
    m_icon = icon.scaled( QSize( 15, 15 ), Qt::KeepAspectRatio );
    init();
}

CBubbleTip::CBubbleTip( QString title, QString text, int duration, QWidget* parent ) : QWidget( parent )
{
    m_pCloseBtn = new CBubbleTipBtn( CBubbleTipBtn::Close, this );
    m_title = title;
    m_text = text;
    m_duration = duration;
    init();
}

void CBubbleTip::init()
{
    setWindowFlags(  Qt::FramelessWindowHint | Qt::ToolTip );
    setAttribute( Qt::WA_TranslucentBackground, true );

    createAnimation();
    createRects();
    defineArrowPosition();
    initButtons();

    if ( parentWidget() != 0 )
    {
        parentWidget()->installEventFilter( this );
        QWidget* w = parentWidget()->parentWidget();
        while ( w != 0  )
        {
            w->installEventFilter( this );
            w = w->parentWidget();
        }
    }

    setFixedSize( m_popupRect.size() + QSize( 60, 60 ) );

    initConnections();
}

void CBubbleTip::initConnections()
{
    if(m_pCloseBtn)
        connect( m_pCloseBtn, SIGNAL(clicked()), this, SLOT(close()) );

    if(m_pCheckBtn)
        connect(m_pCheckBtn, &CBubbleTipBtn::clicked, this, &CBubbleTip::doChecked);

    if(m_pNextBtn)
        connect(m_pNextBtn, &CBubbleTipBtn::clicked, this, &CBubbleTip::doNext);

    connect( m_pAnimation, SIGNAL(finished()), this, SLOT(close()) );
}

void CBubbleTip::initButtons()
{
    if(m_flags & Close)
        m_pCloseBtn = new CBubbleTipBtn(CBubbleTipBtn::Close, this);
    else
    {
        if(m_pCloseBtn)
            m_pCloseBtn->deleteLater();
    }

    if(m_flags & Config)
        m_pConfigBtn = new CBubbleTipBtn(CBubbleTipBtn::Config, this);
    else
    {
        if(m_pConfigBtn)
            m_pConfigBtn->deleteLater();
    }

    if(m_flags & Check)
    {
        m_pCheckBtn = new CBubbleTipBtn(CBubbleTipBtn::Check, this);
        m_pCheckBtn->setCheckable(true);
    }
    else
    {
        if(m_pCheckBtn)
            m_pCheckBtn->deleteLater();
    }

    if(m_flags & Next)
    {
        m_pNextBtn = new CBubbleTipBtn(CBubbleTipBtn::Next, this);
        m_pNextBtn->setFixedSize(50,25);
    }
    else
    {
        if(m_pNextBtn)
            m_pNextBtn->deleteLater();
    }

    moveButtons();
}

void CBubbleTip::createAnimation()
{
    m_pAnimation = new QPropertyAnimation( this, "windowOpacity" );
    m_pAnimation->setStartValue( 0.9 );
    m_pAnimation->setEndValue( 0.0 );
    if(m_duration > -1)
        m_pAnimation->setDuration( m_duration);
    m_pAnimation->setEasingCurve( QEasingCurve::InCubic );   
}

void CBubbleTip::createRects()
{
    QFont font = this->font();
    font.setBold( true );
    font.setPixelSize( 12 );
    QFontMetrics metrics( font );
    QRect rect = metrics.boundingRect( QRect( 10, 10, 500, 500 ), Qt::TextSingleLine, m_title );
    // Width minimum is 100
    if ( rect.width() < 100 )
        rect.setWidth( 100 );

    font.setPixelSize( 12 );
    font.setBold( false );
    setFont( font );
    metrics = QFontMetrics( font );

    m_textRect = metrics.boundingRect( QRect( 10, 30, rect.width() + 150, 500), Qt::TextWordWrap, m_text );
    m_textRect.setBottom(m_textRect.bottom()+30); // Adjust height in order to show all character
    if ( m_textRect.width() < rect.width() )
        m_textRect.setWidth( rect.width() + 90 );

    m_popupRect = QRect( 0, 0, m_textRect.width() + 20, m_textRect.height() + 40 );

    if(m_flags & Check)
    {
        auto r = metrics.boundingRect( QRect( 0, 0, 500, 500), Qt::TextWordWrap, m_checkText);
        m_popupRect.setHeight(m_popupRect.height()+r.height());
        m_popupRect.setWidth(std::max(m_popupRect.width(), r.width()+60));
    }

    if(m_flags & Next)
    {
        m_popupRect.setHeight(m_popupRect.height() + 10);
    }
}

void CBubbleTip::defineArrowPosition()
{
    QSize desktopSize;
    if(parentWidget())
    {
        /*// Better approach when multiple screens
        // Parent widget must be non null
        QScreen* pScreen = parentWidget()->window()->windowHandle()->screen();
        desktopSize = pScreen->availableSize();*/
        desktopSize = m_pApp->size();
    }
    else
        desktopSize = QApplication::primaryScreen()->size();

    if ( m_currentPos.x() < desktopSize.width() / 2 )
    {
        if ( m_currentPos.y() < desktopSize.height() / 2 )
            m_arrowPos = TopLeft;
        else
            m_arrowPos = BottomLeft;
    }
    else
    {
        if ( m_currentPos.y() < desktopSize.height() / 2 )
            m_arrowPos = TopRight;
        else
            m_arrowPos = BottomRight;
    }
}

CBubbleTip::~CBubbleTip()
{
    if(m_pCloseBtn)
        m_pCloseBtn->deleteLater();

    if(m_pConfigBtn)
        m_pConfigBtn->deleteLater();

    if(m_pCheckBtn)
        m_pCheckBtn->deleteLater();

    if(m_pNextBtn)
        m_pNextBtn->deleteLater();
}

void CBubbleTip::paintEvent(QPaintEvent * /*ev*/ )
{
    QPainter painter( this );

    painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );

    drawBubble(&painter);
    drawIconAndTitle(&painter);
    drawDescription(&painter);
    drawCheck(&painter);
}

void CBubbleTip::drawBubble(QPainter* painter)
{
    QRect popupRect = relativePopupRect();
    QPainterPath path;
    QPolygon p;

    painter->setBrush( Qt::white );
    painter->setPen( QColor( 50, 50, 50 , 90 ) );
    painter->setFont( this->font() );

    switch( m_arrowPos )
    {
        case BottomLeft :
            p << QPoint( 30, popupRect.height() + 60 )
              << QPoint( 60, popupRect.height() + 30 )
              << QPoint( 90, popupRect.height() + 30 );
            break;
        case TopLeft :
            p << QPoint( 30, 0 ) << QPoint( 60, 30 ) << QPoint( 90, 30 );
            break;
        case BottomRight :
            p << QPoint( popupRect.width() - 30, popupRect.height() + 60 )
              << QPoint( popupRect.width() - 60, popupRect.height() + 30 )
              << QPoint( popupRect.width() - 90, popupRect.height() +30 );
            break;
        case TopRight :
            p << QPoint( popupRect.width() - 30, 0 )
              << QPoint( popupRect.width() - 60, 30 )
              << QPoint( popupRect.width() - 90, 30 );
            break;
    }

    path.addPolygon( p );
    path.addRoundedRect( popupRect, 20, 20 );
    path = path.simplified();
    painter->drawPath( path );
}

void CBubbleTip::drawDescription(QPainter* painter)
{
    QFont font = this->font();
    font.setBold(false);
    font.setPixelSize( 12 );
    painter->setFont( font );
    painter->setPen( QColor(20, 20, 20 ) );

    QRect textRect = relativeTextRect();
    painter->drawText( textRect, m_text );
}

void CBubbleTip::drawIconAndTitle(QPainter* painter)
{
    QRect textRect = relativeTextRect();
    QFont font = this->font();
    font.setBold( true );
    font.setPixelSize( 12 );
    painter->setFont( font );
    painter->setPen( QColor( 48, 159, 220 ) );
    if ( !m_icon.isNull() )
    {
        painter->drawText( textRect.topLeft() + QPoint( 20, -10 ), m_title );
        painter->drawPixmap( textRect.topLeft() + QPoint( 0, -22 ), m_icon );
    }
    else
    {
        painter->drawText( textRect.topLeft() + QPoint( 5, -10 ), m_title );
    }
}

void CBubbleTip::drawCheck(QPainter* painter)
{
    if(m_flags & Check)
    {
        QFont font = this->font();
        font.setItalic(true);
        font.setPixelSize( 12 );
        painter->setFont( font );

        QRect r = relativePopupRect();
        painter->drawText( r.bottomLeft() - QPoint( -40, 20 ), m_checkText );
    }
}

CBubbleTip::ArrowPosition CBubbleTip::arrowPosition()
{
    return m_arrowPos;
}

void CBubbleTip::setArrowPosition( CBubbleTip::ArrowPosition arrowPos )
{
    m_arrowPos = arrowPos;
}

QRect CBubbleTip::relativePopupRect()
{
    QRect rect = m_popupRect;
    if ( m_arrowPos == BottomRight)
        rect.translate( 0, 30 );
    else if( m_arrowPos == TopRight )
        rect.translate(0, 30);
    else if( m_arrowPos == BottomLeft )
        rect.translate(0, 30);
    else if( m_arrowPos == TopLeft )
        rect.translate(0, 30);


    return rect;
}

QRect CBubbleTip::relativeTextRect()
{
    QRect rect = m_textRect;
    if ( m_arrowPos ==  TopLeft )
        rect.translate( 0, 30 );
    else if( m_arrowPos == TopRight )
        rect.translate(0, 30);
    else if( m_arrowPos == BottomLeft )
        rect.translate(0, 30);
    else if( m_arrowPos == BottomRight )
        rect.translate(0, 30);

    return rect;
}

void CBubbleTip::moveButtons()
{
    QRect r = relativePopupRect();
    if(m_pCloseBtn)
        m_pCloseBtn->move( r.topRight() - QPoint( 30, - 5 ) );
    if(m_pCheckBtn)
        m_pCheckBtn->move( r.bottomLeft() - QPoint( -15, 35 ) );
    if(m_pNextBtn)
        m_pNextBtn->move(m_textRect.center() + QPoint(-19,m_textRect.height()/2+5));

}

void CBubbleTip::enterEvent( QEnterEvent* ev )
{
    setWindowOpacity( 0.9 );
    m_pAnimation->stop();
    QWidget::enterEvent( ev );
}

void CBubbleTip::leaveEvent( QEvent* ev )
{
    if ( m_bIsAnimated == true )
        m_pAnimation->start();

    QWidget::leaveEvent( ev );
}

bool CBubbleTip::close()
{
    m_pAnimation->stop();
    m_bIsAnimated = false;
    emit finished();

    return QWidget::close();
}

void CBubbleTip::show()
{
    QWidget::show();
    m_pAnimation->stop();
    if ( m_duration > -1 )
        m_bIsAnimated = true;
    else
        m_bIsAnimated = false;

    if ( m_bIsAnimated == true )
        m_pAnimation->start();
}

bool CBubbleTip::eventFilter(QObject* /*obj*/, QEvent* ev)
{
    if ( ev->type() != QEvent::Move && ev->type() != QEvent::Resize )
        return false;

    auto pWidget = parentWidget();
    //QScreen* pScreen = pWidget->window()->windowHandle()->screen();
    QSize desktopSize = m_pApp->size();//pScreen->availableSize();
    m_currentPos = pWidget->mapTo(m_pApp, parentWidget()->rect().center());
    QPoint translate_pos;
    if ( m_currentPos.x() < desktopSize.width() / 2 )
    {
        if ( m_currentPos.y() < desktopSize.height() / 2 )
            translate_pos = pWidget->rect().bottomRight();
        else
            translate_pos = pWidget->rect().topRight();
    }
    else
    {
        if ( m_currentPos.y() < desktopSize.height() / 2 )
            translate_pos = pWidget->rect().bottomLeft();
        else
            translate_pos = pWidget->rect().topLeft();
    }
    move(pWidget->mapToGlobal(translate_pos));

    return false;
}

void CBubbleTip::move( QPoint pos )
{
    QWidget::move( pos );
    defineArrowPosition();
    switch( m_arrowPos )
    {
        case BottomLeft :
            pos.setY( pos.y() - m_popupRect.height() - 60 );
        case TopLeft :
            pos.setX( pos.x() - 30 );
            break;
        case BottomRight :
            pos.setY( pos.y() - m_popupRect.height() - 60 );
        case TopRight :
            pos.setX( pos.x() - m_popupRect.width() + 30 );
            break;
    }
    QWidget::move( pos );
    update();
}

void CBubbleTip::update()
{
    setWindowFlags(  Qt::FramelessWindowHint | Qt::ToolTip );
    setAttribute( Qt::WA_TranslucentBackground, true );

    createRects();
    defineArrowPosition();
    moveButtons();

    if ( parentWidget() != 0 )
    {
        parentWidget()->installEventFilter( this );
        QWidget* w = parentWidget()->parentWidget();
        while ( w != 0  )
        {
            w->installEventFilter( this );
            w = w->parentWidget();
        }
    }

    setFixedSize( m_popupRect.size() + QSize( 60, 60 ) );
}

void CBubbleTip::setTitle(const QString& title)
{
    m_title = title;
}

void CBubbleTip::setText(const QString& text)
{
    m_text = text;
}

void CBubbleTip::setCheckText(const QString& text)
{
    m_checkText = text;
}

void CBubbleTip::setIcon(const QPixmap& pix)
{
    m_icon = pix.scaled( QSize( 15, 15 ), Qt::KeepAspectRatio );
}

void CBubbleTip::setIcon(QStyle::StandardPixmap pix)
{
    m_icon = qApp->style()->standardIcon( pix ).pixmap( QSize( 15, 15 ) );
}

void CBubbleTip::setDuration(int seconds)
{
    m_duration = seconds;
}

void CBubbleTip::setAppParent(QWidget* pAppParent)
{
    m_pApp = pAppParent;
}

void CBubbleTip::setCurrentPos(const QPoint& pos)
{
    m_currentPos = pos;
}

void CBubbleTip::setFlags(int flags)
{
    m_flags = flags;
}
