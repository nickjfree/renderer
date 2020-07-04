#include "AnimationSystem.h"


AnimationSystem::AnimationSystem(Context* context) : System(context) {
}


AnimationSystem::~AnimationSystem() {
}

int AnimationSystem::Initialize() {
	context->RegisterObject<Animator>();
	context->RegisterObject<BlendingNode>();
	context->RegisterObject<BlendingNode2>();
	context->RegisterObject<BlendingNode3>();
	return 0;
}



void AnimationSystem::AddAnimator(Animator* animator) {
	animator->AddRef();
	Animators.Insert(animator);
}

void AnimationSystem::RemoveAnimator(Animator* animator) {
	animator->DecRef();
	Animators.Remove(animator);
}

int AnimationSystem::Update(int ms) {
	// update all animators
	for (auto Iter = Animators.Begin(); Iter != Animators.End(); Iter++) {
		Animator* animator = *Iter;
		if (!animator->Destroyed) {
			animator->Update((float)ms);
		}
		else {
			Destroyed.PushBack(animator);
		}
	}
	// handle destroyed animators
	for (auto Iter = Destroyed.Begin(); Iter != Destroyed.End(); Iter++) {
		Animator* animator = *Iter;
		animator->DecRef();
	}
	Destroyed.Empty();
	return 0;
}