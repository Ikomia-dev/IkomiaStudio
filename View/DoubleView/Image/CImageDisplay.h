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

#ifndef CIMAGEDISPLAY_H
#define CIMAGEDISPLAY_H

/**
 * @file      CImageDisplay.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CImageDisplay definition
 *
 * @details   Details
 */

#include "View/DoubleView/CDataDisplay.h"
#include "CImageView.h"

class QSlider;
class QSpinBox;

class CImageDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        enum ImageDisplayBar
        {
            NONE = 0x00000000,
            CHECKBOX = 0x00000001,
            TITLE = 0x00000002,
            MAXIMIZE_BUTTON = 0x0000004,
            CLOSE_BUTTON = 0x0000008,
            ZOOM_BUTTONS = 0x00000010,
            SLIDER = 0x00000020,
            SAVE_BUTTON = 0x00000040,
            EXPORT_BUTTON = 0x00000080,
            GRAPHICS_BUTTON = 0x00000100,
            DEFAULT = TITLE | EXPORT_BUTTON | ZOOM_BUTTONS | MAXIMIZE_BUTTON,
            DEFAULT_3D = TITLE | ZOOM_BUTTONS | SLIDER
        };

        CImageDisplay(QWidget* pParent=nullptr, QString name = "", int flags=DEFAULT);

        void            setImage(const QImage& image, bool bZoomFit = false);
        void            setImage(CImageScene *pScene, const QImage& image, bool bZoomFit = false);
        void            setToolbarSpacing(int size);
        void            setSliderRange(size_t max);
        void            setCurrentImgIndex(size_t index);
        virtual void    setViewSpaceShared(bool bShared) override;
        virtual void    setSelected(bool bSelect) override;

        CImageView*     getView() const;

        void            fitInView();

        virtual void    applyViewProperty() override;
        void            applyZoomViewProperty();

        void            enableImageMoveByKey(bool bEnable);

    signals:

        void            doUpdateIndex(int index);
        void            doSave();
        void            doExport(const QString& path, bool bWithGraphics);
        void            doToggleGraphicsToolbar();

    public slots:

        void            onUpdateIndex(int index);
        void            onZoomFit();
        void            onZoomIn();
        void            onZoomOut();
        void            onZoomOriginal();
        void            onExportBtnClicked();

    private slots:

        void            onSliderChanged(int index);

    protected:

        virtual bool    eventFilter(QObject *obj, QEvent *event) override;
        virtual void    resizeEvent(QResizeEvent* event) override;
        virtual void    keyPressEvent(QKeyEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

        void            showMenu(const QPoint& pos);

    private:

        CImageView*     m_pView = nullptr;
        QSlider*        m_pSlider = nullptr;
        QSpinBox*       m_pSpinBox = nullptr;
        QPushButton*    m_pZoomInBtn = nullptr;
        QPushButton*    m_pZoomOutBtn = nullptr;
        QPushButton*    m_pZoomFitBtn = nullptr;
        QPushButton*    m_pZoomOriginalBtn = nullptr;
        QPushButton*    m_pSaveBtn = nullptr;
        QPushButton*    m_pExportBtn = nullptr;
        QPushButton*    m_pGraphicsBtn = nullptr;
        int             m_toolbarSpacing = 10;
        bool            m_bMoveByKey = true;
};

#endif // CIMAGEDISPLAY_H
