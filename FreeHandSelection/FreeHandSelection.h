#pragma once

#include "ui_FreeHandSelection.h"

#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points

#include <QMainWindow>
class vtkCallbackCommand;
class vtkObject;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkGenericOpenGLRenderWindow;
class vtkPoints;
class InteractorStyleDisplayPolygon;

class FreeHandSelection
  : public QMainWindow,
    private Ui::FreeHandSelection
{
  Q_OBJECT
public:
  FreeHandSelection();

protected:
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
	vtkSmartPointer<vtkRenderer> renderer;
	// The observer to modify this object when the array selections are modified.
	vtkCallbackCommand* SelectionObserver;
	// Callback registered with the SelectionObserver.
	static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid, void* clientdata, void* calldata);
	vtkSmartPointer<InteractorStyleDisplayPolygon> style;

};


