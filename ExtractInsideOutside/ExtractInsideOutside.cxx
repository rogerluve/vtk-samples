#include "ExtractInsideOutside.h"
#include "InteractorStyleSelector.h"
#include "InteractorStyleDisplayPolygon.h"

#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleUser.h>
#include <vtkCamera.h>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTriangleFilter.h>
#include <vtkImplicitSelectionLoop.h>
#include <vtkClipPolyData.h>
#include <vtkTubeFilter.h>
#include <vtkVersion.h>


// Constructor
ExtractInsideOutside::ExtractInsideOutside()
{
	ReferenceLoop = std::vector<vtkSmartPointer<vtkPoints>>();
	Boundaries = std::vector<vtkSmartPointer<vtkPoints>>();
	IsInsideLoop = false;

	this->setupUi(this);
	vtkNew<vtkNamedColors> colors;

	renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	renderWindow->SetWindowName("ExtractInsideOutside");

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());
	renderWindow->AddRenderer(renderer);
	qvtkWidget->SetRenderWindow(renderWindow);

	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	style = vtkSmartPointer<InteractorStyleSelector>::New();
	style->SetCurrentStyleToTrackballCamera();
	renderWindowInteractor->SetInteractorStyle(style);

	// Setup the selection callback to modify this object when an array selection is changed.
	this->SelectionObserver = vtkCallbackCommand::New();
	this->SelectionObserver->SetCallback(&ExtractInsideOutside::SelectionModifiedCallback);
	this->SelectionObserver->SetClientData(this);
	style->GetDisplayPolygonCamera()->AddObserver(vtkCommand::SelectionChangedEvent, this->SelectionObserver);


	double viewUp[3] = { 0.0,1.0,0.0 };
	double position[3] = { 0.0,0.0,100.0 };
	renderer->GetActiveCamera()->SetViewUp(viewUp);
	renderer->GetActiveCamera()->SetPosition(position);
	renderer->ResetCamera();

	renderWindow->Render();
	renderWindowInteractor->Initialize();

	// Set up action signals and slots
	connect(this->pushButton, &QPushButton::clicked, this, &ExtractInsideOutside::slotSetReferenceLoop);
	connect(this->pushButton_2, &QPushButton::clicked, this, &ExtractInsideOutside::slotAddBoundaryLoop);

	connect(this->pushButtonExtract, &QPushButton::clicked, this, [this]() {this->slotInsideOutside(true); });
	connect(this->pushButtonClip, &QPushButton::clicked, this, [this]() {this->slotInsideOutside(false); });
	connect(this->pushButtonAlgo1, &QPushButton::clicked, this, [this]() {this->slotAlgo(true);});
	connect(this->pushButtonAlgo2, &QPushButton::clicked, this, [this]() {this->slotAlgo(false);});
	connect(this->checkBoxInside, &QCheckBox::toggled, this, [this](bool checked) {IsInsideLoop = checked; });
	connect(this->actionExit, &QAction::triggered, this, [this](bool checked) {qApp->exit();});
	connect(this->pushButtonClear, &QPushButton::clicked, this, [this]() {this->renderer->RemoveAllViewProps(); this->renderWindow->Render(); });
};

void ExtractInsideOutside::slotSetReferenceLoop()
{
	LoopType = LoopTypeEnum::Reference;
	this->style->SetCurrentStyleToDrawPolygon();
}

void ExtractInsideOutside::slotAddBoundaryLoop()
{
	LoopType = LoopTypeEnum::Boundary;
	this->style->SetCurrentStyleToDrawPolygon();
}

void ExtractInsideOutside::SelectionModifiedCallback(vtkObject* caller, unsigned long, void* clientdata, void*)
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
	ExtractInsideOutside *obj = static_cast<ExtractInsideOutside*>(clientdata);

	vtkPoints* points = obj->style->GetDisplayPolygonCamera()->GetPolygonPoints();
	vtkSmartPointer<vtkPoints> worldPositionPoints = vtkSmartPointer<vtkPoints>::New();
	worldPositionPoints->DeepCopy(points);

	switch (obj->LoopType)
	{
	case LoopTypeEnum::Reference:
		obj->ReferenceLoop.clear();
		obj->ReferenceLoop.push_back(worldPositionPoints);
		std::cout << "Added to Reference " << obj->ReferenceLoop.size() << std::endl;
		break;
	case LoopTypeEnum::Boundary:
		obj->Boundaries.clear();
		obj->Boundaries.push_back(worldPositionPoints);
		std::cout << "Added to Boundary " << obj->Boundaries.size() << std::endl;
		break;
	default:
		break;
	}

	vtkSmartPointer<vtkPoints> pointsForDrawing = vtkSmartPointer<vtkPoints>::New();
	pointsForDrawing->DeepCopy(worldPositionPoints);
	//pointsForDrawing->InsertNextPoint(pointsForDrawing->GetPoint(0));
	obj->AddSegmentToRenderWindow(pointsForDrawing, obj->LoopType);
}


void ExtractInsideOutside::slotInsideOutside(bool extract)
{
	if (this->ReferenceLoop.size() < 1 && this->Boundaries.size() < 1)
	{
		return;
	}

	std::vector<vtkSmartPointer<vtkPoints>> segments = std::vector<vtkSmartPointer<vtkPoints>>();
	if (extract)
	{
		ExtractSegments(segments, this->Boundaries[0], this->ReferenceLoop[0], this->IsInsideLoop);
	}
	else
	{
		ClipSegments(segments, this->Boundaries[0], this->ReferenceLoop[0], this->IsInsideLoop);
	}

	for (int x = 0; x < segments.size(); x++)
	{
		AddSegmentToRenderWindow(segments[x], this->IsInsideLoop ? LoopTypeEnum::MergedInside : LoopTypeEnum::MergedOutside);
	}
}

void ExtractInsideOutside::slotAlgo(bool inOut)
{
	if (this->ReferenceLoop.size() < 1 && this->Boundaries.size() < 1)
	{
		return;
	}

	std::vector<vtkSmartPointer<vtkPoints>> segments = std::vector<vtkSmartPointer<vtkPoints>>();
	ExtractSegments(segments, this->Boundaries[0], this->ReferenceLoop[0], inOut);
	ExtractSegments(segments, this->ReferenceLoop[0], this->Boundaries[0], inOut);

	//------------------------------------------------------------------------

	vtkSmartPointer<vtkPoints> theSegment = vtkSmartPointer<vtkPoints>::New();
	
	ConnectivityOfSegments(segments, theSegment);

	AddSegmentToRenderWindow(theSegment, inOut ? LoopTypeEnum::Algo1 : LoopTypeEnum::Algo2);
}

void ExtractInsideOutside::ConnectivityOfSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &theSegment)
{
	const double MAXDISTANCE_FOR_CONNECTED = 5;
	//search a point outside
	vtkSmartPointer<vtkPoints> probe = segments[0];
	InsertNextPoints(theSegment, probe, 0, probe->GetNumberOfPoints() - 1);
	segments.erase(segments.begin());

	double theSegmentLastPoint[3];
	theSegment->GetPoint(theSegment->GetNumberOfPoints() - 1, theSegmentLastPoint);

	int closest = 0;
	double minDist = VTK_DOUBLE_MAX;
	bool isFirstPoint = true;

	while (segments.size() > 0)
	{
		closest = 0;
		minDist = VTK_DOUBLE_MAX;
		isFirstPoint = true;
		for (int y = 0; y < segments.size(); y++)
		{
			double firstPoint[3], lastPoint[3];
			segments[y]->GetPoint(0, firstPoint);
			segments[y]->GetPoint(segments[y]->GetNumberOfPoints() - 1, lastPoint);
			double distanceFirstPoint = sqrt(vtkMath::Distance2BetweenPoints(theSegmentLastPoint, firstPoint));
			double distanceLastPoint = sqrt(vtkMath::Distance2BetweenPoints(theSegmentLastPoint, lastPoint));

			if (distanceFirstPoint < minDist)
			{
				minDist = distanceFirstPoint;
				closest = y;
				isFirstPoint = true;
			}
			if (distanceLastPoint < minDist)
			{
				minDist = distanceLastPoint;
				closest = y;
				isFirstPoint = false;
			}
		}

		probe = segments[closest];
		if (minDist < MAXDISTANCE_FOR_CONNECTED) //filter out segments outside of selection
		{
			if (isFirstPoint == true)
			{
				InsertNextPoints(theSegment, probe, 0, probe->GetNumberOfPoints() - 1);
			}
			else
			{
				InsertNextPoints(theSegment, probe, probe->GetNumberOfPoints() - 1, 0);
			}
		}
		segments.erase(segments.begin() + closest);
		theSegment->GetPoint(theSegment->GetNumberOfPoints() - 1, theSegmentLastPoint);
	}
}

void ExtractInsideOutside::InsertNextPoints(vtkPoints* target, vtkPoints* source, vtkIdType srcStart, vtkIdType srcEnd)
{
	if(source->GetNumberOfPoints() > 0 && srcEnd >= srcStart)
	{
		for (size_t i = srcStart; i <= srcEnd; i++)
		{
			target->InsertNextPoint(source->GetPoint(i));
			std::cout << target->GetNumberOfPoints() << ""
				<< " x:" << source->GetPoint(i)[0]
				<< " y:" << source->GetPoint(i)[1]
				<< " z:" << source->GetPoint(i)[2]
				<< " reverse:" << false
				<< std::endl;
		}
		//target->InsertPoints(target->GetNumberOfPoints(), srcEnd - srcStart + 1, srcStart, source);
	}
	else if (source->GetNumberOfPoints() && srcEnd < srcStart)
	{
		for (size_t i = srcEnd; i >= srcStart; i--)
		{
			target->InsertNextPoint(source->GetPoint(i));
			std::cout << target->GetNumberOfPoints() << ""
				<< " x:" << source->GetPoint(i)[0]
				<< " y:" << source->GetPoint(i)[1]
				<< " z:" << source->GetPoint(i)[2]
				<< " reverse:" << true
				<< std::endl;
		}
	}
}

void ExtractInsideOutside::ExtractSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &Boundary, vtkSmartPointer<vtkPoints> &ReferenceLoop, bool insideLoop)
{
	vtkSmartPointer<vtkImplicitSelectionLoop> ImplicitFunction = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
	ImplicitFunction->AutomaticNormalGenerationOff();
	ImplicitFunction->SetLoop(ReferenceLoop);
	ImplicitFunction->SetNormal(this->renderer->GetActiveCamera()->GetDirectionOfProjection());

	const vtkIdType nPoints = Boundary->GetNumberOfPoints();
	double aPoint[3];
	const double multiplier = insideLoop ? 1.0 : -1.0;
	double minDistance;

	vtkSmartPointer<vtkPoints> aSegment = vtkSmartPointer<vtkPoints>::New();
	for (int i = 0; i < nPoints; ++i)
	{
		Boundary->GetPoint(i, aPoint);
		minDistance = ImplicitFunction->FunctionValue(aPoint) * multiplier;

		if (minDistance < 0.0)
		{
			aSegment->InsertNextPoint(aPoint);
			this->AddPointToRenderWindow(aPoint, LoopTypeEnum::MergedInside);
		}
		else
		{
			if (aSegment->GetNumberOfPoints() > 0)
			{
				segments.push_back(aSegment);
				aSegment = vtkSmartPointer<vtkPoints>::New();				
			}
			this->AddPointToRenderWindow(aPoint, LoopTypeEnum::MergedOutside);
		}
	}

	if (aSegment->GetNumberOfPoints() > 0)
	{
		segments.push_back(aSegment);
	}
}

void ExtractInsideOutside::ClipSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &Boundary, vtkSmartPointer<vtkPoints> &ReferenceLoop, bool insideLoop)
{
	vtkSmartPointer<vtkImplicitSelectionLoop> ImplicitFunction = vtkSmartPointer<vtkImplicitSelectionLoop>::New();
	ImplicitFunction->AutomaticNormalGenerationOff();
	ImplicitFunction->SetLoop(ReferenceLoop);
	ImplicitFunction->SetNormal(this->renderer->GetActiveCamera()->GetDirectionOfProjection());

	const vtkIdType nPoints = Boundary->GetNumberOfPoints();

	double aPoint[3];
	double bPoint[3];
	double newPoint[3];
	const double multiplier = insideLoop ? 1.0 : -1.0;
	double minDistanceA;
	double minDistanceB;
	vtkSmartPointer<vtkPoints> aSegment = vtkSmartPointer<vtkPoints>::New();
	for (int i = 0; i < nPoints; ++i)
	{
		Boundary->GetPoint(i, aPoint);
		Boundary->GetPoint(i < (nPoints-1) ? i+1 : 0, bPoint);		
		minDistanceA = ImplicitFunction->FunctionValue(aPoint) * multiplier;
		minDistanceB = ImplicitFunction->FunctionValue(bPoint) * multiplier;

		//test for clipping a line (it can be inside or outside)
		if (minDistanceA * minDistanceB < 0 &&
			ClipLine(ReferenceLoop, insideLoop, aPoint, bPoint, minDistanceA, minDistanceB, newPoint))
		{
			if (minDistanceA < 0.0)
			{
				aSegment->InsertNextPoint(aPoint);
				this->AddPointToRenderWindow(aPoint, LoopTypeEnum::MergedInside);
				aSegment->InsertNextPoint(newPoint);
				this->AddPointToRenderWindow(newPoint, LoopTypeEnum::MergedInsideOutside);
			}
			else
			{
				aSegment->InsertNextPoint(newPoint);
				this->AddPointToRenderWindow(newPoint, LoopTypeEnum::MergedInsideOutside);
				aSegment->InsertNextPoint(bPoint);
				this->AddPointToRenderWindow(bPoint, LoopTypeEnum::MergedInside);
			}
		}
		else if (minDistanceA < 0.0)
		{
			aSegment->InsertNextPoint(aPoint);
			this->AddPointToRenderWindow(aPoint, LoopTypeEnum::MergedInside);
		}
		else
		{
			if (aSegment->GetNumberOfPoints() > 0)
			{
				segments.push_back(aSegment);
				aSegment = vtkSmartPointer<vtkPoints>::New();
			}
		}
		this->AddPointToRenderWindow(aPoint, LoopTypeEnum::MergedOutside);
	}
	if (aSegment->GetNumberOfPoints() > 0)
	{
		segments.push_back(aSegment);
	}
}

bool ExtractInsideOutside::ClipLine(vtkSmartPointer<vtkPoints> & ReferenceLoop, bool insideLoop, double  aPoint[3], double  bPoint[3], double distA, double distB, double  newPoint[3])
{
	//interpolate (just for demo purposes)
	newPoint[0] = aPoint[0] + 0.5 * (bPoint[0] - aPoint[0]);
	newPoint[1] = aPoint[1] + 0.5 * (bPoint[1] - aPoint[1]);
	newPoint[2] = aPoint[2] + 0.5 * (bPoint[2] - aPoint[2]);
	return true;
}

void ExtractInsideOutside::AddPointToRenderWindow(double aPoint[3], LoopTypeEnum options)
{
	vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();

	sphereSource->SetCenter(aPoint);
	sphereSource->Update();
	mapper->SetInputConnection(sphereSource->GetOutputPort());
	actor->SetMapper(mapper);
	ConfigureActor(options, actor);
	this->renderer->AddActor(actor);
}

void ExtractInsideOutside::AddSegmentToRenderWindow(vtkSmartPointer<vtkPoints> &aMergedPoints, LoopTypeEnum options)
{
	vtkSmartPointer<vtkPolyLine> closedPolyline = vtkSmartPointer<vtkPolyLine>::New();
	vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

	closedPolyline->GetPointIds()->SetNumberOfIds(aMergedPoints->GetNumberOfPoints());
	for (vtkIdType i = 0; i < aMergedPoints->GetNumberOfPoints(); i++)
	{
		closedPolyline->GetPointIds()->SetId(i, i);
	}

	polygons->InsertNextCell(closedPolyline);
	polydata->SetPoints(aMergedPoints);
	polydata->SetLines(polygons);
	polydata->Modified();

	vtkSmartPointer<vtkTubeFilter> tube = vtkSmartPointer<vtkTubeFilter>::New();
	tube->SetInputData(polydata);

	// to graphics primitives
	vtkSmartPointer<vtkPolyDataMapper> polygonMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	polygonMapper->SetInputConnection(tube->GetOutputPort());

	vtkSmartPointer<vtkActor> polygonActor = vtkSmartPointer<vtkActor>::New();
	polygonActor->SetMapper(polygonMapper);

	ConfigureActor(options, polygonActor);

	this->renderer->AddActor(polygonActor);
	this->renderWindow->Render();
}

void ExtractInsideOutside::ConfigureActor(ExtractInsideOutside::LoopTypeEnum options, vtkSmartPointer<vtkActor> &actor)
{
	switch (options)
	{
	case LoopTypeEnum::MergedInside:
		actor->GetProperty()->SetColor(45, 0, 0);
		actor->GetProperty()->SetOpacity(1);
		break;
	case LoopTypeEnum::MergedOutside:
		actor->GetProperty()->SetColor(0, 45, 0);
		actor->GetProperty()->SetOpacity(1);
		break;
	case LoopTypeEnum::MergedInsideOutside:
		actor->GetProperty()->SetColor(0, 0, 0);
		actor->GetProperty()->SetOpacity(1);
		break;
	case LoopTypeEnum::Algo1:
		actor->GetProperty()->SetColor(0, 0, 45);
		actor->GetProperty()->SetOpacity(1);
		break;
	case LoopTypeEnum::Algo2:
		actor->GetProperty()->SetColor(45, 0, 45);
		actor->GetProperty()->SetOpacity(1);
		break;
	case LoopTypeEnum::Reference:
		actor->GetProperty()->SetColor(0, 45, 45);
		actor->GetProperty()->SetOpacity(0.25);
		break;
	case LoopTypeEnum::Boundary:
		actor->GetProperty()->SetColor(45, 45, 0);
		actor->GetProperty()->SetOpacity(0.25);
		break;
	default:
		actor->GetProperty()->SetColor(75, 75, 75);
		actor->GetProperty()->SetOpacity(0.25);
		break;
	}
}


