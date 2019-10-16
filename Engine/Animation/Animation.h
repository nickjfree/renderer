#ifndef __ANIMATION__
#define __ANIMATION__

#include "AnimationTrack.h"
#include "AnimationClip.h"
#include "AnimationStage.h"
#include "Resource\Resource.h"
#include "Misc\h3d\h3d.h"

using namespace h3d;

#define ANIMATION_LAYERS 8

class Animation : public Resource {

	BASEOBJECT(Animation);
	OBJECT(Animation);
	LOADEROBJECT(FileLoader);
	DECLAR_ALLOCATER(Animation);

	// allow AnimationClip to sample the data
	friend AnimationClip;


private:
	// track of each bone, the indexed by bone_id
	Vector<AnimationTrack> Tracks;
	Vector<AnimationClip> Clips;
	// up to 8 animation layers

public:
	Animation(Context* context);
	virtual ~Animation();
	// process raw data
	virtual int OnSerialize(Deserializer& serializer);
	// on raw data parse complete(worker thread)
	virtual int OnLoadComplete(Variant& Data) { DeSerial.Release(); return 0; };
	// on destroy
	virtual int OnDestroy(Variant& Param);
	// get clip
	AnimationClip* GetAnimationClip(int Index) { return &Clips[Index]; };
};

#endif

