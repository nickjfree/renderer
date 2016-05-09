#ifndef __MATH_H__
#define __MATH_H__

#include "d3d11.h"
#include "DirectXmath.h"
#include "Math.h"

using namespace DirectX;

typedef struct _Vector3
{
	_Vector3(){};
	_Vector3(float x,float y,float z):_x(x),_y(y),_z(z){};
	_Vector3 operator + (_Vector3& Vec){
		_Vector3 ret;
		ret._x = this->_x + Vec._x;
		ret._y = this->_y + Vec._y;
		ret._z = this->_z + Vec._z;
		return ret;
	};
	_Vector3 operator * (float a){
		_Vector3 ret;
		ret._x = this->_x * a;
		ret._y = this->_y * a;
		ret._z = this->_z * a;
		return ret;
	};
	float operator * (_Vector3& Vec){
		//_Vector3 ret;
		//ret._x = this->_x * Vec._x + this->_y * Vec._y + this->_z * Vec._z;
		//ret._y = this->_y * Vec._y;
		//ret._z = this->_z * Vec._z;
		return this->_x * Vec._x + this->_y * Vec._y + this->_z * Vec._z;
//		return ret;
	};
	_Vector3 operator - (_Vector3& Vec){
		_Vector3 ret;
		ret._x = this->_x - Vec._x;
		ret._y = this->_y - Vec._y;
		ret._z = this->_z - Vec._z;
		return ret;
	};
	float _x,_y,_z;
}Vector3;


typedef struct _Vector4
{
	float x, y, z, w;
}Vector4;


int Vector3Cross(Vector3 * VecRes,Vector3 * Vec1,Vector3 * Vec2);
int Vector3Normal(Vector3 * VecRes);


typedef struct _Matrix4x4
{
	union{
	  struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
	};
	_Matrix4x4 operator * (_Matrix4x4 & Mat){
		Matrix4x4 Res;
		for(int i = 0;i < 4;i++)
		{
			for(int j = 0;j < 4;j++)
			{
				Res.m[i][j] = 0;
				for(int n = 0;n < 4;n++)
				{
					Res.m[i][j] += this->m[i][n] * Mat.m[n][j];
				}
			}
		}
		return Res;
	};
	void Tansform(float x,float y,float z)
	{
		memset(this,0,sizeof(Matrix4x4));
		this->_11 = this->_22 = this->_33 = this->_44 = 1;
		this->_41 = x;
		this->_42 = y;
		this->_43 = z;
	};
	void Scale(float x,float y,float z)
	{
		memset(this,0,sizeof(Matrix4x4));
		this->_11 = x;
		this->_22 = y;
		this->_33 = z;
		this->_44 = 1;
	}
	void RotX(float a)
	{
		float sina = sin(a);
		float cosa = cos(a);
		memset(this,0,sizeof(Matrix4x4));
		this->_11 = this->_44 = 1;
		this->_22 = this->_33 = cosa;
		this->_32 = -sina;
		this->_23 = sina;
	};
	void RotY(float a)
	{
		float sina = sin(a);
		float cosa = cos(a);
		memset(this,0,sizeof(Matrix4x4));
		this->_22 = this->_44 = 1;
		this->_11 = this->_33 = cosa;
		this->_13 = -sina;
		this->_31 = sina;
	};
	void RotZ(float a)
	{
		float sina = sin(a);
		float cosa = cos(a);
		memset(this,0,sizeof(Matrix4x4));
		this->_33 = this->_44 = 1;
		this->_11 = this->_22 = cosa;
		this->_12 = sina;
		this->_21 = -sina;
	};
	void RotXYZ(float x,float y,float z)
	{
		Matrix4x4 Rot;
		Rot.RotX(x);
		*this = (*this) * Rot;
		Rot.RotY(y);
		*this = (*this) * Rot;
		Rot.RotZ(z);
		*this = (*this) * Rot;
	};
	void Transpose()
	{
		for(int i = 0;i < 4;i++)
		{
			for(int j = 0;j < 4;j++)
			{
				float temp = m[i][j];
				m[i][j] = m[j][i];
				m[j][i] = temp;
			}
		}
	};
	void Identity()
	{
		memset(this,0,sizeof(Matrix4x4));
		this->_11 = this->_22 = this->_33 = this->_44 = 1;
	};
}Matrix4x4;


typedef struct _Quaternion
{
	float w, x, y, z;
}Quaternion;


int InverseMatrix(Matrix4x4 * Inv,Matrix4x4 * Mat);
Matrix4x4 * MatrixPerspectiveFovLH(Matrix4x4* Pers, float FovY, float Aspect, float NearPlane,float FarPlane);
Matrix4x4 * QuaternionToMatrix(Quaternion & q, Matrix4x4 * Matrix);





#endif