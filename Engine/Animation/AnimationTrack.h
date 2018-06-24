#ifndef __ANIMATION_TRACK__
#define __ANIMATION_TRACK__

#include "Math\LinearMath.h"
#include "Container\Vector.h"

/*
	keyframe of a bone/node
*/
typedef struct KeyFrame {
	int BoneId;
	int Time;
	Vector3 Translation;
	Quaternion Rotation;
	float Scale;
}KeyFrame;


/*
	aniamtion track of a bone/node
*/
class AnimationTrack {

private:
	Vector<KeyFrame> Frames;
public:
	AnimationTrack();
	~AnimationTrack();
};

#endif
