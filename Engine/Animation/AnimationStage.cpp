#include "AnimationStage.h"



AnimationStage::AnimationStage() : Scale(1.0f), Time(0.0f), StartBone(0) {
	Cache = AnimationCache::Create();
}


AnimationStage::~AnimationStage() {
	Cache->Recycle();
}

void AnimationStage::Advance(float time) {
	Time += (time * Scale);
}


void AnimationStage::Apply() {
	Clip->Sample(Time, Cache);
}