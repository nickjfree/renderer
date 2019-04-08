#ifndef __ANIMATION_TRACK__
#define __ANIMATION_TRACK__

#include "Math\LinearMath.h"
#include "Container\Vector.h"

class Animation;

/*
    keyframe of a bone/node
*/
typedef struct KeyFrame {
    Vector3 Translation;
    Quaternion Rotation;
    float Scale;
    int BoneId;
    float Time;
}KeyFrame;


/*
    aniamtion track of a bone/node
*/
class AnimationTrack {

    friend Animation;

private:
    Vector<KeyFrame> Frames;
public:
    AnimationTrack();
    ~AnimationTrack();

    void Sample(float time, int start, int end, KeyFrame ** left, KeyFrame ** right, float * alpha);

};

#endif
