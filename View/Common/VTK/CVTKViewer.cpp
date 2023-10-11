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

#include "CVTKViewer.h"

#include "IO/Scene3d/CScene3dImage2d.h"
#include "IO/Scene3d/CScene3dShapeCircle.h"
#include "IO/Scene3d/CScene3dShapePoint.h"
#include "IO/Scene3d/CScene3dShapePoly.h"
#include "CException.h"
#include "ExceptionCode.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSliceMapper.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyLine.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>


CVTKViewer::CVTKViewer(QWidget* parent, Qt::WindowFlags flags) :
	QWidget(parent, flags),
    m_pLytMain(nullptr),
    m_pLytToolbar(nullptr),
    m_pBtnAxesOxy(nullptr),
    m_pBtnAxesOxz(nullptr),
    m_pBtnAxesOyz(nullptr),
    m_pBtnDisplayAxes(nullptr),
    m_pBtnResetView(nullptr),
    m_pCmbDisplayMode(nullptr),
	m_pVTKWidget(nullptr)
{
    // Initialization of the layout's widgets
    initLayout();

    // Initialization of the layout's connections
    initConnections();
}

CVTKViewer::~CVTKViewer()
{
	if(m_pVTKWidget)
	{
		delete m_pVTKWidget;
		m_pVTKWidget = nullptr;
	}
}

void CVTKViewer::initLayout()
{
    // Initialization of the widget used to display a VTK scene inside a QT window
    m_pVTKWidget = new CVTKWidget(this);

    // This widget must take as more spaces as possible
    m_pVTKWidget->setSizePolicy(
        QSizePolicy(
            QSizePolicy::Expanding,
            QSizePolicy::Expanding
        )
    );


    // Initialization of the toolbar's components
    m_pBtnAxesOxy = new QPushButton("Oxy", this);
    m_pBtnAxesOxz = new QPushButton("Oxz", this);
    m_pBtnAxesOyz = new QPushButton("Oyz", this);

    m_pBtnDisplayAxes = new QPushButton("Axes", this);
    m_pBtnDisplayAxes->setCheckable(true);

    m_pBtnResetView = new QPushButton("Reset", this);

    m_pCmbDisplayMode = new QComboBox(this);
    m_pCmbDisplayMode->addItem(tr("2D"), QVariant(static_cast<unsigned int>(CVTKWidgetViewMode::VIEW_2D)));
    m_pCmbDisplayMode->addItem(tr("3D"), QVariant(static_cast<unsigned int>(CVTKWidgetViewMode::VIEW_3D)));
    m_pCmbDisplayMode->setCurrentIndex(1);


    // Creation of the toolbar
    m_pLytToolbar = new QHBoxLayout();
    m_pLytToolbar->addWidget(m_pBtnAxesOxy);
    m_pLytToolbar->addWidget(m_pBtnAxesOxz);
    m_pLytToolbar->addWidget(m_pBtnAxesOyz);
    m_pLytToolbar->addWidget(m_pBtnDisplayAxes);
    m_pLytToolbar->addWidget(m_pBtnResetView);
    m_pLytToolbar->addWidget(m_pCmbDisplayMode);


    // Creation of the main layout of this component
    m_pLytMain = new QVBoxLayout();
    m_pLytMain->addLayout(m_pLytToolbar);
    m_pLytMain->addWidget(m_pVTKWidget);

    this->setLayout(m_pLytMain);


    m_pVTKWidget->setBackgroundColor(70.0/255.0, 70.0/255.0, 70.0/255.0);
    m_pVTKWidget->displayAxes(m_pBtnDisplayAxes->isChecked());
}

void CVTKViewer::initConnections()
{
    // Creation of the connections used by the toolbar's components
    connect(m_pBtnAxesOxy,                        &QPushButton::released,           this, &CVTKViewer::onBtnAxesOxyReleased);
    connect(m_pBtnAxesOxz,                        &QPushButton::released,           this, &CVTKViewer::onBtnAxesOxzReleased);
    connect(m_pBtnAxesOyz,                        &QPushButton::released,           this, &CVTKViewer::onBtnAxesOyzReleased);
    connect(m_pBtnDisplayAxes,                    &QPushButton::released,           this, &CVTKViewer::onBtnDisplayAxesReleased);
    connect(m_pBtnResetView,                      &QPushButton::released,           this, &CVTKViewer::onBtnResetReleased);
    connect(m_pCmbDisplayMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CVTKViewer::onCmbDisplayModeCurrentIndexChanged);
}

void CVTKViewer::displayScene3d(const CScene3d &scene3d)
{
    const int CIRCLE_NB_SIDES = 50;
    const double CIRCLE_LINE_WIDTH = 1.0;

    // The whole scene is cleared
    m_pVTKWidget->clear();



    // Array used to store positions of the points (= 'CScene3dShapePoint')
    vtkNew<vtkPoints> lstShapePointsCenter;

    // Array used to store colors of the points (= 'CScene3dShapePoint')
    vtkNew<vtkUnsignedCharArray> lstShapePointsColorArray;
    lstShapePointsColorArray->SetName("colorArray");
    lstShapePointsColorArray->SetNumberOfComponents(3);

    // Array used to store sizes of the points (= 'CScene3dShapePoint')
    vtkNew<vtkDoubleArray> lstShapePointsScaleArray;
    lstShapePointsScaleArray->SetNumberOfComponents(3);
    lstShapePointsScaleArray->SetName("scaleArray");


    // Array used to store positions of the circles (= 'CScene3dShapeCircle')
    vtkNew<vtkPoints> lstShapeCirclesCenter;

    // Array used to store colors of the circles (= 'CScene3dShapeCircle')
    vtkNew<vtkUnsignedCharArray> lstShapeCirclesColorArray;
    lstShapeCirclesColorArray->SetName("colorArray");
    lstShapeCirclesColorArray->SetNumberOfComponents(3);

    // Array used to store sizes of the circles (= 'CScene3dShapeCircle')
    vtkNew<vtkDoubleArray> lstShapeCirclesScaleArray;
    lstShapeCirclesScaleArray->SetNumberOfComponents(3);
    lstShapeCirclesScaleArray->SetName("scaleArray");


    // Array used to store positions of the polygons (= 'CScene3dShapePoly')
    vtkNew<vtkPoints> lstShapePolysPoint;

    // Array used to store colors of the polygons (= 'CScene3dShapePoly')
    vtkNew<vtkUnsignedCharArray> lstShapePolysColorArray;
    lstShapePolysColorArray->SetName("colorArray");
    lstShapePolysColorArray->SetNumberOfComponents(3);

    // Array used to store connections between points of the polygons (= 'CScene3dShapePoly')
    vtkNew<vtkCellArray> lstShapePolysLines;


    // Position of the image 2d coordinate system's origin
    CScene3dCoord sceneOrigin = scene3d.getImage2dCoordSystemOrigin();


    // For each layer in the 3D scene...
    for(auto layer: scene3d.getLstLayers())
    {
        // if this layer is visible...
        if(layer.isVisible())
        {
            // we iterate through its objects...
            for(auto x: layer.getLstObjects())
            {
                // and if these objects are visibles...
                if(x->isVisible())
                {
                    // we look for the derived kind of these objects (points, circles, polygons, images...)
                    if (std::dynamic_pointer_cast<CScene3dImage2d>(x)) {

                        // The current object is a 'CScene3dImage2d'
                        auto obj = std::dynamic_pointer_cast<CScene3dImage2d>(x);

                        // We copy the original data into a 'vtkImageData'
                        vtkNew<vtkImageData> imageData;
                        imageData->SetDimensions(obj->getWidth(), obj->getHeight(), 1);
                        imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

                        unsigned char* start = static_cast<unsigned char*>(imageData->GetScalarPointer(0,0,0));
                        std::copy(obj->getData().data, obj->getData().data+obj->getWidth()*obj->getHeight(), start);
                        imageData->Modified();


                        vtkNew<vtkImageSliceMapper> imageSliceMapper;
                        imageSliceMapper->SetInputData(imageData);
                        // To draw the full border pixels, not half-pixels
                        imageSliceMapper->BorderOn();


                        vtkNew<vtkImageProperty> imageProperty;
                        // To avoid smoothing between pixels (display true color of a pixel onto it whole surface)
                        imageProperty->SetInterpolationTypeToNearest();

                        vtkNew<vtkImageActor> imageActor;
                        imageActor->SetMapper(imageSliceMapper);
                        imageActor->SetProperty(imageProperty);


                        // Put the current image into the real coordinate system
                        vtkNew<vtkTransform> transform;
                        transform->PostMultiply(); // Each transformation will occur after those already defined
                        transform->RotateX(180.0);
                        transform->Translate(
                            sceneOrigin.getCoordX1(),
                            sceneOrigin.getCoordX2(),
                            sceneOrigin.getCoordX3()
                        );

                        imageActor->SetUserTransform(transform);
                        m_pVTKWidget->getRenderer()->AddActor(imageActor);

                    } else if (std::dynamic_pointer_cast<CScene3dShapePoint>(x)) {

                        // The current object is a 'CScene3dShapePoint'
                        auto obj = std::dynamic_pointer_cast<CScene3dShapePoint>(x);

                        CScene3dCoord objPosition = obj->getPosition();

                        if(objPosition.getCoordSystem() == CScene3dCoordSystem::IMAGE)
                        {
                            // The center's coordinates are defined in an image coordinate system: transformation is required,
                            // according to the scene properties
                            lstShapePointsCenter->InsertNextPoint(
                                sceneOrigin.getCoordX1() + objPosition.getCoordX1(),
                                sceneOrigin.getCoordX2() - objPosition.getCoordX2(),
                                sceneOrigin.getCoordX3() + objPosition.getCoordX3()
                            );
                        }
                        else
                        {
                            // The center's coordinates are defined in the real coordinate system or the
                            // coordinate system is not defined: no transformation is required
                            lstShapePointsCenter->InsertNextPoint(
                                objPosition.getCoordX1(),
                                objPosition.getCoordX2(),
                                objPosition.getCoordX3()
                            );
                        }

                        lstShapePointsColorArray->InsertNextTuple3(
                            obj->getColor().getScaledColorR(0.0, 255.0),
                            obj->getColor().getScaledColorG(0.0, 255.0),
                            obj->getColor().getScaledColorB(0.0, 255.0)
                        );

                        lstShapePointsScaleArray->InsertNextTuple3(
                            obj->getSize(),
                            obj->getSize(),
                            obj->getSize()
                        );

                    } else if (std::dynamic_pointer_cast<CScene3dShapeCircle>(x)) {

                        // The current object is a 'CScene3dShapeCircle'
                        auto obj = std::dynamic_pointer_cast<CScene3dShapeCircle>(x);

                        CScene3dCoord objCenter = obj->getCenter();

                        if(objCenter.getCoordSystem() == CScene3dCoordSystem::IMAGE)
                        {
                            // The center's coordinates are defined in an image coordinate system: transformation is required,
                            // according to the scene properties
                            lstShapeCirclesCenter->InsertNextPoint(
                                sceneOrigin.getCoordX1() + objCenter.getCoordX1(),
                                sceneOrigin.getCoordX2() - objCenter.getCoordX2(),
                                sceneOrigin.getCoordX3() + objCenter.getCoordX3()
                            );
                        }
                        else
                        {
                            // The center's coordinates are defined in the real coordinate system or the
                            // coordinate system is not defined: no transformation is required
                            lstShapeCirclesCenter->InsertNextPoint(
                                objCenter.getCoordX1(),
                                objCenter.getCoordX2(),
                                objCenter.getCoordX3()
                            );
                        }

                        lstShapeCirclesColorArray->InsertNextTuple3(
                            obj->getColor().getScaledColorR(0.0, 255.0),
                            obj->getColor().getScaledColorG(0.0, 255.0),
                            obj->getColor().getScaledColorB(0.0, 255.0)
                        );

                        lstShapeCirclesScaleArray->InsertNextTuple3(
                            obj->getRadius()*2.0,
                            obj->getRadius()*2.0,
                            obj->getRadius()*2.0
                        );

                    } else if (std::dynamic_pointer_cast<CScene3dShapePoly>(x)) {

                        // The current object is a 'CScene3dShapePoly'
                        auto obj = std::dynamic_pointer_cast<CScene3dShapePoly>(x);

                        std::vector<CScene3dPt> lstPts = obj->getLstPts();

                        vtkNew<vtkPolyLine> polyLine;
                        polyLine->GetPointIds()->SetNumberOfIds(lstPts.size()+1);
                        vtkIdType firstId = 0;
                        vtkIdType id;

                        for(int i = 0; i < lstPts.size(); ++i)
                        {
                            CScene3dPt pt = lstPts[i];
                            CScene3dCoord coord = std::get<0>(pt);
                            CScene3dColor color = std::get<1>(pt);

                            if(coord.getCoordSystem() == CScene3dCoordSystem::IMAGE)
                            {
                                // The center's coordinates are defined in an image coordinate system: transformation is required,
                                // according to the scene properties
                                id = lstShapePolysPoint->InsertNextPoint(
                                    sceneOrigin.getCoordX1() + coord.getCoordX1(),
                                    sceneOrigin.getCoordX2() - coord.getCoordX2(),
                                    sceneOrigin.getCoordX3() + coord.getCoordX3()
                                );
                            }
                            else
                            {
                                // The center's coordinates are defined in the real coordinate system or the
                                // coordinate system is not defined: no transformation is required
                                id = lstShapePolysPoint->InsertNextPoint(
                                    coord.getCoordX1(),
                                    coord.getCoordX2(),
                                    coord.getCoordX3()
                                );
                            }


                            // We store the vtkId of the first point
                            // This id will be used to close the polygon
                            if(i == 0)
                            {
                                firstId = id;
                            }

                            polyLine->GetPointIds()->SetId(i, id);

                            lstShapePolysColorArray->InsertNextTuple3(
                                color.getScaledColorR(0.0, 255.0),
                                color.getScaledColorG(0.0, 255.0),
                                color.getScaledColorB(0.0, 255.0)
                            );
                        }

                        // We close the polygon by adding the first point of the polygon
                        // at the end of the list
                        polyLine->GetPointIds()->SetId(lstPts.size(), firstId);

                        lstShapePolysLines->InsertNextCell(polyLine);

                    } else {
                        throw CException(CoreExCode::INVALID_PARAMETER, "Object type unknown during the computation of the 3D scene", __func__, __FILE__, __LINE__);
                    }
                }
            }
        }
    }


    ///////////////////////////////////////////////////////////////////////
    //                                                                   //
    //          Display the list of points 'C3dSceneShapePoint'          //
    //                                                                   //
    ///////////////////////////////////////////////////////////////////////

    if(lstShapePointsCenter->GetNumberOfPoints() >= 1)
    {
        vtkNew<vtkPolyData> lstShapePointsPolydata;
        lstShapePointsPolydata->SetPoints(lstShapePointsCenter);
        lstShapePointsPolydata->GetPointData()->AddArray(lstShapePointsColorArray);
        lstShapePointsPolydata->GetPointData()->AddArray(lstShapePointsScaleArray);

        vtkNew<vtkRegularPolygonSource> shapePointPolygonSource;
        shapePointPolygonSource->SetNumberOfSides(12);
        shapePointPolygonSource->Update();

        vtkNew<vtkGlyph3DMapper> lstShapePointsGlyph3Dmapper;
        lstShapePointsGlyph3Dmapper->SetSourceConnection(shapePointPolygonSource->GetOutputPort());
        lstShapePointsGlyph3Dmapper->SetInputData(lstShapePointsPolydata);
        lstShapePointsGlyph3Dmapper->SetScalarModeToUsePointFieldData();
        lstShapePointsGlyph3Dmapper->SetScaleModeToScaleByVectorComponents();
        lstShapePointsGlyph3Dmapper->SetScaleArray("scaleArray");
        lstShapePointsGlyph3Dmapper->SelectColorArray("colorArray");
        lstShapePointsGlyph3Dmapper->Update();

        vtkNew<vtkActor> lstShapePointsActor;
        lstShapePointsActor->SetMapper(lstShapePointsGlyph3Dmapper);

        m_pVTKWidget->getRenderer()->AddActor(lstShapePointsActor);
    }


    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    //          Display the list of circles 'C3dSceneShapeCircle'          //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    if(lstShapeCirclesCenter->GetNumberOfPoints() >= 1)
    {
        vtkNew<vtkPolyData> lstShapeCirclesPolydata;
        lstShapeCirclesPolydata->SetPoints(lstShapeCirclesCenter);
        lstShapeCirclesPolydata->GetPointData()->AddArray(lstShapeCirclesColorArray);
        lstShapeCirclesPolydata->GetPointData()->AddArray(lstShapeCirclesScaleArray);

        vtkNew<vtkRegularPolygonSource> shapeCirclePolygonSource;
        shapeCirclePolygonSource->SetNumberOfSides(CIRCLE_NB_SIDES);
        shapeCirclePolygonSource->GeneratePolygonOff();
        shapeCirclePolygonSource->GeneratePolylineOn();
        shapeCirclePolygonSource->Update();

        vtkNew<vtkGlyph3DMapper> lstShapeCirclesGlyph3Dmapper;
        lstShapeCirclesGlyph3Dmapper->SetSourceConnection(shapeCirclePolygonSource->GetOutputPort());
        lstShapeCirclesGlyph3Dmapper->SetInputData(lstShapeCirclesPolydata);
        lstShapeCirclesGlyph3Dmapper->SetScalarModeToUsePointFieldData();
        lstShapeCirclesGlyph3Dmapper->SetScaleModeToScaleByVectorComponents();
        lstShapeCirclesGlyph3Dmapper->SetScaleArray("scaleArray");
        lstShapeCirclesGlyph3Dmapper->SelectColorArray("colorArray");
        lstShapeCirclesGlyph3Dmapper->Update();

        vtkNew<vtkActor> lstShapeCirclesActor;
        lstShapeCirclesActor->SetMapper(lstShapeCirclesGlyph3Dmapper);

        m_pVTKWidget->getRenderer()->AddActor(lstShapeCirclesActor);
    }


    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    //          Display the list of polygons 'C3dSceneShapePoly'          //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if(lstShapePolysPoint->GetNumberOfPoints() >= 1)
    {
        vtkNew<vtkPolyData> lstShapePolysPolyData;
        lstShapePolysPolyData->SetPoints(lstShapePolysPoint);
        lstShapePolysPolyData->SetLines(lstShapePolysLines);
        lstShapePolysPolyData->GetPointData()->AddArray(lstShapePolysColorArray);

        vtkNew<vtkPolyDataMapper> lstShapePolysMapper;
        lstShapePolysMapper->SetInputData(lstShapePolysPolyData);
        lstShapePolysMapper->SetScalarModeToUsePointFieldData();
        lstShapePolysMapper->SelectColorArray("colorArray");
        lstShapePolysMapper->Update();

        vtkNew<vtkActor> lstShapePolysActor;
        lstShapePolysActor->SetMapper(lstShapePolysMapper);

        m_pVTKWidget->getRenderer()->AddActor(lstShapePolysActor);
    }


    // The new camera's orientation is updated so that
    // the focal point is the center of the new scene
    m_pVTKWidget->resetCamera();
}

void CVTKViewer::onBtnAxesOxyReleased()
{
    m_pVTKWidget->setOxyOrientation();
}

void CVTKViewer::onBtnAxesOxzReleased()
{
	m_pVTKWidget->setOxzOrientation();
}

void CVTKViewer::onBtnAxesOyzReleased()
{
    m_pVTKWidget->setOyzOrientation();
}

void CVTKViewer::onBtnDisplayAxesReleased()
{
    m_pVTKWidget->displayAxes(m_pBtnDisplayAxes->isChecked());
}

void CVTKViewer::onBtnResetReleased()
{
    m_pVTKWidget->resetCamera();
}

void CVTKViewer::onCmbDisplayModeCurrentIndexChanged(int index)
{
    // Boolean used to know if conversion has been made with success
    bool ok = false;

    // The new display mode is get and converted into a unsigned int
    QVariant data = m_pCmbDisplayMode->currentData();
    unsigned int value = data.toUInt(&ok);

    // Does the conversion success ?
    if(ok)
    {
        // Yes: the view mode is changed
        m_pVTKWidget->setViewMode(
            static_cast<CVTKWidgetViewMode>(value)
        );
    }
    else
    {
        // No: an exception is thrown
        throw CException(CoreExCode::INVALID_PARAMETER, "Invalid view mode", __func__, __FILE__, __LINE__);
    }
}
