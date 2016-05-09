//#include <stdafx.h>
#include "Mathlib.h"


void swapf(float &a,float &b)
{
	float t = a;
	a = b;
	b = t;
}

int Vector3Cross(Vector3 * VecRes,Vector3 * Vec1,Vector3 * Vec2)
{
	VecRes->_x = Vec1->_y * Vec2->_z - Vec1->_z * Vec2->_y;
	VecRes->_y = Vec1->_z * Vec2->_x - Vec1->_x * Vec2->_z;
	VecRes->_z = Vec1->_x * Vec2->_y - Vec1->_y * Vec2->_x;
	return 0;
}

float Vector3Dot(Vector3 * Vec1, Vector3 * Vec2)
{
	return 0;
}

int Vector3Normal(Vector3 * VecRes)
{
	float len = pow(VecRes->_x * VecRes->_x + VecRes->_y * VecRes->_y + VecRes->_z * VecRes->_z,0.5f);
	VecRes->_x = VecRes->_x/len;
	VecRes->_y = VecRes->_y/len;
	VecRes->_z = VecRes->_z/len;
	return 0;
}

int InverseMatrix(Matrix4x4 * Inv,Matrix4x4 * Mat)
{
	XMMATRIX XMat = XMLoadFloat4x4((XMFLOAT4X4*)Mat);
	XMVECTOR Dedermine;
	XMat = XMMatrixInverse(&Dedermine,XMat);
	XMStoreFloat4x4((XMFLOAT4X4*)Inv, XMat);
	return 0;
}

Matrix4x4 * MatrixPerspectiveFovLH(Matrix4x4* Pers, float FovY, float Aspect, float NearPlane,float FarPlane)
{
	 XMMATRIX PersMatrix = XMMatrixPerspectiveFovLH(FovY,Aspect,NearPlane,FarPlane);
	 XMStoreFloat4x4((XMFLOAT4X4 * )Pers, PersMatrix);
	 return Pers;
}

Matrix4x4 * QuaternionToMatrix(Quaternion & q, Matrix4x4 * Matrix)
{
	double sqw = q.w*q.w;
	double sqx = q.x*q.x;
	double sqy = q.y*q.y;
	double sqz = q.z*q.z;
	Matrix->Identity();
	// invs (inverse square length) is only required if quaternion is not already normalised
	double invs = 1 / (sqx + sqy + sqz + sqw);
	Matrix->m[0][0] = (sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
	Matrix->m[1][1] = (-sqx + sqy - sqz + sqw)*invs;
	Matrix->m[2][2] = (-sqx - sqy + sqz + sqw)*invs;

	double tmp1 = q.x*q.y;
	double tmp2 = q.z*q.w;
	Matrix->m[1][0] = 2.0 * (tmp1 + tmp2)*invs;
	Matrix->m[0][1] = 2.0 * (tmp1 - tmp2)*invs;

	tmp1 = q.x*q.z;
	tmp2 = q.y*q.w;
	Matrix->m[2][0] = 2.0 * (tmp1 - tmp2)*invs;
	Matrix->m[0][2] = 2.0 * (tmp1 + tmp2)*invs;
	tmp1 = q.y*q.z;
	tmp2 = q.x*q.w;
	Matrix->m[2][1] = 2.0 * (tmp1 + tmp2)*invs;
	Matrix->m[1][2] = 2.0 * (tmp1 - tmp2)*invs;
	return Matrix;
}


