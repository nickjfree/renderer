#include "AnimationStage.h"



AnimationStage::AnimationStage() : Scale(1.0f), Time(0.0f), StartBone(0) {
	Cache = AnimationCache::Create();
}


AnimationStage::~AnimationStage() {
	Cache->Recycle();
}

void AnimationStage::SetAnimationClip(AnimationClip* Clip_) {
	Clip = Clip_;
	RootMotion = Clip_->RootStart;
	RootRotation = Clip->RotationStart;
	Duration = Clip_->EndTime - Clip_->TimeOffset;
}

void AnimationStage::Advance(float time) {
	Time += (time * Scale);
	auto Range = Clip->EndTime - Clip->TimeOffset;
	auto Loop = (int)(Time / Range);
	if (Loop) {
		Time -= Loop * Range;
		RootMotion = RootMotion - Clip->Tanslation * (float)Loop;
	}
	while (Loop--) {
		RootRotation = RootRotation * Clip->Rotation.Inverse();
	}
}


void AnimationStage::Apply() {
	Clip->Sample(Time, Cache);
	// motion delta
	MotionDelta = Cache->Result[0].Translation - RootMotion;
	RootMotion = Cache->Result[0].Translation;;
	// rotaton delta
	RotationDelta = RootRotation.Inverse() * Cache->Result[0].Rotation;
	RootRotation = Cache->Result[0].Rotation;
}