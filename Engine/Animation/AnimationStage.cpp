#include "AnimationStage.h"



AnimationStage::AnimationStage() : Scale(1.0f), Time(0.0f), StartBone(0), Actived(0), Clip(nullptr), Duration(0) {
	Cache = AnimationCache::Create();
}


AnimationStage::~AnimationStage() {
	Cache->Recycle();
}

void AnimationStage::SetAnimationClip(AnimationClip* Clip_) {
	Clip = Clip_;
	Duration = Clip_->EndTime - Clip_->TimeOffset;

	// get start and end root transform. snape to xz plane
	StartTransform = PrevTransform = Matrix4x4::FormPositionRotation(Clip_->RootStart, Clip_->RotationStart);
	StartTransform.SnapeXZ();
	PrevTransform.SnapeXZ();
	EndTransform = Matrix4x4::FormPositionRotation(Clip_->RootEnd, Clip_->RotationEnd);
	EndTransform.SnapeXZ();
}

void AnimationStage::Advance(float time) {
	Time += (time * Scale);
	auto Range = Clip->EndTime - Clip->TimeOffset;
	auto Loop = (int)(Time / Range);
	while (Loop--) {
		Time -= Range;
		// adjust prev transform, set it to a litte bit ahead of the start frame
		//  x * end = prev  
		//  prev =  x * start 
		Matrix4x4 InverseEndTrans;
		Matrix4x4::Inverse(EndTransform, &InverseEndTrans);
		Matrix4x4 RemainTransform = PrevTransform * InverseEndTrans;
		PrevTransform = RemainTransform * StartTransform;
	}
}


void AnimationStage::Apply() {
	Clip->Sample(Time, Cache);
	// motion delta
	Matrix4x4 CurrentTransform = Matrix4x4::FormPositionRotation(Cache->Result[0].Translation, Cache->Result[0].Rotation);
	Matrix4x4 RootTransform = CurrentTransform;
	RootTransform.SnapeXZ();
	Matrix4x4 InvertRoot;
	Matrix4x4::Inverse(RootTransform, &InvertRoot);
	// delta * snapedxz  = current
	Matrix4x4 RelativedToRoot = CurrentTransform * InvertRoot;
	Cache->Result[0].Rotation.FromMatrix(RelativedToRoot);
	// extract motion.  only adrress the rotation in XZ plane

	// Motion * PrevTransform = CurrentTransform
	Matrix4x4 Inverse;
	Matrix4x4::Inverse(PrevTransform, &Inverse);
	Motion = RootTransform * Inverse;
	PrevTransform = RootTransform;
}