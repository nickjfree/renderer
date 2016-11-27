#include "RenderingCamera.h"

USING_ALLOCATER(RenderingCamera);


RenderingCamera::RenderingCamera()
{
	float pi = 3.141592654f;
	Position = Vector3(5, 4, -20);
	Projection = Matrix4x4::PerspectiveFovLH(0.3f*pi, 1920/1080.0f,1,1000);
	MoveSpeed = 0.05f;
	RotationSpeed = 0.1f;
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	ViewMatrix = Matrix4x4::LookAtLH(Look, Up, Right, Position);
	Matrix4x4::Inverse(ViewMatrix, &InvertView);
	ViewProjection = ViewMatrix * Projection;
}


RenderingCamera::~RenderingCamera()
{
}

void RenderingCamera::FromLight(Vector3& Position_, Quaternion& Orientation_, Matrix4x4& Projection_) {
	Projection = Projection_;
	Position = Position_;
	Orientation = Orientation_;
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	ViewMatrix = Matrix4x4::LookAtLH(Look, Up, Right, Position);
	Matrix4x4::Inverse(ViewMatrix, &InvertView);
	ViewProjection = ViewMatrix * Projection;
}

void RenderingCamera::CheckStatus() {
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	ViewMatrix = Matrix4x4::LookAtLH(Look, Up, Right, Position);
	Matrix4x4::Inverse(ViewMatrix, &InvertView);
	ViewProjection = ViewMatrix * Projection;
}

void RenderingCamera::SetTransform(Vector3& Transform) {
	Position = Transform;
	CheckStatus();
}

void RenderingCamera::SetRotation(Quaternion& Rotation) {
	Orientation = Rotation;
	CheckStatus();
}

void RenderingCamera::SetProjection(Matrix4x4& Projection_) {
	Projection = Projection;
	CheckStatus();
}

Frustum& RenderingCamera::GetFrustum() {
	frustum = Frustum::CreateFromProjection(Position, Orientation, Projection);
	//frustum = Frustum::CreateFromProjection(Projection);
	return frustum;
}

void RenderingCamera::Update(int ms) {
	//TestMove(ms);
	//ViewMatrix = Matrix4x4::LookAtLH(Look, Up, Right, Position);
	//Matrix4x4::Inverse(ViewMatrix, &InvertView);
	//ViewProjection = ViewMatrix * Projection;
}

int RenderingCamera::Walk(int TimeDelt)
{
	Position = Position + Look * ((float)TimeDelt * MoveSpeed);
	return 0;
}

int RenderingCamera::Fly(int TimeDelt)
{
	Position = Position + Up * ((float)TimeDelt * MoveSpeed);
	return 0;
}

int RenderingCamera::Strafe(int TimeDelt)
{
	Position = Position + Right * ((float)TimeDelt * MoveSpeed);
	return 0;
}

int RenderingCamera::Pitch(int TimeDelt)
{

	float Degrees = TimeDelt * RotationSpeed;
	float angle = Degrees* (3.1415926f / 180.0f);
	Quaternion rot;
	Right.Normalize();
	rot.RotationNormal(Right, angle);
	Orientation = Orientation * rot;
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	return 0;
}

int RenderingCamera::Yaw(int TimeDelt)
{
	float Degrees = TimeDelt * RotationSpeed;
	float angle = Degrees* (3.1415926f / 180.0f);

	Quaternion rot;
	Up.Normalize();
	rot.RotationNormal(Up, angle);
	Orientation = Orientation * rot;
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	return 0;
}

int RenderingCamera::Roll(int TimeDelt)
{
	float Degrees = TimeDelt * RotationSpeed;
	float angle = Degrees* (3.1415926f / 180.0f);
	Quaternion rot;
	Look.Normalize();
	rot.RotationNormal(Look, angle);
	Orientation = Orientation * rot;
	Look = Vector3(0, 0, 1) * Orientation;
	Up = Vector3(0, 1, 0) * Orientation;
	Right = Vector3(1, 0, 0) * Orientation;
	return 0;
}

void RenderingCamera::TestMove(int Delta) {
	//walk
	if (GetAsyncKeyState('W') & 0x8000)
		Walk(Delta);
	if (GetAsyncKeyState('S') & 0x8000)
		Walk(-Delta);
	//Strafe
	if (GetAsyncKeyState('D') & 0x8000)
		Strafe(Delta);
	if (GetAsyncKeyState('A') & 0x8000)
		Strafe(-Delta);
	//fly
	if (GetAsyncKeyState('M') & 0x8000)
		Fly(Delta);
	if (GetAsyncKeyState('N') & 0x8000)
		Fly(-Delta);
	//Yaw
	if (GetAsyncKeyState('Q') & 0x8000)
		Yaw(-Delta);
	if (GetAsyncKeyState('E') & 0x8000)
		Yaw(Delta);
	//Pitch
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		Pitch(Delta);
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		Pitch(-Delta);
	//Roll
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		Roll(-Delta);
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		Roll(Delta);
}

