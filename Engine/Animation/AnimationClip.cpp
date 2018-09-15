#include "AnimationClip.h"
#include "Animation.h"


AnimationClip::AnimationClip() {
}


AnimationClip::~AnimationClip() {
}


void AnimationClip::Sample(float time, AnimationCache * Cache) {
	

	int NumChannel = Source->Tracks.Size();
	KeyFrame * Left, *Right;
	float Alpha;

	if (Looped) {
		time = fmod(time, EndTime - TimeOffset);
	}

	if (Cache->Result.Size() < NumChannel) {
		Cache->Result.Resize(NumChannel, 1);
	}

	for (int channel = 0; channel < NumChannel; channel++) {
		AnimationTrack * Track = &Source->Tracks[channel];
		Track->Sample(time + TimeOffset, StartFrame, EndFrame, &Left, &Right, &Alpha);
		if (Left && Right) {
			KeyFrame& frame = Cache->Result[channel];
			frame.BoneId = Left->BoneId;
			frame.Translation =  Vector3::Lerp(Left->Translation, Right->Translation, Alpha);
			frame.Rotation = Quaternion::Slerp(Left->Rotation, Right->Rotation, Alpha);
			// no scale
			frame.Scale = Left->Scale;
			frame.Time = time;
		}
	}

	Source->Tracks;
}