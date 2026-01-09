#include "CVTKCustomInteractorStyle2d.h"

#include <vtkInteractorStyleImage.h>
#include <vtkObjectFactory.h>


// Initialization of the new component
vtkStandardNewMacro(CVTKCustomInteractorStyle2d)

// The left button has the same behavior as the parent's middle button behavior
void CVTKCustomInteractorStyle2d::OnLeftButtonDown()
{
    vtkInteractorStyleImage::OnMiddleButtonDown();
}

// The left button has the same behavior as the parent's middle button behavior
void CVTKCustomInteractorStyle2d::OnLeftButtonUp()
{
    vtkInteractorStyleImage::OnMiddleButtonUp();
}

// This method is overloaded to disable the default behavior
void CVTKCustomInteractorStyle2d::OnMiddleButtonDown()
{ }

// This method is overloaded to disable the default behavior
void CVTKCustomInteractorStyle2d::OnMiddleButtonUp()
{ }

// This method is overloaded to disable the default behavior
void CVTKCustomInteractorStyle2d::OnRightButtonDown()
{ }

// This method is overloaded to disable the default behavior
void CVTKCustomInteractorStyle2d::OnRightButtonUp()
{ }
