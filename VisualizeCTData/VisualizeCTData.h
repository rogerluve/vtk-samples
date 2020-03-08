#pragma once

#include "ui_VisualizeCTData.h"

#include <vector>      // For returning Polygon Points
#include "vtkVector.h" // For Polygon Points

#include <QMainWindow>
class vtkCallbackCommand;
class vtkObject;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkGenericOpenGLRenderWindow;
class vtkPoints;
class InteractorStyleDisplayVolume;

class vtkDICOMImageReader;
class vtkFixedPointVolumeRayCastMapper;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolume;
class vtkVolumeProperty;

class VisualizeCTData
  : public QMainWindow,
    private Ui::VisualizeCTData
{
  Q_OBJECT
public:
  VisualizeCTData();

protected:
	vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
	vtkSmartPointer<vtkRenderer> renderer;

	vtkSmartPointer<InteractorStyleDisplayVolume> style;

	vtkSmartPointer<vtkDICOMImageReader> dicomReader;

private:
	void LoadDICOM();
};
