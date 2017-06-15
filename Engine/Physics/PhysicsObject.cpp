#include "PhysicsObject.h"
#include "Scene\GameObject.h"
#include "PhysicsSystem.h"


USING_ALLOCATER(PhysicsObject)

PhysicsObject::PhysicsObject(Context * context):Component(context) {
	// create a default rigitbody
	Physics = context->GetSubsystem<PhysicsSystem>();
	World = Physics->GetWorld();
}


PhysicsObject::~PhysicsObject() {
}

int PhysicsObject::OnAttach(GameObject * GameObj) {
	// get init position and rotation
	Vector3 Position = GameObj->GetWorldTranslation();
	Quaternion Rotation = GameObj->GetWorldRotation();
	btVector3 position = btVector3(Position.x, Position.y, Position.z);
	btQuaternion quaternion;
	btQuaternionFloatData data = { Rotation.x,Rotation.y,Rotation.z,Rotation.w,};
	quaternion.deSerializeFloat(data);
	btTransform transform = btTransform(quaternion, position);
	Shape.Shapes.Box = new btBoxShape(btVector3(2, 2, 2));
	MotionState = new btDefaultMotionState(transform);
	rigidBody = new btRigidBody(1, MotionState, Shape.Shapes.Box);
	// set this physics compoent to rigidbody userdataso we can use it at collison triggers
	rigidBody->setUserPointer(this);
	//add to list
	Physics->AddPhysicsObject(this);
	// add to world
	World->addRigidBody(rigidBody);
	return 0;
}


int PhysicsObject::Update(int ms) {
	// only deals with rigide bodies, read btTransform and set to gameobject
	btTransform transform;
	MotionState->getWorldTransform(transform);
	btVector3& position = transform.getOrigin();
	btQuaternion& quaternion = transform.getRotation();
	Vector3 Position = Vector3(position.x(), position.y(), position.z());
	Quaternion Rotation;
	Rotation.x = quaternion.x();
	Rotation.y = quaternion.y();
	Rotation.z = quaternion.z();
	Rotation.w = quaternion.w();
	Matrix4x4 Transform = Matrix4x4::FormPositionRotation(Position, Rotation);
    // set to gameobject
	Owner->SetRotation(Rotation);
	Owner->SetTranslation(Position);


	return 0;
}
