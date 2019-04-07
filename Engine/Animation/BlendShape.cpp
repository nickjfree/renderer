#include "BlendShape.h"


USING_ALLOCATER(BlendShape);

BlendShape::BlendShape(Context * context): GPUResource(context) 
{
}


BlendShape::~BlendShape() 
{
}

int BlendShape::OnSerialize(Deserializer& serializer) 
{
    return 0;
}

int BlendShape::OnSubResource(int Message, Resource * Sub, Variant & Param)
{
    return 0;
}

int BlendShape::OnCreateComplete(Variant & Parameter)
{
    return 0;
}
