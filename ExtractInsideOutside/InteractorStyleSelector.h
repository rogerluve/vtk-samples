#pragma once

#include "vtkInteractorStyleSwitchBase.h"

#define VTKIS_JOYSTICK  0
#define VTKIS_TRACKBALL 1

#define VTKIS_CAMERA    0
#define VTKIS_ACTOR     1

class vtkInteractorStyleJoystickActor;
class vtkInteractorStyleJoystickCamera;
class vtkInteractorStyleTrackballActor;
class vtkInteractorStyleTrackballCamera;
class vtkInteractorStyleMultiTouchCamera;
class InteractorStyleDisplayPolygon;

class InteractorStyleSelector: public vtkInteractorStyleSwitchBase
{
public:
	static InteractorStyleSelector *New();
	vtkTypeMacro(InteractorStyleSelector, vtkInteractorStyleSwitchBase);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	/**
	 * The sub styles need the interactor too.
	 */
	void SetInteractor(vtkRenderWindowInteractor *iren) override;

	/**
	 * We must override this method in order to pass the setting down to
	 * the underlying styles
	 */
	void SetAutoAdjustCameraClippingRange(vtkTypeBool value) override;

	//@{
	/**
	 * Set/Get current style
	 */
	vtkGetObjectMacro(CurrentStyle, vtkInteractorStyle);
	void SetCurrentStyleToJoystickActor();
	void SetCurrentStyleToJoystickCamera();
	void SetCurrentStyleToTrackballActor();
	void SetCurrentStyleToTrackballCamera();
	void SetCurrentStyleToMultiTouchCamera();
	void SetCurrentStyleToDrawPolygon();
	InteractorStyleDisplayPolygon* GetDisplayPolygonCamera();
	//@}

	/**
	 * Only care about the char event, which is used to switch between
	 * different styles.
	 */
	void OnChar() override;

	//@{
	/**
	 * Overridden from vtkInteractorObserver because the interactor styles
	 * used by this class must also be updated.
	 */
	void SetDefaultRenderer(vtkRenderer*) override;
	void SetCurrentRenderer(vtkRenderer*) override;
	//@}

protected:
	InteractorStyleSelector();
	~InteractorStyleSelector() override;

	void SetCurrentStyle();

	vtkInteractorStyleJoystickActor *JoystickActor;
	vtkInteractorStyleJoystickCamera *JoystickCamera;
	vtkInteractorStyleTrackballActor *TrackballActor;
	vtkInteractorStyleTrackballCamera *TrackballCamera;
	vtkInteractorStyleMultiTouchCamera *MultiTouchCamera;
	InteractorStyleDisplayPolygon *DrawPolygonCamera;

	vtkInteractorStyle* CurrentStyle;

	int JoystickOrTrackball;
	int CameraOrActor;
	
	bool DrawPolygonTool;
	bool MultiTouch;

private:
	InteractorStyleSelector(const InteractorStyleSelector&) = delete;
	void operator=(const InteractorStyleSelector&) = delete;
};