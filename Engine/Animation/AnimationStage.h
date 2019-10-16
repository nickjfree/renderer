#ifndef __ANIMATION_STAGE__
#define __ANIMATION_STAGE__

#include "AnimationClip.h"
#include "AnimationCache.h"

class Animation;


class AnimationStage {

private:
	// current animation time
	float Time;
	// time scale
	float Scale;
	// the clip to use
	AnimationClip* Clip;
	// the animation cache
	AnimationCache* Cache;
	// previous position and translation
	Vector3 RootMotion;
	// start bone
	unsigned char StartBone;
public:
	// motion delta of this frame
	Vector3 MotionDelta;
	// root motion rotation
	Quaternion RootRotation;
	// float duration
	float Duration;
	// actived
	bool Actived;
public:
	AnimationStage();
	~AnimationStage();

	// set clip
	void SetAnimationClip(AnimationClip* Clip_);
	//settime
	void SetTime(float time) { Time = time; }
	// set scale
	void SetScale(float scale) { Scale = scale; }
	// add time
	void Advance(float time);
	// applly
	void Apply();
	// get cache
	AnimationCache* GetAnimationCache() { return Cache; };

};


#endif
