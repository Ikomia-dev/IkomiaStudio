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

#ifndef CDATAVIEW_H
#define CDATAVIEW_H

/**
 * @file      CDataDisplay.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataDisplay definition
 *
 * @details   Details
 */

#include "Main/forwards.hpp"
#include "CDataDisplayLayout.h"
#include "Main/AppDefine.hpp"

class CViewPropertyIO;

/**
 * @brief Base class for displaying generic data
 *
 */
class CDataDisplay : public QFrame
{
    Q_OBJECT

    public:

        enum DataDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008
        };

        CDataDisplay(QWidget* parent=nullptr, const QString &name="", int flags=NONE);

        virtual void            addDataView(CDataDisplay* pDataView, int r, int c);
        virtual void            addDataView(CDataDisplay* pDataView);

        virtual void            removeDataView(CDataDisplay* pDataView);
        virtual void            removeDataView(int r = 0 ,int c = 0);
        virtual void            removeDataViews(const QList<CDataDisplay*>& toRemoveList, bool bDelete);
        virtual void            removeAll();

        virtual void            replaceDataView(CDataDisplay* pDataViewFrom, CDataDisplay* pDataViewTo);

        int                     getDataViewCount() const;
        CDataDisplay*           getDataView(int r, int c);
        QList<CDataDisplay*>    getDataViews() const;
        QPair<int,int>          getFirstFreePosition() const;
        CViewPropertyIO*        getViewProperty();
        int                     getTypeId() const;

        virtual void            setName(QString name);
        void                    setViewProperty(CViewPropertyIO* pView);
        virtual void            setViewSpaceShared(bool bShared);
        void                    setUpdateViewProperty(bool bUpdate);
        void                    setSelectOnClick(bool bSelect);
        virtual void            setSelected(bool bSelected);
        void                    setMaximized(bool bMaximized);

        virtual QSize           minimumSizeHint() const;

        void                    toggleMaximizeView(CDataDisplay* pDataView);
        void                    maximizeView(CDataDisplay* pDataView);

        virtual void            applyViewProperty();

        bool                    isMaximized() const;

        virtual QSize           sizeHint() const;

    signals:

        void                    doClose(CDataDisplay* pDataView);
        void                    doCheck(CDataDisplay* pDataView);
        void                    doUncheck(CDataDisplay* pDataView);
        void                    doToggleMaximize(CDataDisplay* pDataView);
        void                    doMaximize(CDataDisplay* pDataView);
        void                    doDoubleClicked(CDataDisplay* pDataView);
        void                    doSelect(CDataDisplay* pDataView);

    public slots:

        virtual void            onClose();
        virtual void            onChecked(bool bChecked);
        virtual void            onMaximizeClicked();

    protected:

        virtual bool            eventFilter(QObject * obj, QEvent * event) override;
        virtual void            resizeEvent(QResizeEvent* event) override;

        QPushButton*            createButton(const QIcon& icon, const QString& tooltip);

    private:

        void                    initLayout(const QString &name);
        void                    initConnections();

    protected:

        CDataDisplayLayout* m_pLayout = nullptr;
        QVBoxLayout*        m_pVbox = nullptr;
        QHBoxLayout*        m_pHbox = nullptr;        
        QCheckBox*          m_pCheckBox = nullptr;
        QLabel*             m_pLabel = nullptr;
        QPushButton*        m_pCloseBtn = nullptr;
        QPushButton*        m_pMaximizeBtn = nullptr;
        CViewPropertyIO*    m_pViewProperty = nullptr;
        int                 m_flags = NONE;
        int                 m_typeId = DisplayType::EMPTY_DISPLAY;
        bool                m_bIsMaximized = false;
        bool                m_bIsMinimized = false;
        bool                m_bSelectOnClick = false;
        bool                m_bUpdateViewProperty = true;
};

#endif // CDATAVIEW_H
