#include "StdAfx.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "GameCamera.h"

CGameCamera::CGameCamera(void)
{
	m_MoveSpeed = 10.0f/1000;
	m_RotationSpeed = 2.0f;
	m_NearPlane = 1.0f;
	m_FarPlane = 1000.0f;
	m_FovY = XM_PI * 0.3;
}

CGameCamera::~CGameCamera(void)
{
}


Matrix4x4 * CGameCamera::GetViewMatrix()
{
	D3DXVec3Cross((D3DXVECTOR3*)&m_Right,(D3DXVECTOR3*)&m_Look,(D3DXVECTOR3*)&m_Up);

	float x = -D3DXVec3Dot((D3DXVECTOR3*)&m_Position,(D3DXVECTOR3*)&m_Right);
	float y = -D3DXVec3Dot((D3DXVECTOR3*)&m_Position,(D3DXVECTOR3*)&m_Up);
	float z = -D3DXVec3Dot((D3DXVECTOR3*)&m_Position,(D3DXVECTOR3*)&m_Look);

	D3DXMATRIX &Mat = *(D3DXMATRIX*)&m_ViewMatrix;
	
	Mat(0,0) = m_Right._x;
	Mat(1,0) = m_Right._y;
	Mat(2,0) = m_Right._z;
	Mat(3,0) = x;

	Mat(0,1) = m_Up._x;
	Mat(1,1) = m_Up._y;
	Mat(2,1) = m_Up._z;
	Mat(3,1) = y;

	Mat(0,2) = m_Look._x;
	Mat(1,2) = m_Look._y;
	Mat(2,2) = m_Look._z;
	Mat(3,2) = z;

	Mat(0,3) = 0;
	Mat(1,3) = 0;
	Mat(2,3) = 0;
	Mat(3,3) = 1;
	return (Matrix4x4*)&m_ViewMatrix;
}

Matrix4x4 * CGameCamera::GetProjectionMatrix()
{
	return &m_ProjectionMatrix;
}

Matrix4x4 * CGameCamera::GetWorldMatrix()
{
	Matrix4x4 View = m_ViewMatrix;
	View._11 = - View._11;
	View._21 = - View._21;
	View._31 = - View._31;
	View._41 = - View._41;
	InverseMatrix(&m_WorldMatrix,&View);
	return &m_WorldMatrix;
}

 CCullingFrustum&  CGameCamera::GetFrustum()
 {
	 CCullingManager::GetCullingManager()->ConstructFrustum(&m_Frustum,m_Look,m_Right,m_Up,m_Position,
		 m_NearPlane,m_FarPlane,m_FovY,m_Aspect);
	 return m_Frustum;
 }


int CGameCamera::SetCamera(Vector3 * position,Vector3 * Look,Vector3 * up, float aspect)
{
	m_Aspect = aspect;
	m_Position = * position;
	m_Look = * Look;
	m_Up = * up;
	D3DXVec3Normalize((D3DXVECTOR3*)&m_Up,(D3DXVECTOR3*)&m_Up);
	D3DXVec3Normalize((D3DXVECTOR3*)&m_Look,(D3DXVECTOR3*)&m_Look);
	MatrixPerspectiveFovLH(&m_ProjectionMatrix,m_FovY,m_Aspect,m_NearPlane,m_FarPlane);
//	m_Render->SetViewMatrix(GetViewMatrix(),&m_Position);
	return 0;
}

int CGameCamera::Walk(int TimeDelt)
{
	m_Position = m_Position + m_Look * ((float)TimeDelt * m_MoveSpeed);
	return 0;
}

int CGameCamera::Fly(int TimeDelt)
{
	m_Position = m_Position + m_Up * ((float)TimeDelt * m_MoveSpeed);
	return 0;
}

int CGameCamera::Strafe(int TimeDelt)
{
	m_Position = m_Position + m_Right * ((float)TimeDelt * m_MoveSpeed);
	return 0;
}

int CGameCamera::Pitch(int TimeDelt)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R,(D3DXVECTOR3*)&m_Right,TimeDelt * m_RotationSpeed/1000.f);

	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Look,(D3DXVECTOR3*)&m_Look,&R);
	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Up,(D3DXVECTOR3*)&m_Up,&R);
	return 0;
}

int CGameCamera::Yaw(int TimeDelt)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R,(D3DXVECTOR3*)&m_Up,TimeDelt * m_RotationSpeed/1000.f);

	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Look,(D3DXVECTOR3*)&m_Look,&R);
	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Right,(D3DXVECTOR3*)&m_Up,&R);
	return 0;
}

int CGameCamera::Roll(int TimeDelt)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R,(D3DXVECTOR3*)&m_Look,TimeDelt * m_RotationSpeed/1000.f);

	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Right,(D3DXVECTOR3*)&m_Look,&R);
	D3DXVec3TransformNormal((D3DXVECTOR3*)&m_Up,(D3DXVECTOR3*)&m_Up,&R);
	return 0;
}

int CGameCamera::KeyMove(int TimeDelt)
{
	//walk
	if(GetAsyncKeyState('W') & 0x8000)
		Walk(TimeDelt);
	if(GetAsyncKeyState('S') & 0x8000)
		Walk(-TimeDelt);
	//Strafe
	if(GetAsyncKeyState('D') & 0x8000)
		Strafe(TimeDelt);
	if(GetAsyncKeyState('A') & 0x8000)
		Strafe(-TimeDelt);
	//fly
	if(GetAsyncKeyState('M') & 0x8000)
		Fly(TimeDelt);
	if(GetAsyncKeyState('N') & 0x8000)
		Fly(-TimeDelt);
	//Yaw
	if(GetAsyncKeyState('Q' ) & 0x8000)
		Yaw(TimeDelt);
	if(GetAsyncKeyState('E') & 0x8000)
		Yaw(-TimeDelt);
	//Pitch
	if(GetAsyncKeyState(VK_UP) & 0x8000)
		Pitch(-TimeDelt);
	if(GetAsyncKeyState(VK_DOWN) & 0x8000)
		Pitch(TimeDelt);
	//Roll
	if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
		Roll(TimeDelt);
	if(GetAsyncKeyState(VK_LEFT) & 0x8000)
		Roll(-TimeDelt);
	if(GetAsyncKeyState('Z') & 0x8000)
		m_MoveSpeed += TimeDelt/100;
	if(GetAsyncKeyState('X') & 0x8000)
		m_MoveSpeed += -TimeDelt/100;
	return 0;
}

int CGameCamera::FreeMove(int TimeDelt)
{
	return 0;
}

Vector3 CGameCamera::GetCameraPos()
{
	Vector3 Vec;
	Vec = m_Position;
	return Vec;
}

Vector3 CGameCamera::GetLook(void)
{
	Vector3 Look;
	Look = m_Look;
	return Look;
}
