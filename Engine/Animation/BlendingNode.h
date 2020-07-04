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
	DECLARE_ALLOCATER(BlendingNode);

protected:
	// animation stage
	AnimationStage* AnimStage;
	// parameters
	Dict Parameters;
public:
	// length
	float Duration;
	// motion delta
	Vector3 MotionDelta;
	// rotation delta
	Quaternion RotationDelta;
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
	virtual void SetParameter(const String& Name, float Value);
	// get motion
	virtual Matrix4x4& GetMotion() { return AnimStage->GetMotion();  }
};

class BlendingNode2 : public BlendingNode
{
	BASEOBJECT(BlendingNode2);
	OBJECT(BlendingNode2);
	DECLARE_ALLOCATER(BlendingNode2);

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
	// motion
	Matrix4x4 Motion;
public:
	BlendingNode2(Context* context);
	~BlendingNode2();
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
	// get motion
	virtual Matrix4x4& GetMotion();
};

class BlendingNode3 : public BlendingNode
{
	BASEOBJECT(BlendingNode3);
	OBJECT(BlendingNode3);
	DECLARE_ALLOCATER(BlendingNode3);

protected:
	// the 3 nodes to blending,   -x 0 +x => left, center, right
	BlendingNode* Nodes_[3];
	// scale factor
	float Scale;
	// AnimationCache
	AnimationCache* Cache;
	// sync cycle or not 
	bool SyncCycle;
	// alpha
	float Alpha;
	// motion
	Matrix4x4 Motion;

public:
	// constructor
	BlendingNode3(Context* context);
	// destructor
	~BlendingNode3();
	// add 3 nodes
	void AddNodes(BlendingNode* Nodes[3],  bool Sync);
	// advance
	virtual int Advance(float time);
	// apply
	virtual int Apply();
	// GetAnimationCache
	virtual AnimationCache* GetAnimationCache();
	// get motion
	virtual Matrix4x4& GetMotion();
};



#endif
