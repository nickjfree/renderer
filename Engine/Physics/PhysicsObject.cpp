#include "PhysicsObject.h"
#include "Scene\GameObject.h"
#include "PhysicsSystem.h"
#include "Rendering/MeshRenderer.h"

USING_ALLOCATER(PhysicsObject)




void CollisionShape::CreateFromConvexData(int Clusters, MeshConvex* ConvexHulls, Vector3& CenterOffset) {
	btCompoundShape* Compound = new btCompoundShape();
	for (int i = 0; i < Clusters; i++) {
		btConvexHullShape* Convex = new btConvexHullShape(ConvexHulls[i].VBuffer, ConvexHulls[i].VNum);
		//Convex->setMargin(0.0001);
		btTransform trans;
		trans.setIdentity();
		Vector3& Center = ConvexHulls[i].Center;
		Vector3 LocalCenter = Center - CenterOffset;
		btVector3 c = btVector3(LocalCenter.x, LocalCenter.y, LocalCenter.z);
		trans.setOrigin(c);
		Compound->addChildShape(trans, Convex);
	}
	Shapes.Compound = Compound;
}

int CollisionShape::AddRef() {
	return InterlockedIncrement(&ref);
}

int CollisionShape::DecRef() {
	auto ret = InterlockedDecrement(&ref);
	if (ret == 0) {
		// free resource used by this shape
		auto shape = Shapes.Compound;
		if (shape) {
			if (shape->isCompound()) {
				for (auto i = 0; i < shape->getNumChildShapes(); i++) {
					auto child = shape->getChildShape(i);
					shape->removeChildShapeByIndex(i);
					delete child;
				}
			}
			else {
				delete shape;
			}
		}
		// delete this
		delete this;
	}
	return ret;
}



PhysicsObject::PhysicsObject(Context* context) : Component(context), Shape(0), ObjectType(0), rigidBody(nullptr), MotionState(nullptr) {
	// create a default rigitbody
	Physics = context->GetSubsystem<PhysicsSystem>();
	World = Physics->GetWorld();
	CenterOffset = Vector3(0, 0, 0);
	InvertCenter.Identity();
}


PhysicsObject::~PhysicsObject() {
	Clear();
}

int PhysicsObject::OnAttach(GameObject* GameObj) {
	// get init position and rotation
	Vector3 Position = GameObj->GetWorldTranslation();
	Quaternion Rotation = GameObj->GetWorldRotation();
	// offset center of mass to physics center
	Position = CenterOffset * Rotation + Position;
	btVector3 position = btVector3(Position.x, Position.y, Position.z);
	btQuaternion quaternion;
	btQuaternionFloatData data = { Rotation.x,Rotation.y,Rotation.z,Rotation.w, };
	quaternion.deSerializeFloat(data);
	btTransform transform = btTransform(quaternion, position);
	if (!Shape) {
		AutoGenerateShape(GameObj);
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

int PhysicsObject::OnTransform(GameObject* object) {
	if (rigidBody && rigidBody->isKinematicObject()) {
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

void PhysicsObject::CreateShapeFromModel(Model* model) {

	Shape = (CollisionShape*)model->GetUserData();
	MeshConvex* ConvexHulls = model->GetMesh(0)->ConvexHulls;
	int Clusters = model->GetMesh(0)->NumConvex;
	// get the compund object center
	for (int i = 0; i < Clusters; i++) {
		CenterOffset = CenterOffset + ConvexHulls[i].Center;
	}
	if (Clusters) {
		CenterOffset = CenterOffset * (1.0f / Clusters);
		InvertCenter.Translate(CenterOffset * -1.0f);
	}
	// try create the shape
	if (!Shape) {
		Shape = new CollisionShape();
		Shape->ref = 0;
		if (Clusters) {
			Shape->CreateFromConvexData(Clusters, ConvexHulls, CenterOffset);
		}
		else {
			// use box shape as default shape
			AABB& aabb = model->GetMesh(0)->GetAABB();
			Vector3& extent = aabb.InitExtents;
			Shape->Shapes.Box = new btBoxShape(btVector3(extent.x, extent.y, extent.z));
			CenterOffset = Vector3(0, 0, 0);
		}
		// set shape to model
		model->SetUserData(Shape);
	}
	// this shape is shared by many physics objects
	Shape->Shared = 1;
	Shape->AddRef();
}

void PhysicsObject::SetObjectType(PhysicsObject::Type type) {
	ObjectType = type;
}

void PhysicsObject::Clear() {
	// clear all resource used by this one
	if (Shape && !Shape->Shared) {
		delete Shape->Shapes.Box;
		delete Shape;
	}
	if (MotionState) {
		delete MotionState;
	}
	if (rigidBody) {
		delete rigidBody;
	}

}

int PhysicsObject::OnDestroy(GameObject* GameObj) {
	if (rigidBody) {
		World->removeRigidBody(rigidBody);
	}
	Component::OnDestroy(GameObj);
	// remove shape
	if (Shape) {
		Shape->DecRef();
		Shape = nullptr;
	}
	return 0;
}

int PhysicsObject::Load(void* Raw, Level* level) {
	auto* physicsData = (PhysicsEntry *)Raw;
	ObjectType = physicsData->Type;
	return sizeof(PhysicsEntry);
}

void PhysicsObject::AutoGenerateShape(GameObject* GameObj) {
	auto render = (MeshRenderer *)GameObj->GetComponent("Renderer");
	if (render) {
		auto model = render->GetModel();
		CreateShapeFromModel(model);
	}
	else {
		// default to a box shape
		Shape = new CollisionShape();
		Shape->Shapes.Box = new btBoxShape(btVector3(2, 2, 2));
		Shape->Shared = 0;
	}
}