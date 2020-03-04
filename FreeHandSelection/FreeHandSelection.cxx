#include "FreeHandSelection.h"

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
FreeHandSelection::FreeHandSelection()
{
	this->setupUi(this);
	vtkNew<vtkNamedColors> colors;

	renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	renderWindow->SetWindowName("FreeHandSelection");

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());
	renderWindow->AddRenderer(renderer);
	qvtkWidget->SetRenderWindow(renderWindow);

	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	style = vtkSmartPointer<InteractorStyleDisplayPolygon>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// Setup the selection callback to modify this object when an array selection is changed.
	this->SelectionObserver = vtkCallbackCommand::New();
	this->SelectionObserver->SetCallback(&FreeHandSelection::SelectionModifiedCallback);
	this->SelectionObserver->SetClientData(this);
	style->AddObserver(vtkCommand::SelectionChangedEvent, this->SelectionObserver);

	renderWindow->Render();
	renderWindowInteractor->Initialize();
};

void FreeHandSelection::SelectionModifiedCallback(vtkObject* caller, unsigned long, void* clientdata, void*)
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
	FreeHandSelection *obj = static_cast<FreeHandSelection*>(clientdata);

	vtkPoints* points = obj->style->GetPolygonPoints();
	//do something
}



