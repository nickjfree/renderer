#include "Animation.h"


USING_ALLOCATER(Animation);

Animation::Animation(Context * context) : Resource(context) {
	ResourceType = R_ANIMATION;
}

Animation::~Animation() {
}

int Animation::OnSerialize(Deserializer& serializer) {

	ha_header * Header = (ha_header*)serializer.Raw();
	ha_clip * clips = (ha_clip*)((char*)Header + sizeof(ha_header));
	ha_frame * keyframes = (ha_frame*)((char*)Header + Header->OffsetFrames);
	
	// make space for all channels
	Tracks.Resize(Header->NumChannels + 10, 1);
	int NumFrames = Header->NumFrames;
	for (int channel = 0; channel < Header->NumChannels; channel++) {
		ha_frame * frames = keyframes + channel * NumFrames;
		char bone_id = frames->bone_id;
		Tracks[bone_id].Frames.Resize(NumFrames, 1);
		for (int frame = 0; frame < NumFrames; frame++) {
			KeyFrame& keyframe = Tracks[bone_id].Frames[frame];
			ha_frame& raw_frame = frames[frame];
			keyframe.BoneId = bone_id;
			keyframe.Time = raw_frame.time;
			keyframe.Rotation = Quaternion(raw_frame.rx, raw_frame.ry, raw_frame.rz, raw_frame.rw);
			keyframe.Translation = Vector3(raw_frame.tx, raw_frame.ty, raw_frame.tz);
			keyframe.Scale = 1.0f;
		}
	}
	for (int i=0; i < Header->NumClips; i++) {
		AnimationClip Clip;
		ha_clip& raw_clip = clips[i];
		Clip.StartFrame = raw_clip.start;
		Clip.EndFrame = raw_clip.end;
		Clip.Looped = raw_clip.looped;
		Clip.Source = this;
		ha_frame * start = keyframes + raw_clip.start;
		ha_frame * end = keyframes + raw_clip.end;
		Clip.TimeOffset = start->time;
		Clip.EndTime = end->time;
		Clip.Name = raw_clip.name;
		Clips.PushBack(Clip);
	}
	return 0;
}
