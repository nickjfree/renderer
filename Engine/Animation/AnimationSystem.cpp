#include "AnimationSystem.h"


AnimationSystem::AnimationSystem(Context * context) : System(context) {
}


AnimationSystem::~AnimationSystem() {
}


void AnimationSystem::AddAnimator(Animator * animator) {
	animator->AddRef();
	Animators.Insert(animator);
}

void AnimationSystem::RemoveAnimator(Animator * animator) {
	animator->DecRef();
	Animators.Remove(animator);
}

int AnimationSystem::Update(int ms) {
	// update all animators
	List<Animator>::Iterator Iter;
	for (Iter = Animators.Begin(); Iter != Animators.End(); Iter++) {
		Animator * animator = *Iter;
		animator->Update((float)ms);
	}
	return 0;
}