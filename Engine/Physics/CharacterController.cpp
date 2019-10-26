
#include "CharacterController.h"
#include "Scene/GameObject.h"



USING_ALLOCATER(CharacterController)

CharacterController::CharacterController(Context* context) : PhysicsObject(context) {

}


CharacterController::~CharacterController() {

}


int CharacterController::OnAttach(GameObject* GameObj) {
	// get init position and rotation
	Vector3 Position = GameObj->GetWorldTranslation();
	// offset center of mass to physics center
	Position = CenterOffset + Position;
	btVector3 position = btVector3(Position.x, Position.y, Position.z);
	btTransform transform = btTransform(btQuaternion::getIdentity(), position);
	// create capsule
	Shape = new CollisionShape();
	Shape->Shapes.Capsule = new btCapsuleShape(0.5f, 10);
	Shape->Shared = 0;
	// shape should has refcount == 1
	Shape->AddRef();
	// create ghost object
	GhostOBject_ = new btPairCachingGhostObject();
	GhostOBject_->setCollisionShape(Shape->Shapes.Capsule);
	GhostOBject_->setWorldTransform(transform);
	// set collision flag
	GhostOBject_->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	//  create charactercontroller
	Controller_ = new btKinematicCharacterController(GhostOBject_, Shape->Shapes.Capsule, StepHeight_);
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
	return 0;
}


void CharacterController::SetCeterOffset(Vector3& offset) {
	CenterOffset = offset;
	InvertCenter.Translate(CenterOffset * -1.0f);
}


void CharacterController::SetWalkDirection(Vector3& Walk) {

	btQuaternion& quaternion = GhostOBject_->getWorldTransform().getRotation();
	Quaternion Rotation(quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w());
	Vector3 Direction = Walk * Rotation;
	Controller_->setWalkDirection(btVector3(Direction.x, 0.0f, Direction.z));
}

void CharacterController::SetRotationDirection(Quaternion& Rotation) {
	btMatrix3x3 orn = GhostOBject_->getWorldTransform().getBasis();
	orn *= btMatrix3x3(btQuaternion(Rotation.x, Rotation.y, Rotation.z, Rotation.w));
	GhostOBject_->getWorldTransform().setBasis(orn);
}