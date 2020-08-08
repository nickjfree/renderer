#ifndef __NODE_H__
#define __NODE_H__

#include "Core\Allocater.h"
#include "Core\Shortcuts.h"
#include "Container\LinkList.h"
#include "Math\Collision.h"
#include "Math\LinearMath.h"
#include "Container\Vector.h"
#include "BatchCompiler.h"
#include "RenderingCamera.h"

/*
	spatial tree node
*/
typedef AABB CullingObject;

class RenderingCamera;

class Node
{
	DECLARE_ALLOCATER(Node);
	friend Node;
protected:
	// positions
	Vector3 Position;
	// rotation
	Quaternion Rotation;
	// up right look vector
	Vector3 Up;

	Vector3 Right;

	Vector3 Look;

	// AABB
	CullingObject CullingObj;
	// subnode of this node
	LinkList<Node> SubNodes;
	// sibline node
	LinkList<Node> Sibling;
	// node type
	int Type;
	// world matrix
	Matrix4x4 Transform;
	// raytracing geometry id
	int RaytracingGeometry;
	// deforamble buffer id
	int DeformableBuffer;

public:
	enum {
		NODE = 1,
		RENDEROBJECT = 2,
		LIGHT = 4,
		REFLECT = 8,
		PORTAL = 16,
		TRANS = 32,
		TERRAIN = 64,
		NO_CULL = 128,
		POST
	};

private:
	// update culling object
	void SyncCullingObject();

public:
	Node();
	virtual ~Node();
	const CullingObject& GetCullingObject() const { return CullingObj; }
	int SetCullingObject(const CullingObject& obj) { CullingObj = obj; return 0; }
	void AddChild(Node* Child);
	void Remove(Node* Child);
	// query by frustum and types
	virtual int Query(Frustum& Fr, Vector<Node*>& Result, int Types, bool inside);
	// query only by types
	virtual int Query(Vector<Node*>& Result, int Types);
	Matrix4x4& GetWorldMatrix();
	virtual int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context);
	virtual int UpdateRaytracingStructure(RenderContext* Context);
	void SetPosition(Vector3& Position_);
	void SetRotation(Quaternion& Rot);
	Vector3& GetPosition() { return Position; }
	Quaternion& GetRotation() { return Rotation; }
};


#endif