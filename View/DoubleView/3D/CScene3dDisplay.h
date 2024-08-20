/*
 * Copyright (C) 2023 Ikomia SAS
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

#ifndef CSCENE3DDISPLAY_H
#define CSCENE3DDISPLAY_H

#include <QWidget>

#include "IO/Scene3d/CScene3d.h"
#include "View/DoubleView/CDataDisplay.h"
#include "View/Common/VTK/CVTKViewer.h"

/**
 * @brief The 'CScene3dDisplay' class is used to display a 3D scene provided by the 'CScene3DIO'
 * input/ouput into a result window, using the 'CVTKViewer' class, which is a QT's widget.
 */
class CScene3dDisplay : public CDataDisplay
{
    Q_OBJECT

public:
    /**
     * @brief Default contructor.
     */
    CScene3dDisplay(QWidget* pParent = nullptr);

    /**
     * @brief Set the 3d scene to display.
     */
    void setScene3d(const CScene3d &scene3d);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    /**
     * @brief Initialize layouts and widgets used by this display.
     */
    void  initLayout();

    /**
     * @brief Initialize connections used by this display.
     */
    void  initConnections();

private:
    /**
     * @brief Widget used to display the 3d scene, using the VTK library.
     */
    CVTKViewer* m_pVTKViewer;
};

#endif // CSCENE3DDISPLAY_H
