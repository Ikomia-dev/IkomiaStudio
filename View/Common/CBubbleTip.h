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

#ifndef CBUBBLETIP_H
#define CBUBBLETIP_H

#include <QFrame>
#include <QPushButton>
#include <QPainter>
#include <QPropertyAnimation>

#include <QStyle>

/*
  Class TipButton : button used in the BalloonTip class
*/
class CBubbleTipBtn : public QPushButton
{
      Q_OBJECT
    public:
        /* The role determines the look-and-feel of the button. Not used for now... */
        enum TipButtonRoles {
            Close,
            Config,
            Check,
            Next
        };

        Q_DECLARE_FLAGS( TipButtonRole, TipButtonRoles )
        Q_ENUMS( TipButtonRoles )

        CBubbleTipBtn( TipButtonRole role, QWidget* parent = 0 ) : QPushButton( parent )
        {
            setFixedSize(m_size);
            m_role = role;
        }

        void paintEvent(QPaintEvent *)
        {
            QStyleOptionButton opt;
            initStyleOption(&opt);

            QPainter painter( this );
            painter.setRenderHint( QPainter::Antialiasing );

            QColor color(150,150,150);
            QColor highlight(255,122,0);

            QRect border( 2, 2, 16, 16 );
            if(opt.state & QStyle::State_MouseOver)
                painter.setPen( QPen( highlight, 2 ) );
            else
                painter.setPen( QPen( color, 2 ) );

            if ( m_role == CBubbleTipBtn::Close )
            {
                painter.drawRoundedRect( border, 3, 3 );

                painter.setPen( QPen( color, 3 ) );
                painter.drawLine( 6, 6, 14, 14 );
                painter.drawLine( 6, 14, 14, 6 );
            }
            else if ( m_role == CBubbleTipBtn::Config )
            {
                painter.drawRoundedRect( border, 3, 3 );

                painter.setPen( Qt::transparent );
                painter.setBrush( color );
                QPainterPath path;
                QPainterPath path2;
                path.addEllipse( 4, 3, 12, 9 );
                path.addRect( 8, 3, 4, 4 );
                path2.addRect( 8, 11, 4, 5 );

                path = path.united(path2).simplified();
                painter.drawPath( path );
            }
            else if( m_role == CBubbleTipBtn::Check)
            {
                painter.drawRoundedRect( border, 3, 3 );

                if(isChecked())
                {
                    QRect middle( 8, 8, 4, 4);
                    painter.setPen( QPen( color, 1 ) );
                    painter.setBrush(QBrush(color));
                    painter.drawEllipse(middle);
                }
            }
            else if( m_role == CBubbleTipBtn::Next)
            {
                QRect r(3, 3, 40, 20);
                painter.setBrush(QBrush(color));
                painter.drawRoundedRect(r, 3, 3);

                QColor colorTxt(255,255,255);
                painter.setPen( QPen( colorTxt, 3 ) );
                painter.setBrush( QBrush(colorTxt) );
                painter.drawText(r, Qt::AlignCenter, "Next");
            }
        }

    protected:
        TipButtonRole   m_role;
        QSize           m_size = QSize(20,20);
};

/*
  Class BalloonTip : A balloon tip used to display messages
*/
class CBubbleTip : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( ArrowPosition m_arrowPos READ arrowPosition WRITE setArrowPosition )
    public:

      /* This enum determine in which corner will be displayed the arrow */
      enum ArrowPositions {
        TopLeft     = 0,
        TopRight    = 1,
        BottomLeft  = 2,
        BottomRight = 3
      };

      Q_DECLARE_FLAGS( ArrowPosition, ArrowPositions )
      Q_ENUMS( ArrowPositions )

      enum Buttons {
          None = 0x00000000,
          Close = 0x00000001,
          Config = 0x00000002,
          Check = 0x00000004,
          Next = 0x00000008
      };

      explicit CBubbleTip(QWidget* parent = 0, QWidget* pMainApp = nullptr, int flags = None);
      explicit CBubbleTip( QString title, QString text, int duration = 2000, QWidget* parent = 0 );
      explicit CBubbleTip( QPixmap pix, QString title, QString text, int duration = 2000, QWidget* parent = 0 );
      explicit CBubbleTip( QStyle::StandardPixmap pix, QString title, QString text, int duration = 2000, QWidget* parent = 0 );

      ~CBubbleTip();
      ArrowPosition arrowPosition();
      void          setArrowPosition( ArrowPosition pos );
      void          move( QPoint pos );
      void          update();

      void          setTitle(const QString& title);
      void          setText(const QString& text);
      void          setCheckText(const QString& text);
      void          setIcon(const QPixmap& pix);
      void          setIcon(QStyle::StandardPixmap pix);
      void          setDuration(int seconds);
      void          setAppParent(QWidget* pAppParent);
      void          setCurrentPos(const QPoint& pos);
      void          setFlags(int flags);

    signals:
      void          finished();
      void          doChecked(bool isChecked);
      void          doNext();

    public slots:
      void          show();
      bool          close();

    protected:

      void          init();
      void          initConnections();
      void          initButtons();

      void          moveButtons();

      void          paintEvent( QPaintEvent* ev);

      void          drawBubble(QPainter* painter);
      void          drawDescription(QPainter* painter);
      void          drawIconAndTitle(QPainter* painter);
      void          drawCheck(QPainter* painter);

      QRect         relativePopupRect();
      QRect         relativeTextRect();

      void          enterEvent( QEvent* ev );
      void          leaveEvent( QEvent* ev );
      void          createAnimation();
      void          defineArrowPosition();
      void          createRects();
      bool          eventFilter(QObject *, QEvent *);

    private:
      QPixmap               m_icon;
      QString               m_text;
      QString               m_title;
      QString               m_checkText;
      QPoint                m_currentPos;
      ArrowPosition         m_arrowPos;
      QRect                 m_popupRect;
      QRect                 m_textRect;
      int                   m_duration;
      int                   m_flags;
      bool                  m_bIsAnimated;

      QWidget*              m_pApp = nullptr;
      QPropertyAnimation*   m_pAnimation = nullptr;
      CBubbleTipBtn*        m_pCloseBtn = nullptr;
      CBubbleTipBtn*        m_pConfigBtn = nullptr;
      CBubbleTipBtn*        m_pCheckBtn = nullptr;
      CBubbleTipBtn*        m_pNextBtn = nullptr;
};

#endif // CBUBBLETIP_H
