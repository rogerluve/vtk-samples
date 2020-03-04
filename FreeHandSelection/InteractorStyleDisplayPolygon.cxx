#include "InteractorStyleDisplayPolygon.h"

#include "vtkCommand.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnsignedCharArray.h"
#include "vtkVectorOperators.h"
#include "vtkPoints.h"
#include "vtkPolygon.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkTriangleFilter.h"
#include "vtkProperty.h"

namespace 
{
	const int RESOLUTION = 5000;
}

vtkStandardNewMacro(InteractorStyleDisplayPolygon);

//-----------------------------------------------------------------------------
class InteractorStyleDisplayPolygon::vtkInternal
{
public:
	vtkInternal()
	{
			this->worldPositionPoints = vtkSmartPointer<vtkPoints>::New();
			this->polygon = vtkSmartPointer<vtkPolygon>::New();
			this->polygons = vtkSmartPointer<vtkCellArray>::New();
			this->polydata = vtkSmartPointer<vtkPolyData>::New();
	}

	std::vector<vtkVector2i> points;

	void AddPoint(const vtkVector2i &point)
	{
		this->points.push_back(point);
	}

	void AddPoint(int x, int y)
	{
		this->AddPoint(vtkVector2i(x, y));
	}

	vtkVector2i GetPoint(vtkIdType index) const
	{
		return this->points[index];
	}

	vtkIdType GetNumberOfPoints() const
	{
		return static_cast<vtkIdType>(this->points.size());
	}

	void Clear()
	{
		this->points.clear();
		this->worldPositionPoints->Reset();
	}

	void DrawPixels(const vtkVector2i& StartPos, const vtkVector2i& EndPos, unsigned char *pixels, int *size)
	{
		int x1 = StartPos.GetX(), x2 = EndPos.GetX();
		int y1 = StartPos.GetY(), y2 = EndPos.GetY();

		double x = x2 - x1;
		double y = y2 - y1;
		double length = sqrt(x*x + y * y);
		if (length == 0)
		{
			return;
		}
		double addx = x / length;
		double addy = y / length;

		x = x1;
		y = y1;
		int row, col;
		for (double i = 0; i < length; i += 1)
		{
			col = (int)x;
			row = (int)y;
			pixels[3 * (row*size[0] + col)] = 255 ^ pixels[3 * (row*size[0] + col)];
			pixels[3 * (row*size[0] + col) + 1] = 255 ^ pixels[3 * (row*size[0] + col) + 1];
			pixels[3 * (row*size[0] + col) + 2] = 255 ^ pixels[3 * (row*size[0] + col) + 2];
			x += addx;
			y += addy;
		}
	}

	vtkSmartPointer<vtkPoints> worldPositionPoints;
	vtkSmartPointer<vtkPolygon> polygon;
	vtkSmartPointer<vtkCellArray> polygons;
	vtkSmartPointer<vtkPolyData> polydata;
	vtkSmartPointer<vtkTriangleFilter> tri;
	vtkSmartPointer<vtkDataSetMapper> polygonMapper;
	vtkSmartPointer<vtkActor> polygonActor;
};

//----------------------------------------------------------------------------
InteractorStyleDisplayPolygon::InteractorStyleDisplayPolygon()
{
	this->Internal = new vtkInternal();
	this->StartPosition[0] = this->StartPosition[1] = 0;
	this->EndPosition[0] = this->EndPosition[1] = 0;
	this->Moving = 0;
	this->DisplayPolygon = true;

	// to graphics primitives
	this->Internal->tri = vtkSmartPointer<vtkTriangleFilter>::New();
	this->Internal->tri->SetInputData(this->Internal->polydata);

	this->Internal->polygonMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	this->Internal->polygonMapper->SetInputConnection(this->Internal->tri->GetOutputPort());
	this->Internal->polygonMapper->Update();

	this->Internal->polygonActor = vtkSmartPointer<vtkActor>::New();
	this->Internal->polygonActor->SetMapper(this->Internal->polygonMapper);
	this->Internal->polygonActor->GetProperty()->SetEdgeColor(0.1, 0.3, 0.3);
	this->Internal->polygonActor->GetProperty()->SetLineWidth(3);
	this->Internal->polygonActor->GetProperty()->SetOpacity(0.2);
	this->Internal->polygonActor->GetProperty()->SetColor(1, 0, 0);
}

//----------------------------------------------------------------------------
InteractorStyleDisplayPolygon::~InteractorStyleDisplayPolygon()
{
	delete this->Internal;
}

//----------------------------------------------------------------------------
std::vector<vtkVector2i> InteractorStyleDisplayPolygon::GetPolygonPointsDisplayUnits()
{
	return this->Internal->points;
}

//----------------------------------------------------------------------------
vtkPoints* InteractorStyleDisplayPolygon::GetPolygonPoints()
{
	return this->Internal->worldPositionPoints;
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::OnMouseMove()
{
	if (!this->Interactor || !this->Moving)
	{
		return;
	}

	this->EndPosition[0] = this->Interactor->GetEventPosition()[0];
	this->EndPosition[1] = this->Interactor->GetEventPosition()[1];
	int *size = this->Interactor->GetRenderWindow()->GetSize();
	if (this->EndPosition[0] > (size[0] - 1))
	{
		this->EndPosition[0] = size[0] - 1;
	}
	if (this->EndPosition[0] < 0)
	{
		this->EndPosition[0] = 0;
	}
	if (this->EndPosition[1] > (size[1] - 1))
	{
		this->EndPosition[1] = size[1] - 1;
	}
	if (this->EndPosition[1] < 0)
	{
		this->EndPosition[1] = 0;
	}

	vtkVector2i lastPoint = this->Internal->GetPoint(this->Internal->GetNumberOfPoints() - 1);
	vtkVector2i newPoint(this->EndPosition[0], this->EndPosition[1]);
	if ((lastPoint - newPoint).SquaredNorm() > RESOLUTION)
	{
		this->Internal->AddPoint(newPoint);
		this->insertNewPointOnEventPosition(newPoint, lastPoint);

		if (this->DisplayPolygon)
		{
			this->GeneratePolygon();
		}
	}
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::insertNewPointOnEventPosition(vtkVector2i &newPoint, vtkVector2i &lastPoint)
{
	double displayPosition[3];
	double worldOrientRef[9];
	double worldPosition[3];
	double worldPositionRef[3];
	double worldOrient[9];
	vtkSmartPointer<vtkFocalPlanePointPlacer> placer = vtkSmartPointer<vtkFocalPlanePointPlacer>::New();

	displayPosition[0] = lastPoint.GetX();
	displayPosition[1] = lastPoint.GetY();

	placer->ComputeWorldPosition(this->GetCurrentRenderer(), displayPosition, worldPositionRef, worldOrientRef);

	displayPosition[0] = newPoint.GetX();
	displayPosition[1] = newPoint.GetY();
	placer->ComputeWorldPosition(this->GetCurrentRenderer(), displayPosition, worldPositionRef, worldPosition, worldOrient);

	this->Internal->worldPositionPoints->InsertNextPoint(worldPosition);

	std::cout << this->Internal->worldPositionPoints->GetNumberOfPoints() 
		<< " x:" << worldPosition[0] 
		<< " y:" << worldPosition[1] 
		<< " z:" << worldPosition[2] 
		<< std::endl;
}


//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::OnLeftButtonDown()
{
	if (!this->Interactor)
	{
		return;
	}
	if (!this->CurrentRenderer)
	{
		this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
			this->Interactor->GetLastEventPosition()[0],
			this->Interactor->GetLastEventPosition()[1]));
		if (this->CurrentRenderer == nullptr)
		{
			return;
		}
	}

	this->Moving = 1;

	this->CurrentRenderer->AddActor(this->Internal->polygonActor);

	this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
	this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
	this->EndPosition[0] = this->StartPosition[0];
	this->EndPosition[1] = this->StartPosition[1];

	vtkVector2i lastPoint(this->StartPosition[0], this->StartPosition[1]);
	vtkVector2i newPoint(this->EndPosition[0], this->EndPosition[1]);

	this->Internal->Clear();
	this->Internal->AddPoint(this->StartPosition[0], this->StartPosition[1]);
	this->insertNewPointOnEventPosition(newPoint, lastPoint);

	this->InvokeEvent(vtkCommand::StartInteractionEvent);
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::OnLeftButtonUp()
{
	if (!this->Interactor || !this->Moving)
	{
		return;
	}

	if (this->DisplayPolygon)
	{
		this->CurrentRenderer->RemoveActor(this->Internal->polygonActor);
	}

	this->Moving = 0;
	this->InvokeEvent(vtkCommand::SelectionChangedEvent);
	this->InvokeEvent(vtkCommand::EndInteractionEvent);
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::GeneratePolygon()
{
	this->Internal->polydata->SetPoints(this->Internal->worldPositionPoints);
	this->Internal->polygon->GetPointIds()->SetNumberOfIds(this->Internal->worldPositionPoints->GetNumberOfPoints());
	for (vtkIdType i = 0; i < this->Internal->worldPositionPoints->GetNumberOfPoints(); i++)
	{
		this->Internal->polygon->GetPointIds()->SetId(i, i);
	}
	this->Internal->polygons->Reset();
	this->Internal->polygons->InsertNextCell(this->Internal->polygon);	
	this->Internal->polydata->SetPolys(this->Internal->polygons);
	this->Internal->worldPositionPoints->Modified();
	this->Internal->polygons->Modified();
	this->Internal->polydata->Modified();

	this->CurrentRenderer->GetRenderWindow()->Render();
}

//-------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::ExecuteCameraUpdateEvent()
{
	if (!this->CurrentRenderer)
	{
		return;
	}

	double vp[4];
	this->CurrentRenderer->GetViewport(vp);
	this->CurrentRenderer->ViewportToNormalizedDisplay(vp[0], vp[1]);
	this->CurrentRenderer->ViewportToNormalizedDisplay(vp[2], vp[3]);
	this->CurrentRenderer->ResetCamera();
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayPolygon::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "Moving: " << this->Moving << endl;
	os << indent << "StartPosition: " << this->StartPosition[0] << "," << this->StartPosition[1] << endl;
	os << indent << "EndPosition: " << this->EndPosition[0] << "," << this->EndPosition[1] << endl;
}
