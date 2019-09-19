#ifndef __RENDEROBJ_H__
#define __RENDEROBJ_H__


#include "Node.h"
#include "Model.h"
#include "Rendering\Material.h"
#include "Animation\BlendShape.h"

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
    // Dict Parameter;
    // pre create keys for sorting
    unsigned int SortKeyHigh;
    unsigned int SortKeyLow;
    // InstanceBuffer
    unsigned char InstanceBuffer[64 * 4];
    // matrix palette
    ShaderParameterArray palette;
    // blendshapes parameters for shader
    ShaderParameterArray blendshape_;
    // blendshape
    BlendShape * BlendShape_;
public:
    RenderObject();
    virtual ~RenderObject();
    // set model
    int SetModel(Model* model);
    // set material
    int SetMaterial(Material* material);
    // set type
    void SetTransparent();
    // get model
    Model * GetModel() const { return model; }
	// get material
	Material* GetMaterial() const{ return material; }
    // get render mesh
    virtual int GetRenderMesh(int Stage, int Lod) const;
    virtual int Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera * Camera, RenderContext * Context);
    // set matrix palette
    void SetMatrixPalette(Matrix4x4 * palette, unsigned int NumMatrix);
    // set blendshape desc
    void SetBlendShapeDesc(BSDesc * desc);
    // set blendshap data
    void SetBlendShape(BlendShape * Shape);
};

#endif