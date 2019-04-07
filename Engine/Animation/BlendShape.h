
#ifndef __BLENDSHAPE__
#define __BLENDSHAPE__

#include "../Rendering/H3DMesh.h"
#include "../Container/Vector.h"


class BlendShape : public GPUResource
{

    OBJECT(BlendShape);
    BASEOBJECT(BlendShape);
    LOADEROBJECT(FileLoader);
    DECLAR_ALLOCATER(BlendShape);

private:
    // blendshapes
    Vector<H3DMesh*> BlendShapes;
public:
    BlendShape(Context * context);
    virtual ~BlendShape();
    // processing the file describeing blendshapes
    virtual int OnSerialize(Deserializer& serializer);
    // handle load complete event of each blendshape meshes
    virtual int OnSubResource(int Message, Resource * Sub, Variant& Param);       
    // async create all the blend shape meshes
    virtual int OnCreateComplete(Variant& Parameter);
    // on raw data parse complete(worker thread)
    virtual int OnLoadComplete(Variant& Data) { DeSerial.Release(); return 0; };
};

#endif