/*
    NcFramelessHelper, an easy way to support move/resize on
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

//Version 3.0.0

#ifndef NC_FRAMELESS_HELPER_H
#define NC_FRAMELESS_HELPER_H

/**
 * @file      CFramelessHelper.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CFramelessHelper definition
 *
 * @details   Details
 */

#include <QObject>

class CFramelessHelperImpl;

/**
 * @brief
 *
 */
class CFramelessHelper : public QObject
{
    public:

        explicit CFramelessHelper( QObject* parent = 0 );
        ~CFramelessHelper();

        /**
        * @brief
        *
        * @param topLevelWidget
        */
        void activateOn( QWidget* topLevelWidget );
        /**
        * @brief
        *
        * @param topLevelWidget
        */
        void removeFrom( QWidget* topLevelWidget );

        /**
        * @brief
        *
        * @param movable
        */
        void setWidgetMovable( bool movable );
        /**
        * @brief
        *
        * @return bool
        */
        bool isWidgetMovable();
        /**
        * @brief
        *
        * @param resizable
        */
        void setWidgetResizable( bool resizable );
        /**
        * @brief
        *
        * @return bool
        */
        bool isWidgetResizable();
        /**
        * @brief
        *
        * @param use
        */
        void useRubberBandOnMove( bool use );
        /**
        * @brief
        *
        * @return bool
        */
        bool isUsingRubberBandOnMove();
        /**
        * @brief
        *
        * @param use
        */
        void useRubberBandOnResize( bool use );
        /**
        * @brief
        *
        * @return bool
        */
        bool isUsingRubberBandOnResisze();

        //Make sure to leave the same content margins
        //around the widget as the newBorderWidth
        //this can be done by
        //yourWidget->layout()->setMargin( newBorderWidth );
        //otherwise your widget will not expose the
        //area where this class works
        /**
        * @brief
        *
        * @param newBorderWidth
        */
        void setBorderWidth( int newBorderWidth );
        /**
        * @brief
        *
        * @param newBorderWidth
        */
        void setEdgeWidth( int newWidth );

        void setWidgetGeometry(QWidget* pWidget);

    protected:

        virtual bool eventFilter( QObject* obj, QEvent* event );

    private:

        CFramelessHelperImpl* m_pHelperImpl;
};

#endif // NC_FRAMELESS_HELPER_H
