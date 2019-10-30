#ifndef __ANIMATOR__
#define __ANIMATOR__


#include "../Scene/Component.h"
#include "AnimationClip.h"
#include "AnimationStage.h"
#include "BlendingNode.h"
#include "Skeleton.h"
#include "BlendShape.h"
#include "Scene\GameObject.h"


/*
	Animator Component
*/

class Animator : public Component
{
	OBJECT(Animator);
	BASEOBJECT(Animator);
	DECLAR_ALLOCATER(Animator);

private:
	// test stage
	AnimationStage* Stage;
	// test blending node
	BlendingNode* BlendNode;
	// skeleton
	Skeleton* skeleton;
	// blendshape
	BlendShape* BlendShape_;
	// blendshape description
	BSDesc BlendShapeDescStage_;
	BSDesc BlendShapeDesc_;

	// motion status
	Vector3 PrevPosition_;
	Matrix4x4 ReferenceTransform_;
	
public:
	Animator(Context* context);
	virtual ~Animator();
	// Set Stage
	void SetAnimationStage(int Layer, AnimationClip* Clip, unsigned char StartBone, float Scale = 1.0f);
	// Set blending node
	void SetBlendingNode(BlendingNode* Node);
	// Set Skeleton
	void SetSkeleton(Skeleton* Skeleton) { skeleton = Skeleton; }
	// Set blendshape
	void SetBlendShape(BlendShape* blendshape);
	// Set blendshape weight by mesh name
	void SetBlendShapeWeight(const String& name, float weight);
	// Set blendshape weight by mesh id
	void SetBlendShapeWeight(int index, float weight);
	// get blending node
	BlendingNode* GetBlendingNode() const;
	// list bendshapes
	void ListBlendShapes() const;

	// update
	void Update(float time);
	// on attach
	virtual int OnAttach(GameObject* GameObj);
	// on destroy
	virtual int OnDestroy(GameObject* GameObj);
};

#endif