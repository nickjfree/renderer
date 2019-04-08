#ifndef __SKELETON__
#define __SKELETON__


#include "Container\Vector.h"
#include "Math\LinearMath.h"
#include "Resource\Resource.h"
#include "Misc\h3d\h3d.h"

#define NUM_BONES 128

using namespace h3d;

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

class Skeleton : public Resource {

    BASEOBJECT(Skeleton);
    OBJECT(Skeleton);
    DECLAR_ALLOCATER(Skeleton);
    LOADEROBJECT(FileLoader);

public:
    // bones
    Vector<Bone> Bones;
public:
    Skeleton(Context * context);
    ~Skeleton();

    virtual int OnSerialize(Deserializer& serializer);

};

#endif
