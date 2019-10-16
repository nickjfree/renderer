#include "AnimationTrack.h"



AnimationTrack::AnimationTrack() {
}


AnimationTrack::~AnimationTrack() {
}

void AnimationTrack::Sample(float time, int start, int end, KeyFrame** left, KeyFrame** right, float* alpha) {
	int lindex = start;
	int rindex = end;
	if (!Frames.Size()) {
		*left = 0;
		*right = 0;
		*alpha = 0.0f;
		return;
	}
	while (lindex < rindex - 1) {
		int mindex = (lindex + rindex) / 2;
		KeyFrame& frame = Frames[mindex];
		if (frame.Time <= time) {
			lindex = mindex;
		}
		else {
			rindex = mindex;
		}
	}
	*left = &Frames[lindex];
	*right = &Frames[rindex];
	if (lindex == rindex) {
		*alpha = 0.5f;
	}
	else {
		*alpha = (time - Frames[lindex].Time) / (Frames[rindex].Time - Frames[lindex].Time);
	}
}
