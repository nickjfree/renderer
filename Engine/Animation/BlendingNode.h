#ifndef __BLENDING_NODE__
#define __BLENDING_NODE__

#include "../Core/Object.h"
#include "../Container/Vector.h"
#include "Animation.h"
#include "AnimationStage.h"

/*
	simple blending node
*/

class BlendingNode : public Object
{
	BASEOBJECT(BlendingNode);
	OBJECT(BlendingNode);

protected:
	// animation stage
	AnimationStage* AnimStage;
	// parameters
	Dict Parameters;
public:
	// length
	float Duration;
	// motion delta

public:
	BlendingNode(Context* context);
	~BlendingNode();
	// set animation clip
	void SetAnimationClip(Animation* animation, int index);
	// set scale
	void SetScale(float Scale_);
	// advance
	virtual int Advance(float time);
	// apply
	virtual int Apply();
	// GetAnimationCache
	virtual AnimationCache* GetAnimationCache();
	// Set Parameters
	void SetParameter(const String& Name, float Value);
};

class BinaryBlendingNode : public BlendingNode
{
	BASEOBJECT(BlendingNode);
	OBJECT(BlendingNode);

protected:
	// animation stages
	BlendingNode* NodeA;
	BlendingNode* NodeB;
	// blend factor
	float Alpha;
	// scale factor Duration_A/Duration_B, so it is scale for nodeA
	float Scale;
	// AnimationCache
	AnimationCache* Cache;
	// sync cycle or not 
	bool SyncCycle;
public:
	BinaryBlendingNode(Context* context);
	~BinaryBlendingNode();
	// add 2 nodes
	void AddNodes(BlendingNode* NodeA, BlendingNode* NodeB, bool SyncCycle);
	// set alpha
	void SetAlpha(float alpha) { SetParameter("x", alpha); };
	// advance
	virtual int Advance(float time);
	// apply
	virtual int Apply();
	// GetAnimationCache
	virtual AnimationCache* GetAnimationCache();
};

#endif
