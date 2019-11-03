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
	Matrix4x4 PrevTransform;
	// start frame transform
	Matrix4x4 StartTransform;
	// end frame transform
	Matrix4x4 EndTransform;
	// delta transform, motion between frames
	Matrix4x4 Motion;
	// start bone
	unsigned char StartBone;
public:
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
	// get motion
	Matrix4x4& GetMotion() { return Motion; }
	// get cache
	AnimationCache* GetAnimationCache() { return Cache; };

};


#endif
