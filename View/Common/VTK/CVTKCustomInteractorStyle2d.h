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

#ifndef CVTKCUSTOMINTERACTORSTYLE2D_H
#define CVTKCUSTOMINTERACTORSTYLE2D_H

#include <QObject>
#include <vtkInteractorStyleImage.h>


/**
 * VTK style used to interact with a 2D view.
 *
 * The left button is used to pick pixels
 * The wheel is used to zoom
 * The right button is used to move inside the image
 * (like the middle button in the parent's style)
 */
class CVTKCustomInteractorStyle2d : public vtkInteractorStyleImage
{
    public:

        static CVTKCustomInteractorStyle2d* New();
        vtkTypeMacro(CVTKCustomInteractorStyle2d, vtkInteractorStyleImage)


        // The left button has the same behavior as the parent's middle button behavior
        void OnLeftButtonDown() override;
        void OnLeftButtonUp() override;

        // These methods are overloaded to disable the default behavior
        void OnMiddleButtonDown() override;
        void OnMiddleButtonUp() override;

        // These methods are overloaded to disable the default behavior
        void OnRightButtonDown() override;
        void OnRightButtonUp() override;
};

#endif // CVTKCUSTOMINTERACTORSTYLE2D_H
