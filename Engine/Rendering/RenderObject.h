#ifndef __RENDEROBJ_H__
#define __RENDEROBJ_H__


#include "Node.h"
#include "Model.h"
#include "Rendering\Material.h"

#include "Container\Dict.h"
#include "RenderingCamera.h"
#include "RenderContext.h"


class RenderObject : public Node
{
	DECLAR_ALLOCATER(RenderObject);
protected:
	// geometry
	Model * model;
	// Material
	Material * material;
	// parameter
	Dict Parameter;
	// pre create keys for sorting
	unsigned int SortKeyHigh;
	unsigned int SortKeyLow;
public:
	RenderObject();
	virtual ~RenderObject();
	// set model
	int SetModel(Model* model);
	// set material
	int SetMaterial(Material* material);
	// get render mesh
	virtual int GetRenderMesh(int Stage, int Lod);
	virtual int Compile(BatchCompiler * Compiler, int Stage, int Lod, RenderingCamera * Camera, RenderContext * Context);
};

#endif