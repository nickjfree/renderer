#include "Camera.h"
#include "Rendering\RenderingSystem.h"

USING_ALLOCATER(Camera);

Camera::Camera(Context * context) :Component(context)
{
	RenderCamera = new RenderingCamera();
	RenderingSystem * Render = context->GetSubsystem<RenderingSystem>();
	Render->AddCamera(RenderCamera);
}


Camera::~Camera()
{
}

void Camera::Enable() {
}


void Camera::Disable() {

}

int Camera::HandleEvent(Event * Ev) {

	return 0;
}
