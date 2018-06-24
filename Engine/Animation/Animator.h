#ifndef __ANIMATOR__
#define __ANIMATOR__
#include "../Scene/Component.h"

#include "AnimationClip.h"


class Animator : public Component {
	OBJECT(Animator);
	BASEOBJECT(Animator);
	DECLAR_ALLOCATER(Animator);

private:
	// current playing clip, for none blending animation only
	AnimationClip * Clip;

public:
	Animator(Context * context);
	virtual ~Animator();
};

#endif