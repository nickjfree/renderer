#ifndef __GAME_CAMERA__
#define __GAME_CAMERA__

#include "d3d11.h"
#include "IMRender.h"
#include "CullingManager.h"


using namespace CullingSystem;

class CGameCamera
{
private:
	Vector3 m_Position;
	Vector3 m_Look;
	Vector3 m_Up;
	Vector3 m_Right;
	//
	Matrix4x4  m_ViewMatrix;
	Matrix4x4  m_ProjectionMatrix;
	Matrix4x4  m_WorldMatrix;
	float  m_MoveSpeed;
	float  m_RotationSpeed;

	float  m_NearPlane;
	float  m_FarPlane;
	float  m_FovY;
	float  m_Aspect;
	// frustum
	CCullingFrustum m_Frustum;
public:
	CGameCamera(void);
	~CGameCamera(void);
	 int SetCamera(Vector3 * position,Vector3 * Look,Vector3 * up, float aspect);
	 int Walk(int TimeDelt);
	 int Fly(int TimeDelt);
	 int Strafe(int TimeDelt);
	 int Pitch(int TimeDelt);
	 int Yaw(int	TimeDelt);
	 int Roll(int TimeDelt);
	 int FreeMove(int TimeDelt);
	 int KeyMove(int TimeDelt);
	 Vector3 GetCameraPos();
	 Matrix4x4 * GetViewMatrix();
	 Matrix4x4 * GetWorldMatrix();
	 Matrix4x4 * GetProjectionMatrix();
	 CCullingFrustum &GetFrustum();

private:
	//funcs
	
public:
	Vector3 GetLook(void);
};

#endif