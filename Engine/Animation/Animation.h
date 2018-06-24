#ifndef __ANIMATION__
#define __ANIMATION__

#include "AnimationTrack.h"
#include "Resource\Resource.h"


class Animation : public Resource {

private:
	// track of each bone, the indexed by bone_id
	Vector<AnimationTrack> Tracks;
public:
	Animation(Context * context);
	virtual ~Animation();
};

#endif

