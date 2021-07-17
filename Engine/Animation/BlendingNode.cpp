#include "BlendingNode.h"


USING_ALLOCATER(BlendingNode)
USING_ALLOCATER(BlendingNode2)
USING_ALLOCATER(BlendingNode3)


BlendingNode::BlendingNode(Context* context) : Object(context), AnimStage(nullptr), Duration(0) {
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
	Parameters[Name] = Value;
}


BlendingNode2::BlendingNode2(Context* context) : BlendingNode(context), Alpha(0.5f), Scale(1.0f), SyncCycle(true), NodeA(nullptr), NodeB(nullptr) {
	Cache = AnimationCache::Create();
}

BlendingNode2::~BlendingNode2() {
	Cache->Recycle();
	if (NodeA) {
		NodeA->DecRef();
	}
	if (NodeB) {
		NodeB->DecRef();
	}
}

void BlendingNode2::AddNodes(BlendingNode* NodeA_, BlendingNode* NodeB_, bool Sync) {
	NodeA = NodeA_;
	NodeB = NodeB_;
	SyncCycle = Sync;
}

// advance
int BlendingNode2::Advance(float time) {
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
	// adjust duration
	Duration = NodeA->Duration * (1 - Alpha) + NodeB->Duration * Alpha;
	return 0;
}

// apply
int BlendingNode2::Apply() {
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
AnimationCache* BlendingNode2::GetAnimationCache() {
	return Cache;
}

// get motion
Matrix4x4& BlendingNode2::GetMotion() { 
	Matrix4x4& MotionA = NodeA->GetMotion();
	Matrix4x4& MotionB = NodeB->GetMotion();
	
	Vector3 TranslationA = Vector3(0, 0, 0) * MotionA;
	Vector3 TranslationB = Vector3(0, 0, 0) * MotionB;

	Quaternion RotationA, RotationB;
	RotationA.FromMatrix(MotionA);
	RotationB.FromMatrix(MotionB);
	
	Vector3 Translation = Vector3::Lerp(TranslationA, TranslationB, Alpha);
	Quaternion Rotation = Quaternion::Slerp(RotationA, RotationB, Alpha);

	Motion = Matrix4x4::FormPositionRotation(Translation, Rotation);

	return Motion;
}


BlendingNode3::BlendingNode3(Context* context): BlendingNode(context), Scale(1.0f), SyncCycle(true), Nodes_() {
	Cache = AnimationCache::Create();
}


BlendingNode3::~BlendingNode3() {
	Cache->Recycle();
	for (auto i = 0; i < 3; i++) {
		if (Nodes_[i]) {
			Nodes_[i]->DecRef();
		}
	}
}

void BlendingNode3::AddNodes(BlendingNode* Nodes[3], bool Sync) {
	for (auto i = 0; i < 3; i++) {
		Nodes_[i] = Nodes[i];
	}
	SyncCycle = Sync;
}

void BlendingNode3::SetNode(int Index, BlendingNode* Node) {
	Nodes_[Index] = Node;
}

// advance
int BlendingNode3::Advance(float time) {
	Alpha = Parameters["x"].as<float>();
	// calculate scale factor
	if (SyncCycle) {
		if (Alpha > 0) {
			// blenging to right node
			Scale = Nodes_[1]->Duration / Nodes_[2]->Duration;
			float scale_min = 1.0f / Scale;
			float scaleOther = scale_min * (1 - Alpha) + Alpha * 1.0f;
			Nodes_[1]->SetScale(scaleOther * Scale);
			Nodes_[0]->SetScale(scaleOther * Scale * Nodes_[0]->Duration / Nodes_[1]->Duration);
			Nodes_[2]->SetScale(scaleOther);
			// adjust duration
			Duration = Nodes_[1]->Duration * (1 - Alpha) * Nodes_[2]->Duration * Alpha;
		} else {
			// blending to left node
			Scale = Nodes_[1]->Duration / Nodes_[0]->Duration;
			float scale_min = 1.0f / Scale;
			float scaleOther = scale_min * (1 - Alpha) + Alpha * 1.0f;
			Nodes_[1]->SetScale(scaleOther * Scale);
			Nodes_[0]->SetScale(scaleOther);
			Nodes_[2]->SetScale(scaleOther * Scale * Nodes_[2]->Duration / Nodes_[1]->Duration);
			// adjust duration
			Duration = Nodes_[1]->Duration * (1 - Alpha) * Nodes_[2]->Duration * Alpha;
		}
	}
	else {
		for (auto i = 0; i < 3; i++) {
			Nodes_[i]->SetScale(1.0f);
		}
	}
	// advance
	for (auto i = 0; i < 3; i++) {
		Nodes_[i]->Advance(time);
	}
	return 0;
}


// apply
int BlendingNode3::Apply() {
	Alpha = Parameters["x"].as<float>();
	for (auto i = 0; i < 3; i++) {
		Nodes_[i]->Apply();
	}
	// Blending
	AnimationCache* CacheA, * CacheB;
	CacheA = Nodes_[1]->GetAnimationCache();
	if (Alpha > 0) {
		CacheB = Nodes_[2]->GetAnimationCache();
	} else {
		CacheB = Nodes_[0]->GetAnimationCache();
	}

	int NumChannel = CacheA->Result.Size();
	if (Cache->Result.Size() < NumChannel) {
		Cache->Result.Resize(NumChannel, 1);
	}
	for (int channel = 0; channel < NumChannel; channel++) {
		KeyFrame& frame = Cache->Result[channel];
		KeyFrame& frameA = CacheA->Result[channel];
		KeyFrame& frameB = CacheB->Result[channel];
		frame.BoneId = frameA.BoneId;
		frame.Translation = Vector3::Lerp(frameA.Translation, frameB.Translation, abs(Alpha));
		frame.Rotation = Quaternion::Slerp(frameA.Rotation, frameB.Rotation, abs(Alpha));
		// no scale
		frame.Scale = frameA.Scale;
		frame.Time = frameA.Time;
	}
	return 0;
}

// GetAnimationCache
AnimationCache* BlendingNode3::GetAnimationCache() {
	return Cache;
}

// get motion
Matrix4x4& BlendingNode3::GetMotion() {

	int targetIndex = 2;
	if (Alpha > 0) {
		targetIndex = 2;
	} else {
		targetIndex = 0;
	}

	Matrix4x4& MotionA = Nodes_[1]->GetMotion();
	Matrix4x4& MotionB = Nodes_[targetIndex]->GetMotion();


	Vector3 TranslationA = Vector3(0, 0, 0) * MotionA;
	Vector3 TranslationB = Vector3(0, 0, 0) * MotionB;

	Quaternion RotationA, RotationB;
	RotationA.FromMatrix(MotionA);
	RotationB.FromMatrix(MotionB);

	Vector3 Translation = Vector3::Lerp(TranslationA, TranslationB, abs(Alpha));
	Quaternion Rotation = Quaternion::Slerp(RotationA, RotationB, abs(Alpha));

	Motion = Matrix4x4::FormPositionRotation(Translation, Rotation);

	return Motion;
}