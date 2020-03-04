#pragma once

#include "ui_ExtractInsideOutside.h"

#include "InteractorStyleSelector.h"
#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points

#include <QMainWindow>
class vtkCallbackCommand;
class vtkObject;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkGenericOpenGLRenderWindow;
class vtkPoints;

class ExtractInsideOutside
  : public QMainWindow,
    private Ui::ExtractInsideOutside
{
  Q_OBJECT
public:
  ExtractInsideOutside();

protected:
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
	vtkSmartPointer<vtkRenderer> renderer;
	// The observer to modify this object when the array selections are modified.
	vtkCallbackCommand* SelectionObserver;
	// Callback registered with the SelectionObserver.
	static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid, void* clientdata, void* calldata);
	vtkSmartPointer<InteractorStyleSelector> style;
	std::vector<vtkSmartPointer<vtkPoints>> ReferenceLoop;
	std::vector<vtkSmartPointer<vtkPoints>> Boundaries;
	enum class LoopTypeEnum
	{
		Reference,
		Boundary,
		MergedInside,
		MergedInsideOutside,
		MergedOutside,
		Algo1,
		Algo2
	} LoopType;
	bool IsInsideLoop;

public slots:

	void ExtractSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &Boundary, vtkSmartPointer<vtkPoints> &ReferenceLoop, bool insideLoop);
	void ClipSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &Boundary, vtkSmartPointer<vtkPoints> &ReferenceLoop, bool insideLoop);
	bool ClipLine(vtkSmartPointer<vtkPoints> & ReferenceLoop, bool insideLoop, double  aPoint[3], double bPoint[3], double distA, double distB, double  newPoint[3]);
	void AddSegmentToRenderWindow(vtkSmartPointer<vtkPoints> &points, LoopTypeEnum options);
	void AddPointToRenderWindow(double aPoint[3], LoopTypeEnum options);
	void ConfigureActor(ExtractInsideOutside::LoopTypeEnum options, vtkSmartPointer<vtkActor> &actor);
    void slotSetReferenceLoop();
    void slotAddBoundaryLoop();
    void slotInsideOutside(bool extract);
    void slotAlgo(bool inOut);

	void ConnectivityOfSegments(std::vector<vtkSmartPointer<vtkPoints>> &segments, vtkSmartPointer<vtkPoints> &theSegment);

	void InsertNextPoints(vtkPoints * target, vtkPoints * source, vtkIdType srcStart, vtkIdType srcEnd);

};


