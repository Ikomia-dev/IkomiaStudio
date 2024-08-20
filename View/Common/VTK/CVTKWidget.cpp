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

#include "CVTKWidget.h"
#include "CVTKCustomInteractorStyle2d.h"
#include "CException.h"
#include "ExceptionCode.hpp"

#include <vtkCamera.h>
#include <vtkNew.h>


CVTKWidget::CVTKWidget(QWidget* parent) :
    QVTKOpenGLWidget(parent),
    m_renderer(vtkNew<vtkRenderer>()),
    m_renderWindow(vtkNew<vtkGenericOpenGLRenderWindow>()),
    m_axesActor(vtkNew<vtkAxesActor>()),
    m_customInteractorStyle2d(vtkNew<CVTKCustomInteractorStyle2d>()),
    m_interactorStyleTrackballCamera(vtkNew<vtkInteractorStyleTrackballCamera>()),
    m_viewMode(CVTKWidgetViewMode::VIEW_3D),
    m_bkgColor{0.0, 0.0, 0.0}
{
    // Display result inside the widget
    m_renderWindow->AddRenderer(m_renderer);
    this->SetRenderWindow(m_renderWindow);
    this->clear();

    // Initialization of the axes (not shown by default)
    m_axesActor->SetTotalLength(1, 1, 1);
    displayAxes(false);
}

void CVTKWidget::forceRender()
{
    // We force the window to be render right now
    m_renderWindow->Render();
}

void CVTKWidget::clear()
{
    m_renderer->RemoveAllViewProps();
    m_renderer->ResetCamera();
    m_renderer->SetBackground(m_bkgColor);
    this->forceRender();
}

void CVTKWidget::resetCamera()
{
    m_renderer->ResetCamera();
    this->forceRender();
}

void CVTKWidget::setOxyOrientation()
{
    double posX, posY, posZ;
    double fpX, fpY, fpZ;

    // We get the current camera's position and the current focal point
    m_renderer->GetActiveCamera( )->GetPosition( posX, posY, posZ );
    m_renderer->GetActiveCamera( )->GetFocalPoint( fpX, fpY, fpZ );

    // We compute the distance between the camera's position and the current focal point
    double distance = std::sqrt( (posX-fpX)*(posX-fpX) + (posY-fpY)*(posY-fpY) + (posZ-fpZ)*(posZ-fpZ) );

    // We set the new camera's properties
    m_renderer->GetActiveCamera( )->SetFocalPoint( 0.0, 0.0, 0.0 );
    m_renderer->GetActiveCamera( )->SetPosition( 0.0, 0.0, distance );
    m_renderer->GetActiveCamera( )->SetViewUp( 0.0, 1.0, 0.0 );

    // The renderer is updated
    this->forceRender();
}

void CVTKWidget::setOxzOrientation()
{
    double posX, posY, posZ;
    double fpX, fpY, fpZ;

    // We get the current camera's position and the current focal point
    m_renderer->GetActiveCamera( )->GetPosition( posX, posY, posZ );
    m_renderer->GetActiveCamera( )->GetFocalPoint( fpX, fpY, fpZ );

    // We compute the distance between the camera's position and the current focal point
    double distance = std::sqrt( (posX-fpX)*(posX-fpX) + (posY-fpY)*(posY-fpY) + (posZ-fpZ)*(posZ-fpZ) );

    // We set the new camera's properties
    m_renderer->GetActiveCamera( )->SetFocalPoint( 0.0, 0.0, 0.0 );
    m_renderer->GetActiveCamera( )->SetPosition( 0.0, -distance, 0.0 );
    m_renderer->GetActiveCamera( )->SetViewUp( 0.0, 0.0, 1.0 );

    // The renderer is updated
    this->forceRender();
}

void CVTKWidget::setOyzOrientation()
{
    double posX, posY, posZ;
    double fpX, fpY, fpZ;

    // We get the current camera's position and the current focal point
    m_renderer->GetActiveCamera( )->GetPosition( posX, posY, posZ );
    m_renderer->GetActiveCamera( )->GetFocalPoint( fpX, fpY, fpZ );

    // We compute the distance between the camera's position and the current focal point
    double distance = std::sqrt( (posX-fpX)*(posX-fpX) + (posY-fpY)*(posY-fpY) + (posZ-fpZ)*(posZ-fpZ) );

    // We set the new camera's properties
    m_renderer->GetActiveCamera( )->SetFocalPoint( 0.0, 0.0, 0.0 );
    m_renderer->GetActiveCamera( )->SetPosition( distance, 0.0, 0.0 );
    m_renderer->GetActiveCamera( )->SetViewUp( 0.0, 1.0, 0.0 );

    // The renderer is updated
    this->forceRender();
}

void CVTKWidget::displayAxes(bool isVisible)
{
    isVisible ?
        m_renderer->AddActor(m_axesActor)
    :
        m_renderer->RemoveActor(m_axesActor);

    this->forceRender();
}

CVTKWidgetViewMode CVTKWidget::getViewMode() const
{
    return m_viewMode;
}

void CVTKWidget::setViewMode(CVTKWidgetViewMode viewMode)
{
    m_viewMode = viewMode;

    switch(m_viewMode)
    {
        case CVTKWidgetViewMode::VIEW_2D:
        {
            this->GetInteractor()->SetInteractorStyle(m_customInteractorStyle2d);
            break;
        }

        case CVTKWidgetViewMode::VIEW_3D:
        {
            this->GetInteractor()->SetInteractorStyle(m_interactorStyleTrackballCamera);
            break;
        }

        default:
        {
            // Invalid view mode !
            throw CException(CoreExCode::INVALID_PARAMETER, "Invalid view mode during the visualization process", __func__, __FILE__, __LINE__);
        }
    }

    // Force the renderer to be redrawn
    this->forceRender();
}

const double* CVTKWidget::getBackgroundColor() const
{
    return m_bkgColor;
}

void CVTKWidget::setBackgroundColor(double bkgColorR, double bkgColorG, double bkgColorB)
{
    // The new background color is stored...
    m_bkgColor[0] = bkgColorR;
    m_bkgColor[1] = bkgColorG;
    m_bkgColor[2] = bkgColorB;

    // ... and the renderer is updated and refreshed
    m_renderer->SetBackground(m_bkgColor);
    this->forceRender();
}

const vtkSmartPointer<vtkRenderer> CVTKWidget::getRenderer() const
{
    return m_renderer;
}
