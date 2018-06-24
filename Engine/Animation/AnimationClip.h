#ifndef __ANIMATION_CLIP__
#define __ANIMATION_CLIP__

#include "Animation.h"

class AnimationClip {

private:
	// start frame
	int StartFrame;
	// end frame
	int EndFrame;
	// animation source data of this clip
	Animation * Source;
	// is looped
	bool Looped;
public:
	AnimationClip();
	~AnimationClip();
};
#endif

