#include "PhysicsObject.h"
#include "Scene\GameObject.h"
#include "PhysicsSystem.h"


USING_ALLOCATER(PhysicsObject)




void CollisionShape::CreateFromConvexData(int Clusters, unsigned int * VNum, unsigned int * INum, float ** VBuffer, unsigned int ** IBuffer, Vector3 * Centers) {
}





PhysicsObject::PhysicsObject(Context * context):Component(context), Shape(0), ObjectType(0) {
	// create a default rigitbody
	Physics = context->GetSubsystem<PhysicsSystem>();
	World = Physics->GetWorld();
	CenterOffset = Vector3(0, 0, 0);
	InvertCenter.Identity();
}


PhysicsObject::~PhysicsObject() {
	Clear();
}

int PhysicsObject::OnAttach(GameObject * GameObj) {
	// get init position and rotation
	Vector3 Position = GameObj->GetWorldTranslation();
	Quaternion Rotation = GameObj->GetWorldRotation();
	// offset center of mass to physics center
	Position = CenterOffset * Rotation + Position;
	btVector3 position = btVector3(Position.x, Position.y, Position.z);
	btQuaternion quaternion;
	btQuaternionFloatData data = { Rotation.x,Rotation.y,Rotation.z,Rotation.w,};
	quaternion.deSerializeFloat(data);
	btTransform transform = btTransform(quaternion, position);
	if (!Shape) {
		Shape = new CollisionShape();
		Shape->Shapes.Box = new btBoxShape(btVector3(2, 2, 2));
		Shape->Shared = 0;
	}
	MotionState = new btDefaultMotionState(transform);
	btVector3 Inertia;
	Shape->Shapes.Compound->calculateLocalInertia(1, Inertia);
	float mass = 1;
	if (ObjectType) {
		mass = 0;
	}
	rigidBody = new btRigidBody(mass, MotionState, Shape->Shapes.Compound, Inertia);
	// set this physics compoent to rigidbody userdataso we can use it at collison triggers
	if (ObjectType == Type::KINEMATIC) {
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
	rigidBody->setUserPointer(this);
	//add to list
	Physics->AddPhysicsObject(this);
	// add to world
	World->addRigidBody(rigidBody);
	return 0;
}


int PhysicsObject::Update(int ms) {
	// only deals with rigide bodies, read btTransform and set to gameobject
	if (rigidBody->isKinematicObject()) {
		// skip kinematic objects
		return 1;
	}
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
    // set to gameobject, the invert is used to offset back to gameobject root
	Owner->SetTransform(InvertCenter * Transform);
	// Owner->SetRotation(Rotation);
	// Owner->SetTranslation(Position);


	return 0;
}

int PhysicsObject::OnTransform(GameObject * object) {
	if (rigidBody->isKinematicObject()) {
		Vector3 Position = object->GetWorldTranslation();
		Quaternion Rotation = object->GetWorldRotation();
		// offset to real position of physics object
		Position = CenterOffset * Rotation + Position;
		btVector3 position = btVector3(Position.x, Position.y, Position.z);
		btQuaternion quaternion;
		btQuaternionFloatData data = { Rotation.x,Rotation.y,Rotation.z,Rotation.w, };
		quaternion.deSerializeFloat(data);
		btTransform transform = btTransform(quaternion, position);
		MotionState->setWorldTransform(transform);
	}
	return 0;
}

void PhysicsObject::CreateShapeFromModel(Model * model) {

	Shape = (CollisionShape *)model->GetUserData();
	MeshConvex * ConvexHulls = model->GetMesh(0)->ConvexHulls;
	int Clusters = model->GetMesh(0)->NumConvex;
	// get the compund object center
	for (int i = 0; i < Clusters; i++) {
		CenterOffset = CenterOffset + ConvexHulls[i].Center;
	}
	if (Clusters) {
		CenterOffset = CenterOffset * (1.0f / Clusters);
		InvertCenter.Translate(CenterOffset * -1.0f);
	}
	if (!Shape) {
		if (Clusters) {
			Shape = new CollisionShape();
			btCompoundShape * Compound = new btCompoundShape();
			for (int i = 0; i < Clusters; i++) {
				btConvexHullShape * Convex = new btConvexHullShape(ConvexHulls[i].VBuffer, ConvexHulls[i].VNum);
				//Convex->setMargin(0.0001);
				btTransform trans;
				trans.setIdentity();
				Vector3 & Center = ConvexHulls[i].Center;
				Vector3 LocalCenter = Center - CenterOffset;
				btVector3 c = btVector3(LocalCenter.x, LocalCenter.y, LocalCenter.z);
				trans.setOrigin(c);
				Compound->addChildShape(trans, Convex);
			}
			Shape->Shapes.Compound = Compound;
			model->SetUserData(Shape);
		} 
		else {
			// use box shape as default shape
			Shape = new CollisionShape();
			AABB& aabb = model->GetMesh(0)->GetAABB();
			Vector3& extent = aabb.InitExtents;
			Shape->Shapes.Box = new btBoxShape(btVector3(extent.x, extent.y, extent.z));
			CenterOffset = Vector3(0,0,0);
		}
	}
	// this shape is shared by many physics objects
	Shape->Shared = 1;
}

void PhysicsObject::SetObjectType(PhysicsObject::Type type) {
	ObjectType = type;
}

void PhysicsObject::Clear() {
	// clear all resource used by this one
	if (!Shape->Shared) {
		delete Shape->Shapes.Compound;
		delete Shape;
	}
}

int PhysicsObject::OnDestroy(GameObject * GameObj) {
	World->removeRigidBody(rigidBody);
	Component::OnDestroy(GameObj);
	return 0;
}