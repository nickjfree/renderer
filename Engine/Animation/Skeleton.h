#ifndef __SKELETON__
#define __SKELETON__


#include "Container\Vector.h"
#include "Math\LinearMath.h"


#define NUM_BONES 128

/*
	Bone
*/
typedef struct Bone {
	Matrix4x4 InverseBindPose;
	int ParentId;
	int Id;
	String Name;
}Bone;


/*
	bone hierarchy
*/
class Skeleton {
private:
	// bones
	Vector<Bone> Bones;
public:
	Skeleton();
	~Skeleton();
};

#endif
