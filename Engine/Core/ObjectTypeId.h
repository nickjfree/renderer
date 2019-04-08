#ifndef __OBJECT_TYPE__
#define __OBJECT_TYPE__

/*
A type-class for a certain class, only the id is usefull
*/
class ObjectTypeId {
private:
    static unsigned int TypeId;
    unsigned int InstanceId;
    unsigned int ClassId;
public:
    ObjectTypeId();
    ~ObjectTypeId();
    unsigned int GetId();
};

#endif