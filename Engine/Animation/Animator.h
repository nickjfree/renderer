#ifndef __ANIMATOR__
#define __ANIMATOR__
#include "../Scene/Component.h"

#include "AnimationClip.h"
#include "AnimationStage.h"
#include "BlendingNode.h"
#include "Skeleton.h"
#include "Scene\GameObject.h"

/*
    Animator Component
*/

class Animator : public Component {


    OBJECT(Animator);
    BASEOBJECT(Animator);
    DECLAR_ALLOCATER(Animator);

private:
    // test stage
    AnimationStage * Stage;
    // test blending node
    BlendingNode * BlendNode;
    // skeleton
    Skeleton * skeleton;
public:
    Animator(Context * context);
    virtual ~Animator();
    // Set Stage
    void SetAnimationStage(int Layer, AnimationClip * Clip, unsigned char StartBone, float Scale = 1.0f);
    // Set blending node
    void SetBlendingNode(BlendingNode * Node);
    // Set Skeleton
    void SetSkeleton(Skeleton * Skeleton) { skeleton = Skeleton; }
    // get blending node
    BlendingNode * GetBlendingNode();
    // update
    void Update(float time);
    // on attach
    virtual int OnAttach(GameObject * GameObj);
    // on destroy
    virtual int OnDestroy(GameObject * GameObj);
};

#endif