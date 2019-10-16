#ifndef __ANIMATION_SYSTEM__
#define __ANIMATION_SYSTEM__

#include "Core\System.h"
#include "Container\Vector.h"
#include "Container\LinkList.h"
#include "Container\List.h"
#include "Animator.h"


class AnimationSystem : public System
{

	BASEOBJECT(System);
	OBJECT(AnimationSystem);

private:
	// all the animators
	List<Animator> Animators;
	// destroyed animator
	Vector<Animator*> Destroyed;
public:
	AnimationSystem(Context* context);
	virtual ~AnimationSystem();

	// add animator
	void AddAnimator(Animator* animator);
	// remove animator
	void RemoveAnimator(Animator* animator);
	// update
	virtual int Update(int ms);
};


#endif 
