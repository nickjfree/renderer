#ifndef __COLLISION__
#define __COLLISION__

#include "LinearMath.h"
#include "DirectXCollision.h"



using namespace DirectX;


enum ContainType
{
	DISJOINT = 0,
	INTERSECTS = 1,
	CONTAINS = 2,
};

enum PlaneIntersectType
{
	PLANE_FRONT = 0,
	PLANE_INTERSECTING = 1,
	PLANE_BACK = 2,
};



struct AABB;
struct Frustum;

/*
	AABB
*/
typedef struct AABB {
	BoundingBox Box;
	Vector3 InitExtents;
    Vector3 LocalCenter;
	AABB() {};
	AABB(Vector3& LocalCenter_, Vector3& Extents_) {
		XMStoreFloat3(&Box.Center, LocalCenter_.vector);
		XMStoreFloat3(&Box.Extents, Extents_.vector);
		InitExtents = Extents_;
        LocalCenter = LocalCenter_;
	};

	// contain test
	ContainType Contains(const AABB& box) {
		return (ContainType)Box.Contains(box.Box);
	};

	void Translate(Vector3& Position) {
		XMStoreFloat3(&Box.Center, Position.vector);
	}

	void UniformScale(float Scale) {
		XMStoreFloat3(&Box.Extents, (InitExtents * Scale).vector);
	}
}AABB;

/*
	Frustum
*/

typedef struct Frustum {
	BoundingFrustum fr;

	Frustum(){};

	Frustum(const Vector3& _Origin, const Quaternion& _Orientation,
	float _RightSlope, float _LeftSlope, float _TopSlope, float _BottomSlope,
	float _Near, float _Far) {
		
	};


	/*
		Method
	*/
	void Transform(Matrix4x4& Trans) {
		BoundingFrustum result;
		fr.Transform(result, Trans.matrix);
		fr = result;
	}

	ContainType Contains(const AABB& rh) {
		return (ContainType)fr.Contains(rh.Box);
	}

	ContainType Contains(const Frustum& rh) {
		return (ContainType)fr.Contains(rh.fr);
	}

	/*
		static
	*/
	static Frustum CreateFromProjection(Matrix4x4& Projection) {
		Frustum result;
		BoundingFrustum::CreateFromMatrix(result.fr, Projection.matrix);
		return result;
	}

	static Frustum CreateFromProjection(Vector3& Origin, Quaternion& Orientation, Matrix4x4& Projection) {
		Frustum result;
		BoundingFrustum temp;
		BoundingFrustum::CreateFromMatrix(result.fr, Projection.matrix);
		result.fr.Transform(temp, 1, Orientation.quaternion, Origin.vector);
		result.fr = temp;
		return result;
	}
}Frustum;



#endif