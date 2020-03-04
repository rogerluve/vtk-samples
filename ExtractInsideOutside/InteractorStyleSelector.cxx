#include "InteractorStyleSelector.h"

#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkInteractorStyleJoystickActor.h"
#include "vtkInteractorStyleJoystickCamera.h"
#include "vtkInteractorStyleTrackballActor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkInteractorStyleMultiTouchCamera.h"
#include "InteractorStyleDisplayPolygon.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(InteractorStyleSelector);

//----------------------------------------------------------------------------
InteractorStyleSelector::InteractorStyleSelector()
{
	this->JoystickActor = vtkInteractorStyleJoystickActor::New();
	this->JoystickCamera = vtkInteractorStyleJoystickCamera::New();
	this->TrackballActor = vtkInteractorStyleTrackballActor::New();
	this->TrackballCamera = vtkInteractorStyleTrackballCamera::New();
	this->MultiTouchCamera = vtkInteractorStyleMultiTouchCamera::New();
	this->DrawPolygonCamera = InteractorStyleDisplayPolygon::New();
	this->JoystickOrTrackball = VTKIS_JOYSTICK;
	this->CameraOrActor = VTKIS_CAMERA;
	this->MultiTouch = false;
	this->DrawPolygonTool = false;
	this->CurrentStyle = nullptr;
}

//----------------------------------------------------------------------------
InteractorStyleSelector::~InteractorStyleSelector()
{
	this->JoystickActor->Delete();
	this->JoystickActor = nullptr;

	this->JoystickCamera->Delete();
	this->JoystickCamera = nullptr;

	this->TrackballActor->Delete();
	this->TrackballActor = nullptr;

	this->TrackballCamera->Delete();
	this->TrackballCamera = nullptr;

	this->MultiTouchCamera->Delete();
	this->MultiTouchCamera = nullptr;

	this->DrawPolygonCamera->Delete();
	this->DrawPolygonCamera = nullptr;
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetAutoAdjustCameraClippingRange(vtkTypeBool value)
{
	if (value == this->AutoAdjustCameraClippingRange)
	{
		return;
	}

	if (value < 0 || value > 1)
	{
		vtkErrorMacro("Value must be between 0 and 1 for" <<
			" SetAutoAdjustCameraClippingRange");
		return;
	}

	this->AutoAdjustCameraClippingRange = value;
	this->JoystickActor->SetAutoAdjustCameraClippingRange(value);
	this->JoystickCamera->SetAutoAdjustCameraClippingRange(value);
	this->TrackballActor->SetAutoAdjustCameraClippingRange(value);
	this->TrackballCamera->SetAutoAdjustCameraClippingRange(value);
	this->MultiTouchCamera->SetAutoAdjustCameraClippingRange(value);
	this->DrawPolygonCamera->SetAutoAdjustCameraClippingRange(value);

	this->Modified();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToJoystickActor()
{
	this->JoystickOrTrackball = VTKIS_JOYSTICK;
	this->CameraOrActor = VTKIS_ACTOR;
	this->MultiTouch = false;
	this->DrawPolygonTool = false;
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToJoystickCamera()
{
	this->JoystickOrTrackball = VTKIS_JOYSTICK;
	this->CameraOrActor = VTKIS_CAMERA;
	this->MultiTouch = false;
	this->DrawPolygonTool = false;
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToTrackballActor()
{
	this->JoystickOrTrackball = VTKIS_TRACKBALL;
	this->CameraOrActor = VTKIS_ACTOR;
	this->MultiTouch = false;
	this->DrawPolygonTool = false;
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToTrackballCamera()
{
	this->JoystickOrTrackball = VTKIS_TRACKBALL;
	this->CameraOrActor = VTKIS_CAMERA;
	this->MultiTouch = false;
	this->DrawPolygonTool = false;
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToMultiTouchCamera()
{
	this->MultiTouch = true;
	this->DrawPolygonTool = false;
	this->SetCurrentStyle();
}


//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentStyleToDrawPolygon()
{
	this->MultiTouch = false;
	this->DrawPolygonTool = true;
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
InteractorStyleDisplayPolygon* InteractorStyleSelector::GetDisplayPolygonCamera()
{
	return this->DrawPolygonCamera;
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::OnChar()
{
	switch (this->Interactor->GetKeyCode())
	{
	case 'j':
	case 'J':
		this->JoystickOrTrackball = VTKIS_JOYSTICK;
		this->MultiTouch = false;
		this->DrawPolygonTool = false;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case 't':
	case 'T':
		this->JoystickOrTrackball = VTKIS_TRACKBALL;
		this->MultiTouch = false;
		this->DrawPolygonTool = false;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case 'c':
	case 'C':
		this->CameraOrActor = VTKIS_CAMERA;
		this->MultiTouch = false;
		this->DrawPolygonTool = false;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case 'a':
	case 'A':
		this->CameraOrActor = VTKIS_ACTOR;
		this->MultiTouch = false;
		this->DrawPolygonTool = false;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case 'm':
	case 'M':
		this->MultiTouch = true;
		this->DrawPolygonTool = false;
		this->EventCallbackCommand->SetAbortFlag(1);
		break;
	case '1':
		this->MultiTouch = false;
		this->DrawPolygonTool = true;
		this->EventCallbackCommand->SetAbortFlag(1);
	}
	// Set the CurrentStyle pointer to the picked style
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
// this will do nothing if the CurrentStyle matches
// JoystickOrTrackball and CameraOrActor
// It should! If the this->Interactor was changed (using SetInteractor()),
// and the currentstyle should not change.
void InteractorStyleSelector::SetCurrentStyle()
{
	// if the currentstyle does not match JoystickOrTrackball
	// and CameraOrActor ivars, then call SetInteractor(0)
	// on the Currentstyle to remove all of the observers.
	// Then set the Currentstyle and call SetInteractor with
	// this->Interactor so the callbacks are set for the
	// currentstyle.
	if (this->DrawPolygonTool)
	{
		if (this->CurrentStyle != this->DrawPolygonCamera)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->DrawPolygonCamera;
		}
	}
	else if (this->MultiTouch)
	{
		if (this->CurrentStyle != this->MultiTouchCamera)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->MultiTouchCamera;
		}
	}
	else if (this->JoystickOrTrackball == VTKIS_JOYSTICK &&
		this->CameraOrActor == VTKIS_CAMERA)
	{
		if (this->CurrentStyle != this->JoystickCamera)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->JoystickCamera;
		}
	}
	else if (this->JoystickOrTrackball == VTKIS_JOYSTICK &&
		this->CameraOrActor == VTKIS_ACTOR)
	{
		if (this->CurrentStyle != this->JoystickActor)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->JoystickActor;
		}
	}
	else if (this->JoystickOrTrackball == VTKIS_TRACKBALL &&
		this->CameraOrActor == VTKIS_CAMERA)
	{
		if (this->CurrentStyle != this->TrackballCamera)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->TrackballCamera;
		}
	}
	else if (this->JoystickOrTrackball == VTKIS_TRACKBALL &&
		this->CameraOrActor == VTKIS_ACTOR)
	{
		if (this->CurrentStyle != this->TrackballActor)
		{
			if (this->CurrentStyle)
			{
				this->CurrentStyle->SetInteractor(nullptr);
			}
			this->CurrentStyle = this->TrackballActor;
		}
	}
	if (this->CurrentStyle)
	{
		this->CurrentStyle->SetInteractor(this->Interactor);
		this->CurrentStyle->SetTDxStyle(this->TDxStyle);
	}
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetInteractor(vtkRenderWindowInteractor *iren)
{
	if (iren == this->Interactor)
	{
		return;
	}
	// if we already have an Interactor then stop observing it
	if (this->Interactor)
	{
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
	}
	this->Interactor = iren;
	// add observers for each of the events handled in ProcessEvents
	if (iren)
	{
		iren->AddObserver(vtkCommand::CharEvent,
			this->EventCallbackCommand,
			this->Priority);

		iren->AddObserver(vtkCommand::DeleteEvent,
			this->EventCallbackCommand,
			this->Priority);
	}
	this->SetCurrentStyle();
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "CurrentStyle " << this->CurrentStyle << "\n";
	if (this->CurrentStyle)
	{
		vtkIndent next_indent = indent.GetNextIndent();
		os << next_indent << this->CurrentStyle->GetClassName() << "\n";
		this->CurrentStyle->PrintSelf(os, indent.GetNextIndent());
	}
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetDefaultRenderer(vtkRenderer* renderer)
{
	this->vtkInteractorStyle::SetDefaultRenderer(renderer);
	this->JoystickActor->SetDefaultRenderer(renderer);
	this->JoystickCamera->SetDefaultRenderer(renderer);
	this->TrackballActor->SetDefaultRenderer(renderer);
	this->TrackballCamera->SetDefaultRenderer(renderer);
}

//----------------------------------------------------------------------------
void InteractorStyleSelector::SetCurrentRenderer(vtkRenderer* renderer)
{
	this->vtkInteractorStyle::SetCurrentRenderer(renderer);
	this->JoystickActor->SetCurrentRenderer(renderer);
	this->JoystickCamera->SetCurrentRenderer(renderer);
	this->TrackballActor->SetCurrentRenderer(renderer);
	this->TrackballCamera->SetCurrentRenderer(renderer);
}

