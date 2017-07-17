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
	if (!Shape) {
		Shape = new CollisionShape();
		Shape->Shapes.Box = new btBoxShape(btVector3(2, 2, 2));
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
    // set to gameobject
	Owner->SetRotation(Rotation);
	Owner->SetTranslation(Position);


	return 0;
}

int PhysicsObject::OnTransform(GameObject * object) {
	if (rigidBody->isKinematicObject()) {
		Vector3 Position = object->GetWorldTranslation();
		Quaternion Rotation = object->GetWorldRotation();
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
	if (!Shape) {
		
		MeshConvex * ConvexHulls = model->GetMesh(0)->ConvexHulls;
		int Clusters = model->GetMesh(0)->NumConvex;
		if (Clusters) {
			Shape = new CollisionShape();
			btCompoundShape * Compound = new btCompoundShape();
			// get the compund object center
			for (int i = 0; i < Clusters; i++) {
				CenterOffset = CenterOffset + ConvexHulls[i].Center;
			}
			CenterOffset = CenterOffset * (1.0f / Clusters);
			for (int i = 0; i < Clusters; i++) {
				btConvexHullShape * Convex = new btConvexHullShape(ConvexHulls[i].VBuffer, ConvexHulls[i].VNum);
				//Convex->setMargin(0.0001);
				btTransform trans;
				trans.setIdentity();
				Vector3 & Center = ConvexHulls[i].Center;
				Vector3 LocalCenter = Center - CenterOffset;
				btVector3 c = btVector3(Center.x, Center.y, Center.z);
				trans.setOrigin(c);
				Compound->addChildShape(trans, Convex);
			}
			Shape->Shapes.Compound = Compound;
			model->SetUserData(Shape);
		}
	}
}

void PhysicsObject::SetObjectType(PhysicsObject::Type type) {
	ObjectType = type;
}
