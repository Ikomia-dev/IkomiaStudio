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

#ifndef CVTKWIGDET_H
#define CVTKWIGDET_H


#include <cstdint>
#include <QVTKOpenGLWidget.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkAxesActor.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include "CVTKCustomInteractorStyle2d.h"


/**
 * Enumeration used to characterize the current view mode:
 * - 3D view
 * - 2D view
 */
enum class CVTKWidgetViewMode : std::uint8_t
{
    VIEW_3D = 1,
    VIEW_2D = 2,
};


/**
 * @brief The CVTKWidget class initializes an OpenGL widget for
 * displaying 3D view using the VTK library.
 */
class CVTKWidget : public QVTKOpenGLWidget
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor.
     * @param parent: Qt's parent window.
     */
    CVTKWidget(QWidget *parent=Q_NULLPTR);

    /**
     * @brief Force the current renderer to be redrawn.
     */
    void forceRender();

    /**
     * @brief Clear the current scene and reset the camera properties.
     */
    void clear();

    /**
     * @brief Set up the camera based on the visible content of the 3D scene.
     * The new point of view (POV) will be the center of the visible content
     * and the distance between the camera and this POV will be ajusted so that
     * the whole scene can be seen.
     */
    void resetCamera();

    /**
     * @brief Set the focal point to the origin and change the
     * camera's orientation so that the camera is in front of
     * the (Oxy) plane. The current distance between the focal
     * point and the camera is not modified.
     */
    void setOxyOrientation();

    /**
     * @brief Set the focal point to the origin and change the
     * camera's orientation so that the camera is in front of
     * the (Oxz) plane. The current distance between the focal
     * point and the camera is not modified.
     */
    void setOxzOrientation();

    /**
     * @brief Set the focal point to the origin and change the
     * camera's orientation so that the camera is in front of
     * the (Oyz) plane. The current distance between the focal
     * point and the camera is not modified.
     */
    void setOyzOrientation();

    /**
     * @brief Show / hide the VTK's axes.
     * @param isVisible: true if the axes must be shown, false if they must be hidden.
     */
    void displayAxes(bool isVisible);

    /**
     * @brief Return the current view mode (2D, 3D...).
     */
    CVTKWidgetViewMode getViewMode() const;

    /**
     * @brief Change the current view mode.
     * @param viewMode: the new view mode.
     */
    void setViewMode(CVTKWidgetViewMode viewMode);

    /**
     * @brief Return the background color of the 3D scene (defined
     * in the renderer).
     */
    const double* getBackgroundColor() const;

    /**
     * @brief Set the background color of the 3D scene (defined
     * in the renderer).
     */
    void setBackgroundColor(double bkgColorR, double bkgColorG, double bkgColorB);

    /**
     * @brief Return a reference onto the VTK's renderer.
     */
    const vtkSmartPointer<vtkRenderer> getRenderer() const;

protected:
    /**
     * @brief VTK's renderer containing the whole 3D scene.
     */
    vtkSmartPointer<vtkRenderer> m_renderer;

    /**
     * @brief VTK's window, used to display the renderer.
     */
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;

    /**
     * @brief VTK's actor used to store axes.
     * Axes can be displayed or hidden using the 'displayAxes()' method.
     */
    vtkSmartPointer<vtkAxesActor> m_axesActor;

    /**
     * @brief Custom interactor used to control the camera when the
     * view mode is set to 2D.
     */
    vtkSmartPointer<CVTKCustomInteractorStyle2d> m_customInteractorStyle2d;

    /**
     * @brief VTK's interactor used to control the camera when the
     * view mode is set to 3D.
     */
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_interactorStyleTrackballCamera;

    /**
     * @brief Current view mode (2D, 3D...).
     */
    CVTKWidgetViewMode m_viewMode;

    /**
     * @brief Background color (RGB).
     */
    double m_bkgColor[3];
};

#endif // CVTKWIGDET_H
