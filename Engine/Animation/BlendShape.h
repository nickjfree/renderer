
#ifndef __BLENDSHAPE__
#define __BLENDSHAPE__

#include "../Rendering/GPUResource.h"
#include "../Rendering/Mesh.h"
#include "../Container/Vector.h"


class BlendShape : public GPUResource
{

    OBJECT(BlendShape);
    BASEOBJECT(BlendShape);
    LOADEROBJECT(FileLoader);
    DECLAR_ALLOCATER(BlendShape);

private:
    // blendshapes
    Vector<Mesh*> BlendShapes_;
    // blendshape desc

    // total buffer size
    unsigned int BufferSize_;
    // each buffer size
    unsigned int ShapeStride_;
    // each vertex size
    unsigned int VertexStride_;
    // shape count
    int ShapeCount_;
    // 
    Variant OwnerParameter_;
public:
    BlendShape(Context * context);
    virtual ~BlendShape();
    // processing the file describeing blendshapes
    virtual int OnSerialize(Deserializer& deserializer);
    // handle load complete event of each blendshape meshes
    virtual int OnSubResource(int Message, Resource * Sub, Variant& Param);
    // async create all the blend shape meshes
    virtual int OnCreateComplete(Variant& Parameter);
    // on raw data parse complete(worker thread)
    virtual int OnLoadComplete(Variant& Data);
};

#endif