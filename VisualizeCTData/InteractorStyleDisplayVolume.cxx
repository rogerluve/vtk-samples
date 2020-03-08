#include "InteractorStyleDisplayVolume.h"

#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnsignedCharArray.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"

#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkImageShiftScale.h>
#include <vtkPointData.h>

vtkStandardNewMacro(InteractorStyleDisplayVolume);

//-----------------------------------------------------------------------------
class InteractorStyleDisplayVolume::vtkInternal
{
public:
	vtkInternal()
		: Threshold(95)
	{
		this->mapperVolume = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
		this->volume = vtkSmartPointer<vtkVolume>::New();
		this->volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
		this->volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
		this->volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
		this->volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();

		this->resample = vtkSmartPointer<vtkImageResample>::New();
		this->imageScale = vtkSmartPointer<vtkImageShiftScale>::New();
	}

	vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> mapperVolume;
	vtkSmartPointer<vtkVolume> volume;
	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity;
	vtkSmartPointer<vtkColorTransferFunction> volumeColor;
	vtkSmartPointer<vtkImageResample> resample;
	vtkSmartPointer<vtkImageShiftScale> imageScale;

	int Threshold;
};

//----------------------------------------------------------------------------
InteractorStyleDisplayVolume::InteractorStyleDisplayVolume()
{
	this->Internal = new vtkInternal();
}

//----------------------------------------------------------------------------
InteractorStyleDisplayVolume::~InteractorStyleDisplayVolume()
{
	delete this->Internal;
}

void InteractorStyleDisplayVolume::OnChar()
{
	char key = this->Interactor->GetKeyCode();
	switch (key)
	{
	case '1':
		Internal->Threshold += 5;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case '2':
		Internal->Threshold -= 5;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	default:
		break;
	}

	VolumeToThreshold(Internal->Threshold);
	GetInteractor()->Render();
}



//-------------------------------------------------------------------------
void InteractorStyleDisplayVolume::ExecuteCameraUpdateEvent()
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

void InteractorStyleDisplayVolume::VolumeToThreshold(int value)
{
	// The color transfer function maps voxel intensities to colors.
	// It is modality-specific, and often anatomy-specific as well.  
	Internal->volumeColor->RemoveAllPoints();
	Internal->volumeColor->AddRGBPoint(0, 0, 0, 0);
	Internal->volumeColor->AddRGBPoint(5, .55, .25, .15);
	Internal->volumeColor->AddRGBPoint(40, 0.88, .60, .29);
	Internal->volumeColor->AddRGBPoint(80, 1, .94, .95);
	Internal->volumeColor->AddRGBPoint(255, 1, 1, 1);

	// The opacity transfer function is used to control the opacity
	// of different tissue types.
	Internal->volumeScalarOpacity->RemoveAllPoints();
	Internal->volumeScalarOpacity->AddPoint(0, 0);
	Internal->volumeScalarOpacity->AddPoint(value - 0.01, 0);
	Internal->volumeScalarOpacity->AddPoint(value + 0.01, 1);
	Internal->volumeScalarOpacity->AddPoint(255, 1);

	// The gradient opacity function is used to decrease the opacity
    // in the "flat" regions of the volume while maintaining the opacity
    // at the boundaries between tissue types.  The gradient is measured
    // as the amount by which the intensity changes over unit distance.
    // For most medical data, the unit distance is 1mm.
	Internal->volumeGradientOpacity->RemoveAllPoints();
	Internal->volumeGradientOpacity->AddPoint(0, 0.8);
	Internal->volumeGradientOpacity->AddPoint(255, 1.0);

	Internal->volumeProperty->DisableGradientOpacityOn();
	Internal->volumeProperty->SetInterpolationTypeToLinear();
	Internal->volumeProperty->ShadeOn();
	Internal->volumeProperty->SetAmbient(0.4);
	Internal->volumeProperty->SetDiffuse(0.6);
	Internal->volumeProperty->SetSpecular(0.10);

	Internal->mapperVolume->SetBlendModeToComposite();
}

void InteractorStyleDisplayVolume::GenerateVolumePreview(vtkImageData& imageData)
{
	//rescale to 0-255
	double range[3];
	imageData.GetScalarRange(range);
	Internal->imageScale->SetInputData(&imageData);
	Internal->imageScale->SetShift(-range[0]);
	Internal->imageScale->SetScale(255.0 / (range[1] - range[0]));
	Internal->imageScale->Update();

	//connect mapper
	Internal->mapperVolume->SetInputConnection(Internal->imageScale->GetOutputPort());

	// Set the sample distance on the ray to be 1/2 the average spacing
	double spacing[3];
	imageData.GetSpacing(spacing);
	Internal->mapperVolume->SetSampleDistance((float)((spacing[0] + spacing[1] + spacing[2]) / 6.0));

	// The VolumeProperty attaches the color and opacity functions to the
	// volume, and sets other volume properties.  The interpolation should
	// be set to linear to do a high-quality rendering.  The ShadeOn option
	// turns on directional lighting, which will usually enhance the
	// appearance of the volume and make it look more "3D".  However,
	// the quality of the shading depends on how accurately the gradient
	// of the volume can be calculated, and for noisy data the gradient
	// estimation will be very poor.  The impact of the shading can be
	// decreased by increasing the Ambient coefficient while decreasing
	// the Diffuse and Specular coefficient.  To increase the impact
	// of shading, decrease the Ambient and increase the Diffuse and Specular.
	Internal->volumeProperty->SetIndependentComponents(1);
	Internal->volumeProperty->SetColor(Internal->volumeColor);
	Internal->volumeProperty->SetScalarOpacity(Internal->volumeScalarOpacity);
	Internal->volumeProperty->SetGradientOpacity(Internal->volumeGradientOpacity);
	Internal->volumeProperty->SetInterpolationTypeToLinear();

	// connect up the volume to the property and the mapper
	Internal->volume->SetProperty(Internal->volumeProperty);
	Internal->volume->SetMapper(Internal->mapperVolume);

	VolumeToThreshold(Internal->Threshold);
	GetCurrentRenderer()->AddVolume(Internal->volume);

	GetCurrentRenderer()->ResetCamera();
	GetCurrentRenderer()->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void InteractorStyleDisplayVolume::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}
