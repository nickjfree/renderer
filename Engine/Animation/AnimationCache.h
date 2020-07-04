#ifndef __ANIMATION_CACH__
#define __ANIMATION_CACH__

#include "Skeleton.h"
#include "AnimationTrack.h"

class AnimationCache {

	DECLARE_ALLOCATER(AnimationCache);
	DECLARE_RECYCLE(AnimationCache);

public:
	// interpolated result
	Vector<KeyFrame> Result;
	// previous key frame
	Vector<KeyFrame> Prev;
	// Matrix palette
	Matrix4x4 Palette[NUM_BONES];
public:
	AnimationCache();
	~AnimationCache();
	// genaration matrix palette
	void GeneratePalette(Skeleton* skeleton);

};

#endif 
