#include "Mesh.h"

USING_ALLOCATER(Mesh);

Mesh::Mesh(Context * context) : GPUResource(context), NumConvex(0)
{
}


Mesh::~Mesh()
{
}
