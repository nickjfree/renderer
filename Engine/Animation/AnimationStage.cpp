#include "AnimationStage.h"



AnimationStage::AnimationStage() : Scale(1.0f), Time(0.0f), StartBone(0) {
	Cache = AnimationCache::Create();
}


AnimationStage::~AnimationStage() {
	Cache->Recycle();
}

void AnimationStage::SetAnimationClip(AnimationClip * Clip_) {
	 Clip = Clip_;
	 RootMotion = Clip_->RootStart;
}

void AnimationStage::Advance(float time) {
	Time += (time * Scale);
	float Range = Clip->EndTime - Clip->TimeOffset;
    int Loop = (int)(Time / Range);
	if (Loop) {
		Time -= Loop * Range;
		RootMotion = RootMotion - Clip->Tanslation * Loop;
	}
}


void AnimationStage::Apply() {
	Clip->Sample(Time, Cache);
	Vector3& CurrentRootMotion = Cache->Result[0].Translation;
	RootRotation = Cache->Result[0].Rotation;
	MotionDelta = CurrentRootMotion - RootMotion;
	RootMotion = CurrentRootMotion;
}