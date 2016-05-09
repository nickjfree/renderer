#ifndef __LINEAR_MATH__
#define __LINEAR_MATH__

/*
	Math inplement with dorectx math
*/

#include  "DirectXMath.h"
using namespace DirectX;


struct Quaternion;
struct Matrix4x4;

__declspec(align(16)) struct Vector2 {
	union {
		XMVECTOR vector;
		struct {
			float x, y;
		};
	};

	Vector2() {
	}

	Vector2(float x_, float y_) {
		x = x_;
		y = y_;
	}

	Vector2 operator + (Vector2& rh) {
		XMVECTOR result = vector + rh.vector;
		return *(Vector2 *)&result;
	}

	void operator += (Vector2& rh) {
		vector += rh.vector;
	}

	Vector2 operator - (Vector2& rh) {
		XMVECTOR result = vector - rh.vector;
		return *(Vector2 *)&result;
	}

	void operator -= (Vector2& rh) {
		vector -= rh.vector;
	}

	Vector2 operator * (float scalar) {
		XMVECTOR result = vector * scalar;
		return *(Vector2*)&result;
	}

	void Normalize() {
		vector = XMVector2Normalize(vector);
	}

	float Length() {
		return XMVectorGetX(XMVector2Length(vector));
	}

	float LengthSq() {
		return XMVectorGetX(XMVector2LengthSq(vector));
	}



	/*
	static functions
	*/
	inline static float Distance(Vector2& lh, Vector2& rh) {
		XMVECTOR result = XMVectorSubtract(lh.vector, rh.vector);
		return XMVectorGetX(XMVector2Length(result));
	}

};

__declspec(align(16)) struct Vector3 {
	union {
		XMVECTOR vector;
		struct {
			float x, y, z;
		};
	};

	Vector3() {
	}

	Vector3(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}

	Vector3 operator + (Vector3& rh) {
		XMVECTOR result = vector + rh.vector;
		return *(Vector3 *)&result;
	}

	void operator += (Vector3& rh) {
		vector +=rh.vector;
	}

	Vector3 operator - (Vector3& rh) {
		XMVECTOR result = vector - rh.vector;
		return *(Vector3 *)&result;
	}

	void operator -= (Vector3& rh) {
		vector -= rh.vector;
	}
	
	Vector3 operator * (float scalar) {
		XMVECTOR result = vector * scalar;
		return *(Vector3*)&result;
	}

	Vector3 Vector3::operator*(Quaternion& rh);

	Vector3 Vector3::operator*(Matrix4x4& rh);

	float Dot(Vector3& rh) {
		XMVECTOR result = XMVector3Dot(vector, rh.vector);
		return XMVectorGetX(result);
	}

	void Normalize() {
		vector = XMVector3Normalize(vector);
	}

	float Length() {
		return XMVectorGetX(XMVector3Length(vector));
	}

	float LengthSq() {
		return XMVectorGetX(XMVector3LengthSq(vector));
	}



	/*
		static functions
	*/
	inline static float Distance(Vector3& lh, Vector3& rh) {
		XMVECTOR result = XMVectorSubtract(lh.vector, rh.vector);
		return XMVectorGetX(XMVector3Length(result));
	}

};


__declspec(align(16)) struct Quaternion {
	union {
		XMVECTOR quaternion;
		struct {
			float x, y, z, w;
		};
	};

	Quaternion() {
		quaternion = XMQuaternionIdentity();
	}

	void RotationYawPitchRoll(float Pitch, float Yaw, float Roll) {
		quaternion = XMQuaternionRotationRollPitchYaw(Pitch, Yaw, Roll);
	}

	Quaternion operator * (Quaternion& rh) {
		Quaternion result;
		result.quaternion = XMQuaternionMultiply(quaternion, rh.quaternion);
		return result;
	}

	void RotationNormal(Vector3& Normal, float angle) {
		quaternion = XMQuaternionRotationNormal(Normal.vector, angle);
	}

	void RotationAxis(Vector3& Vector, float angle) {
		quaternion = XMQuaternionRotationAxis(Vector.vector, angle);
	}
};


__declspec(align(16)) struct Matrix4x4 {
	XMMATRIX matrix;

	Matrix4x4() {
		matrix = XMMatrixIdentity();
	}

	Matrix4x4 operator * (Matrix4x4& rh) {
		XMMATRIX result = matrix * rh.matrix;
		return *(Matrix4x4*)&result;
	}

	void Translate(float x, float y, float z) {
		matrix = XMMatrixTranslation(x, y, z);
	}

	void Translate(Vector3& translation) {
		matrix = XMMatrixTranslationFromVector(translation.vector);
	}

	void Identity() {
		matrix = XMMatrixIdentity();
	}

	void Tranform(Vector3& Position, Quaternion& rh);

	void Scale(Vector3& Scale) {
		matrix = XMMatrixScalingFromVector(Scale.vector);
	}
	/*
		static functions
	*/

	inline static void Inverse(Matrix4x4& in, Matrix4x4 * out) {
		out->matrix = XMMatrixInverse(0, in.matrix);
	}

	inline static void Tranpose(Matrix4x4& in, Matrix4x4 * out) {
		out->matrix = XMMatrixTranspose(in.matrix);
	}

	inline static Matrix4x4 PerspectiveFovLH(float fov, float aspect, float nearZ, float farZ) {
		XMMATRIX result = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
		return *(Matrix4x4*)&result;
	}

	inline static Matrix4x4 LookAtLH(Vector3& Look, Vector3& Up, Vector3& Right, Vector3& Position) {
		
		XMMATRIX result = XMMatrixLookToLH(Position.vector, Look.vector, Up.vector);
		return *(Matrix4x4*)&result;
	}

	inline static Matrix4x4 ViewMatrix(Vector3& Position, Quaternion& Orientation) {
		XMFLOAT4X4 View;
		// calculate matrix terms
		float twoXSquared = 2 * Orientation.x * Orientation.x;
		float twoYSquared = 2 * Orientation.y * Orientation.y;
		float twoZSquared = 2 * Orientation.z * Orientation.z;
		float twoXY = 2 * Orientation.x * Orientation.y;
		float twoWZ = 2 * Orientation.w * Orientation.z;
		float twoXZ = 2 * Orientation.x * Orientation.z;
		float twoWY = 2 * Orientation.w * Orientation.y;
		float twoYZ = 2 * Orientation.y * Orientation.z;
		float twoWX = 2 * Orientation.w * Orientation.x;

		// update view matrix orientation
		View._11 = 1 - twoYSquared - twoZSquared;
		View._12 = twoXY + twoWZ;
		View._13 = twoXZ - twoWY;
		View._14 = 0;
		View._21 = twoXY - twoWZ;
		View._22 = 1 - twoXSquared - twoZSquared;
		View._23 = twoYZ + twoWX;
		View._24 = 0;
		View._31 = twoXZ + twoWY;
		View._32 = twoYZ - twoWX;
		View._33 = 1 - twoXSquared - twoYSquared;
		View._34 = 0;

		// update view translation
		Vector3 right = Vector3(View._11, View._21, View._31);
		Vector3 up =  Vector3(View._12, View._22, View._32);
		Vector3 front = Vector3(View._13, View._23, View._33);
		View._41 = - right.Dot(Position);
		View._42 = - up.Dot(Position);
		View._43 = - front.Dot(Position);
		View._44 = 1;
		return *(Matrix4x4*)&View;
	}
};





#endif