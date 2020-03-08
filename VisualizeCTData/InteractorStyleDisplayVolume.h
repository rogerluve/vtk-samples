#pragma once

#include "vtkInteractorStyleTrackballCamera.h"

#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points

class vtkUnsignedCharArray;
class vtkPoints;
class vtkImageData;

class InteractorStyleDisplayVolume : public vtkInteractorStyleTrackballCamera
{
public:
	static InteractorStyleDisplayVolume *New();
	vtkTypeMacro(InteractorStyleDisplayVolume, vtkInteractorStyleTrackballCamera);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnChar() override;
	//@}


	void GenerateVolumePreview(vtkImageData& imageData);

protected:
	InteractorStyleDisplayVolume();
	~InteractorStyleDisplayVolume() override;

	void ExecuteCameraUpdateEvent();

	void VolumeToThreshold(int min);

	int StartPosition[2];
	int EndPosition[2];
	int Moving;

	bool DisplayPolygon;

private:
	InteractorStyleDisplayVolume(const InteractorStyleDisplayVolume&) = delete;
	void operator=(const InteractorStyleDisplayVolume&) = delete;

	class vtkInternal;
	vtkInternal* Internal;
};
