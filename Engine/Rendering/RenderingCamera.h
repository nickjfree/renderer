#ifndef __RENDERING_CAMERA_H__
#define __RENDERING_CAMERA_H__

#include "Math\LinearMath.h"
#include "Math\Collision.h"
#include "Core\Allocater.h"
#include "Core\Shortcuts.h"
#include "RenderContext.h"
// #include "Rendering\Node.h"

/*
	RenderingCamera. Main Camera, shadow casting light camera, reflection camera and more.
	Camera used for rendering, not confused with camera component
*/
class RenderingCamera
{
	DECLARE_ALLOCATER(RenderingCamera);
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
	void CheckStatus();

protected:
	Vector3 Position;
	Quaternion Orientation;
	Vector3 Look;
	Vector3 Up;
	Vector3 Right;
	// p
	Matrix4x4 Projection;
	// v
	Matrix4x4 ViewMatrix;
	// vp
	Matrix4x4 ViewProjection;
	// previous frame's vp
	Matrix4x4 PrevViewProjection;
	// previous frame's v
	Matrix4x4 PrevViewMatrix;
	// inversed v
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

	Matrix4x4& GetPrevViewMatrix() { return PrevViewMatrix; }
	Matrix4x4& GetPrevViewProjection() { return PrevViewProjection; }

	Vector3& GetViewPoint() { return Position; }
	void FromLight(Vector3& Position, Quaternion& Orientation, Matrix4x4& Projection);
	void SetTransform(Vector3& Transform);
	void SetRotation(Quaternion& Rotation);
	void SetProjection(Matrix4x4& Projection);

	void UpdatePrevMatrix();

};
#endif
