#ifndef __ANIMATOR__
#define __ANIMATOR__
#include "../Scene/Component.h"

#include "AnimationClip.h"
#include "AnimationStage.h"
#include "Skeleton.h"
#include "Scene\GameObject.h"

/*
	Animator Component
*/

class Animator : public Component {


	OBJECT(Animator);
	BASEOBJECT(Animator);
	DECLAR_ALLOCATER(Animator);

private:
	// test stage
	AnimationStage * Stage;
	// skeleton
	Skeleton * skeleton;
public:
	Animator(Context * context);
	virtual ~Animator();
	// Set Stage
	void SetAnimationStage(int Layer, AnimationClip * Clip, unsigned char StartBone, float Scale=1.0f);
	// Set Skeleton
	void SetSkeleton(Skeleton * Skeleton) { skeleton = Skeleton; }
	// update
	void Update(float time);
	// on attach
	virtual int OnAttach(GameObject * GameObj);

};

#endif