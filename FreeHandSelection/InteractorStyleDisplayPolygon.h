#pragma once

#include "vtkInteractorStyle.h"

#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points

class vtkUnsignedCharArray;
class vtkPoints;

class InteractorStyleDisplayPolygon : public vtkInteractorStyle
{
public:
	static InteractorStyleDisplayPolygon *New();
	vtkTypeMacro(InteractorStyleDisplayPolygon, vtkInteractorStyle);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	//@{
	/**
	 * Event bindings
	 */
	void OnMouseMove() override;
	void insertNewPointOnEventPosition(vtkVector2i &newPoint, vtkVector2i &lastPoint);
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	//@}

	//@{
	/**
	 * Whether to draw polygon in screen pixels. Default is ON
	 */
	vtkSetMacro(DisplayPolygon, bool);
	vtkGetMacro(DisplayPolygon, bool);
	vtkBooleanMacro(DisplayPolygon, bool);
	//@}

	/**
	 * Get the current polygon points in display units
	 */
	std::vector<vtkVector2i> GetPolygonPointsDisplayUnits();
	vtkPoints* GetPolygonPoints();


protected:
	InteractorStyleDisplayPolygon();
	~InteractorStyleDisplayPolygon() override;

	virtual void GeneratePolygon();

	void ExecuteCameraUpdateEvent();

	int StartPosition[2];
	int EndPosition[2];
	int Moving;

	bool DisplayPolygon;

private:
	InteractorStyleDisplayPolygon(const InteractorStyleDisplayPolygon&) = delete;
	void operator=(const InteractorStyleDisplayPolygon&) = delete;

	class vtkInternal;
	vtkInternal* Internal;
};
