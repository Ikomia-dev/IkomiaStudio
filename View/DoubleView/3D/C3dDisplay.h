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

#ifndef C3DDISPLAY_H
#define C3DDISPLAY_H

/**
 * @file      C3dDisplay.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including C3dDisplay definition
 *
 * @details   Details
 */

#include "../CDataDisplay.h"
#include "View/Common/CGLWidget.h"

class C3dAnimationDlg;

/**
 * @brief The C3dDisplay class for displaying 3D data using OpenGL
 */
class C3dDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        C3dDisplay(QWidget* parent = nullptr, const QString& name = QString());

        bool            eventFilter(QObject *obj, QEvent *event) override;

        void            initLayout();
        void            initConnections();
        void            initToolBar();
        void            initSettings();
        void            initGL();

        void            changeVolume();
        void            updateRenderVolume();

        CGLWidget*      getGLWidget();

    signals:

        void            doUpdateRenderMode(size_t mode);
        void            doUpdateColormap(size_t colormap);
        void            doUpdateParam(size_t type, float value);

    public slots:

        void            onUseRaycasting();
        void            onUseMIP();
        void            onUseMinIP();
        void            onUseIso();
        void            onUpdateOffset(int value);
        void            onExportScreenshot();
        void            onExportAnimation();

    private:

        QPushButton*    createToolbarBtn(const QString &tooltip, const QIcon &icon);

    private:

        CGLWidget*          m_pGLWidget = nullptr;
        QPushButton*        m_pRaycastBtn = nullptr;
        QPushButton*        m_pMIPBtn = nullptr;
        QPushButton*        m_pMinIPBtn = nullptr;
        QPushButton*        m_pIsoBtn = nullptr;
        QPushButton*        m_pSettingsBtn = nullptr;
        QPushButton*        m_pScreenshotBtn = nullptr;
        QPushButton*        m_pAnimationBtn = nullptr;
        QComboBox*          m_pColorMapBtn = nullptr;
        QWidget*            m_pToolbar = nullptr;
        QVBoxLayout*        m_pVBoxLayout = nullptr;
        QWidget*            m_pSettings = nullptr;
        C3dAnimationDlg*    m_pAnimationDlg = nullptr;
};

#endif // C3DDISPLAY_H
