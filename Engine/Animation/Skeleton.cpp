#include "Skeleton.h"


USING_ALLOCATER(Skeleton);

Skeleton::Skeleton(Context* context) : Resource(context) {
	ResourceType = R_SKELETON;
}


Skeleton::~Skeleton() {
}


int Skeleton::OnSerialize(Deserializer& serializer) {

	void* Data = serializer.Raw();
	hb_header* Header = (hb_header*)Data;
	hb_bone* entries = (hb_bone*)((char*)Data + sizeof(hb_header));
	Bone bone;
	for (int i = 0; i < Header->NumBones; i++) {
		hb_bone& entry = entries[i];
		bone.Id = i;
		bone.ParentId = char(entry.parent);
		bone.Name = entry.name;
		Matrix4x4 Mat = Matrix4x4(entry.m00, entry.m01, entry.m02, entry.m03,
			entry.m10, entry.m11, entry.m12, entry.m13,
			entry.m20, entry.m21, entry.m22, entry.m23,
			entry.m30, entry.m31, entry.m32, entry.m33);
		// transpose to make it row-major
		Matrix4x4::Tranpose(Mat, &bone.InverseBindPose);
		// append to bones
		Bones.PushBack(bone);
	}

	return 0;
}


int Skeleton::OnDestroy(Variant& Param) {
	Bones.Empty();
	DeSerial.Release();
	return 0;
}