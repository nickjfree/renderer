#ifndef __PHYSICS_COMPONENT__
#define __PHYSICS_COMPONENT__

#include "Scene\Component.h"
#include "PhysicsSystem.h"




class PhysicsSystem;

/*
	using bullet physics
*/
typedef struct CollisionShape {
	enum ShapeType {
		BOX = 1,
		SPHERE = 2,
		COMPOUND = 3,
	};
	// the collisionShape type
	ShapeType Type;
	// the shapes
	union CollisionShapes{
		btBoxShape * Box;
		btSphereShape * Sphere;
		btCapsuleShape * Capsule;
		btCompoundShape * Compound;
	};
	CollisionShapes Shapes;

	void CreateFromConvexData(int Clusters, unsigned int * VNum, unsigned int * INum, float ** VBuffer, unsigned int ** IBuffer, Vector3 * Centers);
} CollisionShape;



class PhysicsObject : public Component {

	DECLAR_ALLOCATER(PhysicsObject);
	OBJECT(PhysicsObject);
	BASEOBJECT(PhysicsObject);

private:
	// shape
	CollisionShape * Shape;
	// motion state
	btMotionState * MotionState;
	// rigidbody
	btRigidBody * rigidBody;
	// physics system
	PhysicsSystem * Physics;
	// world
	btDiscreteDynamicsWorld * World;
	// center
	Vector3 CenterOffset;
	// invers transform of center offset
	Matrix4x4 InvertCenter;
public:
	enum Type{
		DYNAMIC = 0,
		STATIC = 1,
		KINEMATIC = 2,
	};
	// object type
	int ObjectType;
public:
	PhysicsObject(Context * context);
	virtual ~PhysicsObject();
	// update
	virtual int Update(int ms);
	// onattach
	virtual int OnAttach(GameObject * GameObj);
	// set shape
	void SetCollisionShape(CollisionShape * Shape_) { Shape = Shape_;}
	// crate shape from model
	void CreateShapeFromModel(Model * model);
	// on transform
	virtual int OnTransform(GameObject * object);
	// set type
	void SetObjectType(PhysicsObject::Type  type);
};

#endif