#include "AnimationCache.h"


USING_ALLOCATER(AnimationCache);
USING_RECYCLE(AnimationCache);


AnimationCache::AnimationCache() {
}


AnimationCache::~AnimationCache() {
}


void AnimationCache::GeneratePalette(Skeleton* skeleton) {

	int ParentId;
	int NumBones = skeleton->Bones.Size();
	// root bone is in place
	Vector3 Identity;
	Quaternion NoRotation;
	for (int BoneId = 0; BoneId < NumBones; BoneId++) {
		Vector3& Translation = Result[BoneId].Translation;
		Quaternion& Rotation = Result[BoneId].Rotation;
		if (!BoneId) {
			Palette[BoneId] = Matrix4x4::FormPositionRotation(Identity, Rotation);
		}
		else {
			Palette[BoneId] = Matrix4x4::FormPositionRotation(Translation, Rotation);
		}

		ParentId = skeleton->Bones[BoneId].ParentId;
		if (ParentId != -1) {
			// update base on parent
			Palette[BoneId] = Palette[BoneId] * Palette[ParentId];
		}
	}
	// apply offset matrix and transpose matrix for redering
	for (int BoneId = 0; BoneId < NumBones; BoneId++) {
		Palette[BoneId] = skeleton->Bones[BoneId].InverseBindPose * Palette[BoneId];
		Matrix4x4::Tranpose(Palette[BoneId], &Palette[BoneId]);
	}
}