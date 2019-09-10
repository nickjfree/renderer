#include "RenderObject.h"
#include "RenderQueue.h"
#include "Core\StringTable.h"

USING_ALLOCATER(RenderObject);
RenderObject::RenderObject() : BlendShape_(nullptr)
{
    Type = Node::RENDEROBJECT;
}


RenderObject::~RenderObject()
{
}

int RenderObject::SetModel(Model* model_) {
    model = model_;
    CullingObj = model->MeshResource[0]->GetAABB();
    CullingObj.Translate(Position);
    return 0;
}

int RenderObject::SetMaterial(Material * material_) {
    material = material_;
    return 0;
}

void RenderObject::SetTransparent() {
    Type = Node::TRANS;
}

int RenderObject::GetRenderMesh(int Stage, int Lod) const {
    // ignore stage
    Mesh * mesh = model->MeshResource[Lod];
    if (mesh) {
        return mesh->GetId();
    }
    return -1;
}

int RenderObject::Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera * Camera, RenderContext * Context) {

    if (Stage == R_STAGE_PREPASSS) {
        Stage = 0;
    }
    else if (Stage == R_STAGE_SHADING) {
        Stage = 1;
    }
    else if (Stage == R_STAGE_SHADOW) {
        Stage = 2;
    }
    else if (Stage == R_STAGE_OIT) {
        Stage = 3;
    }
    // prepare perObject constants
    Matrix4x4& Transform = GetWorldMatrix();
    // per-object position
    Matrix4x4::Tranpose(Transform * Camera->GetViewProjection(), &StageParameter[hash_string::gWorldViewProjection].as<Matrix4x4>());
    Matrix4x4::Tranpose(Transform * Camera->GetViewMatrix(), &StageParameter[hash_string::gWorldViewMatrix].as<Matrix4x4>());
    // if there is a skinning matrix
    if (palette.Size) {
        StageParameter["gSkinMatrix"].as<ShaderParameterArray>() = palette;
    }
    // if there are  blend shapes
    if (BlendShape_) {
        StageParameter["gBlendShapes"].as<unsigned int>() = BlendShape_->GetId();
        StageParameter["gWeightsArray"].as<ShaderParameterArray>() = blendshape_;

    }

    int Compiled = 0;
    int Instance = 0;
    int InstanceSize = 0;
    Shader * shader = 0;
    // prepare material 
    if (material) {
        Compiled += material->Compile(Compiler, Stage, Lod);
        // process shader
        shader = material->GetShader();
        Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
        Instance = shader->IsInstance(Stage);
    }
    // prepare batch
    int Geometry = GetRenderMesh(Stage, Lod);
    if (Geometry != -1) {
        if (Instance) {
            unsigned char InstanceBuffer[64 * 4];
            InstanceSize = shader->MakeInstance(Compiler, Stage, StageParameter, InstanceBuffer);
            Compiled += Compiler->Instance(Geometry, InstanceBuffer, InstanceSize);
        }
        else {
            Compiled += Compiler->RenderGeometry(Geometry);
        }
    }
    return Compiled;
}


void RenderObject::SetMatrixPalette(Matrix4x4 * palette_, unsigned int NumMatrix_) {
    palette.Data = palette_;
    palette.Size = sizeof(Matrix4x4) * NumMatrix_;
}

void RenderObject::SetBlendShapeDesc(BSDesc * desc) {
    blendshape_.Data = desc;
    blendshape_.Size = sizeof(BSWeight) * (int)desc->num_weiths + sizeof(float) * 4;
}

void RenderObject::SetBlendShape(BlendShape * Shape) {
    BlendShape_ = Shape;
}
