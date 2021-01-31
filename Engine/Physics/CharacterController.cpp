
#include "CharacterController.h"
#include "Scene/GameObject.h"



USING_ALLOCATER(CharacterController)

CharacterController::CharacterController(Context* context) : PhysicsObject(context), Controller_(nullptr), GhostOBject_(nullptr), 
	Height_(2.0f), StepHeight_(0.1f), Width_(0.5)

{

}


CharacterController::~CharacterController() {

}


int CharacterController::OnAttach(GameObject* GameObj) {
	// get init position and rotation
	Vector3 Position = GameObj->GetWorldTranslation();
	Quaternion Rotation = GameObj->GetWorldRotation();
	// offset center of mass to physics center
	Position = CenterOffset + Position;
	btVector3 position = btVector3(Position.x, Position.y, Position.z);
	btTransform transform = btTransform(btQuaternion(Rotation.x, Rotation.y, Rotation.z, Rotation.w), position);
	// create capsule
	Shape = new CollisionShape();
	Shape->Shapes.Capsule = new btCapsuleShape(5.0f, 10);
	Shape->Shared = 0;
	// shape should has refcount == 1
	Shape->AddRef();
	// create ghost object  
	GhostOBject_ = new btPairCachingGhostObject();
	GhostOBject_->setCollisionShape(Shape->Shapes.Capsule);
	// set collision flag
	GhostOBject_->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	//  create charactercontroller
	Controller_ = new btKinematicCharacterController(GhostOBject_, Shape->Shapes.Capsule, StepHeight_);
	Controller_->setGravity(btVector3(0, -10, 0));
	// set transform, must be called after btKinematicCharacterController creation
	GhostOBject_->setWorldTransform(transform);
	// add to world
	Physics->AddPhysicsObject(this);
	// add controller and ghost object to world
	World->addCollisionObject(GhostOBject_, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	World->addAction(Controller_);
	GhostOBject_->setUserPointer(this);
	return 0;
}


int CharacterController::OnDestroy(GameObject* GameObj) {
	// remove character controller and ghost object
	World->removeAction(Controller_);
	World->removeCollisionObject(GhostOBject_);
	// call base ondestroy
	PhysicsObject::OnDestroy(GameObj);
	delete GhostOBject_;
	delete Controller_;
	GhostOBject_ = nullptr;
	Controller_ = nullptr;
	return 0;
}


int CharacterController::Update(int ms) {
	// only deals with rigide bodies, read btTransform and set to gameobject

	btTransform transform = GhostOBject_->getWorldTransform();
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
	// Owner->SetTransform(InvertCenter * Transform);
	Owner->SetTranslation(Position - CenterOffset);
	return 0;
}


void CharacterController::SetCeterOffset(Vector3& offset) {
	CenterOffset = offset;
	InvertCenter.Translate(CenterOffset * -1.0f);
}


void CharacterController::SetWalkDirection(Vector3& Walk) {
	Controller_->setWalkDirection(btVector3(Walk.x, 0.0f, Walk.z));
}

void CharacterController::SetRotation(Quaternion& Rotation) {
	auto orn = btMatrix3x3(btQuaternion(Rotation.x, Rotation.y, Rotation.z, Rotation.w));
	GhostOBject_->getWorldTransform().setBasis(orn);
}