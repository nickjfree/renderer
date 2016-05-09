#ifndef __RENDERING_CAMERA_H__
#define __RENDERING_CAMERA_H__

#include "Math\LinearMath.h"
#include "Math\Collision.h"
#include "Core\Allocater.h"
#include "Core\Shortcuts.h"

/*
	RenderingCamera. Main Camera, shadow casting light camera, reflection camera and more.
	Camera used for rendering, not confused with camera component
*/
class RenderingCamera
{
	DECLAR_ALLOCATER(RenderingCamera);
private:
	float MoveSpeed;
	float RotationSpeed;
private:
	// test move
	void TestMove(int Delta);
	// private method
	int Walk(int TimeDelt);
	int Fly(int TimeDelt);
	int Strafe(int TimeDelt);
	int Pitch(int TimeDelt);
	int Yaw(int	TimeDelt);
	int Roll(int TimeDelt);
	int FreeMove(int TimeDelt);
	int KeyMove(int TimeDelt);

protected:
	Vector3 Position;
	Quaternion Orientation;
	Vector3 Look;
	Vector3 Up;
	Vector3 Right;
	Matrix4x4 Projection;
	Matrix4x4 ViewMatrix;
	Matrix4x4 ViewProjection;
	Matrix4x4 InvertView;
	Frustum frustum;
public:
	RenderingCamera();
	virtual ~RenderingCamera();
	Frustum& GetFrustum();
	void Update(int ms);
	Matrix4x4& GetViewMatrix() { return ViewMatrix; }
	Matrix4x4& GetViewProjection() { return ViewProjection; }
	Matrix4x4& GetProjection() { return Projection; }
	Matrix4x4& GetInvertView() { return InvertView; }
	Vector3& GetViewPoint() { return Position; }


};
#endif
