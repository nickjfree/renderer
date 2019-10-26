#ifndef __CHARACTOR_CONTROLLER__
#define __CHARACTOR_CONTROLLER__

#include "PhysicsObject.h"



class CharacterController : public PhysicsObject
{
	DECLAR_ALLOCATER(CharacterController);
	OBJECT(CharacterController);
	BASEOBJECT(CharacterController);

private:
	// character controller
	btKinematicCharacterController* Controller_;
	// ghostobject
	btPairCachingGhostObject * GhostOBject_;
	// step height
	float StepHeight_;
	// chracter height
	float Height_;
	// cahracter width
	float Width_;


public:
	// constructer/destructor
	CharacterController(Context* context);
	~CharacterController();

	// on attach to gameobject
	int virtual OnAttach(GameObject* GameObj);
	// on destroy 
	int virtual OnDestroy(GameObject* GameObj);
	// update
	int virtual Update(int ms);
	// character controller actions
	void SetCeterOffset(Vector3& offset);
	// set character Size
	void SetHeight(float Height) { Height_ = Height; }
	// set width or radius
	void SetWidth(float Width) { Width_ = Width; }
	// set StepHeight
	void SetStepHeight(float StepHeight) { StepHeight_ = StepHeight; }
	// set walk direction
	void SetWalkDirection(Vector3& Walk);
	// set rotation
	void SetRotationDirection(Quaternion& Rotation);

};



#endif