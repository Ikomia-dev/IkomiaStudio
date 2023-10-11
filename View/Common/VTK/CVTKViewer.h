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

#ifndef CVTKVIEWER_H
#define CVTKVIEWER_H


#include <QtWidgets>

#include "CVTKWidget.h"
#include "IO/CScene3dIO.h"


/**
 * @brief This 'CVTKViewer' class converts a the 3d scene contains in a 'CScene3d' class
 * into a VTK's scene. The result is past to a 'CVTKWidget' component to display the final
 * content into a Qt window.
 */
class CVTKViewer : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor.
     */
    CVTKViewer(QWidget* parent=Q_NULLPTR, Qt::WindowFlags flags=Qt::WindowFlags());

    /**
     * @brief Destructor.
     */
    virtual ~CVTKViewer();

    /**
     * @brief Initialize the widget's layout.
     */
    void initLayout();

    /**
     * @brief Initialize the widget's connections.
     */
    void initConnections();

    /**
     * @brief Convert the given 3D scene into VTK primitives and send the result to the 'CVTKWidget' class.
     * @param scene3d: 3D scene to draw
     */
	void displayScene3d(const CScene3d &scene3d);


public slots:
    void onBtnAxesOxyReleased();
	void onBtnAxesOxzReleased();
	void onBtnAxesOyzReleased();
	void onBtnDisplayAxesReleased();
	void onBtnResetReleased();
	void onCmbDisplayModeCurrentIndexChanged(int index);

protected:
	/**
	 * @brief Main layout of the component.
	 */
	QVBoxLayout *m_pLytMain;

	/**
     * @brief Toolbar's layout.
     */
    QHBoxLayout *m_pLytToolbar;

    /**
     * @brief Button used to put camera in front of the plane (Oxy).
     */
	QPushButton *m_pBtnAxesOxy;

    /**
     * @brief Button used to put camera in front of the plane (Oxz).
     */
	QPushButton *m_pBtnAxesOxz;

    /**
     * @brief Button used to put camera in front of the plane (Oyz).
     */
	QPushButton *m_pBtnAxesOyz;

    /**
     * @brief Button used to show or hide axes.
     */
	QPushButton *m_pBtnDisplayAxes;

    /**
     * @brief Button used to reset the point of view. The new point of view is (0, 0, 0).
     */
	QPushButton *m_pBtnResetView;

    /**
     * @brief Combo-box used to change the view mode (2D/3D).
     */
	QComboBox   *m_pCmbDisplayMode;

    /**
     * @brief Widget used to display the VTK scene into a QT window.
     */
    CVTKWidget  *m_pVTKWidget;
};

#endif   // CVTKVIEWER_H
