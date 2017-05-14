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
		SPHERE = 2
	};
	// the collisionShape type
	ShapeType Type;
	// the shapes
	union CollisionShapes{
		btBoxShape * Box;
		btSphereShape * Sphere;
		btCapsuleShape * Capsule;
	};
	CollisionShapes Shapes;

} CollisionShape;



class PhysicsObject : public Component {

	DECLAR_ALLOCATER(PhysicsObject);
	OBJECT(PhysicsObject);
	BASEOBJECT(PhysicsObject);

private:
	// shape
	CollisionShape Shape;
	// motion state
	btMotionState * MotionState;
	// rigidbody
	btRigidBody * rigidBody;
	// physics system
	PhysicsSystem * Physics;
	// world
	btDiscreteDynamicsWorld * World;
public:
	PhysicsObject(Context * context);
	virtual ~PhysicsObject();
	// update
	virtual int Update(int ms);
	// onattach
	virtual int OnAttach(GameObject * GameObj);


};

#endif