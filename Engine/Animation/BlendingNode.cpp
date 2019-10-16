#include "BlendingNode.h"



BlendingNode::BlendingNode(Context* context) : Object(context) {
}


BlendingNode::~BlendingNode() {
	// clean up
	if (AnimStage) {
		delete AnimStage;
	}
}

void BlendingNode::SetAnimationClip(Animation* animation, int index) {
	AnimationClip* Clip = animation->GetAnimationClip(index);
	AnimStage = new AnimationStage();
	AnimStage->SetAnimationClip(Clip);
	AnimStage->SetTime(0.0f);
	AnimStage->SetScale(1);
	// get animation length
	Duration = AnimStage->Duration;
}

// advance
int BlendingNode::Advance(float time) {
	AnimStage->Advance(time);
	return 0;
}

// apply
int BlendingNode::Apply() {
	AnimStage->Apply();
	return 0;
}

// set scale
void BlendingNode::SetScale(float Scale_) {
	AnimStage->SetScale(Scale_);
}

// GetAnimationCache
AnimationCache* BlendingNode::GetAnimationCache() {
	return AnimStage->GetAnimationCache();
}

void BlendingNode::SetParameter(const String& Name, float Value) {
	Parameters[Name].as<float>() = Value;
}

BinaryBlendingNode::BinaryBlendingNode(Context* context) : BlendingNode(context), Alpha(0.5f) {
	Cache = AnimationCache::Create();
}

BinaryBlendingNode::~BinaryBlendingNode() {
	Cache->Recycle();
	if (NodeA) {
		NodeA->DecRef();
	}
	if (NodeB) {
		NodeB->DecRef();
	}
}

void BinaryBlendingNode::AddNodes(BlendingNode* NodeA_, BlendingNode* NodeB_, bool Sync) {
	NodeA = NodeA_;
	NodeB = NodeB_;
	SyncCycle = Sync;
}

// advance
int BinaryBlendingNode::Advance(float time) {
	Alpha = Parameters["x"].as<float>();
	// calculate scale factor
	if (SyncCycle) {
		Scale = NodeA->Duration / NodeB->Duration;
		float scale_min = 1.0f / Scale;
		float scaleB = scale_min * (1 - Alpha) + Alpha * 1.0f;
		NodeA->SetScale(scaleB * Scale);
		NodeB->SetScale(scaleB);
	}
	else {
		NodeA->SetScale(1.0f);
		NodeB->SetScale(1.0f);
	}
	// advance
	NodeA->Advance(time);
	NodeB->Advance(time);
	return 0;
}

// apply
int BinaryBlendingNode::Apply() {
	Alpha = Parameters["x"].as<float>();
	NodeA->Apply();
	NodeB->Apply();
	// Blending
	AnimationCache* CacheA = NodeA->GetAnimationCache();
	AnimationCache* CacheB = NodeB->GetAnimationCache();
	int NumChannel = CacheA->Result.Size();
	if (Cache->Result.Size() < NumChannel) {
		Cache->Result.Resize(NumChannel, 1);
	}
	for (int channel = 0; channel < NumChannel; channel++) {
		KeyFrame& frame = Cache->Result[channel];
		KeyFrame& frameA = CacheA->Result[channel];
		KeyFrame& frameB = CacheB->Result[channel];
		frame.BoneId = frameA.BoneId;
		frame.Translation = Vector3::Lerp(frameA.Translation, frameB.Translation, Alpha);
		frame.Rotation = Quaternion::Slerp(frameA.Rotation, frameB.Rotation, Alpha);
		// no scale
		frame.Scale = frameA.Scale;
		frame.Time = frameA.Time;
	}
	return 0;
}

// GetAnimationCache
AnimationCache* BinaryBlendingNode::GetAnimationCache() {
	return Cache;
}