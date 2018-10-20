#include "RenderObject.h"
#include "RenderQueue.h"
#include "Core\StringTable.h"

USING_ALLOCATER(RenderObject);
RenderObject::RenderObject()
{
	Type = Node::RENDEROBJECT;
	Matrix4x4 InitMatrix;
	// init the need keys in dict. so the rendereing process wouln't create them. in case the multi-thread creation problem
	Parameter[hash_string::gWorldViewProjection].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::gWorldViewMatrix].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::gInvertViewMaxtrix].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::gProjectionMatrix].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::InstanceWVP].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::InstanceWV].as<Matrix4x4>() = InitMatrix;
	Parameter[hash_string::gViewPoint].as<Vector3>() = Vector3(0,0,0);
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

int RenderObject::GetRenderMesh(int Stage, int Lod) {
	// ignore stage
	Mesh * mesh = model->MeshResource[Lod];
	if (mesh) {
		return mesh->GetId();
	}
	return -1;
}

int RenderObject::Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera * Camera, RenderContext * Context){
	if (Stage == R_STAGE_PREPASSS) {
		Stage = 0;
	} else if (Stage == R_STAGE_SHADING ) {
		Stage = 1;
	} else if (Stage == R_STAGE_SHADOW) {
		Stage = 2;
	} else if (Stage == R_STAGE_OIT) {
        Stage = 3;
    }
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewProjection(), &Tmp);
	StageParameter[hash_string::gWorldViewProjection].as<Matrix4x4>() = Tmp;
	StageParameter[hash_string::InstanceWVP].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewMatrix(), &Tmp);
	StageParameter[hash_string::gWorldViewMatrix].as<Matrix4x4>() = Tmp;
	StageParameter[hash_string::InstanceWV].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetInvertView(), &Tmp);
	StageParameter[hash_string::gInvertViewMaxtrix].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetProjection(), &Tmp);
	StageParameter[hash_string::gProjectionMatrix].as<Matrix4x4>() = Tmp;
	StageParameter[hash_string::gViewPoint].as<Vector3>() = Camera->GetViewPoint();
	// if has skinning matrix
	if (palette.Size) {
		StageParameter[hash_string::gSkinMatrix].as<ShaderParameterArray>() = palette;
	}
	int Compiled = 0;
	int Instance = 0;
	int InstanceSize = 0;
	Shader * shader = 0;
	//int a = 100 / (int)shader;
	if (material) {
		Compiled += material->Compile(Compiler, Stage, Lod);
		// process shader
		shader = material->GetShader();
		Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
		Instance = shader->IsInstance(Stage);
	}
	int Geometry = GetRenderMesh(Stage, Lod);
	if (Geometry != -1) {
		if (Instance) {
			unsigned char InstanceBuffer[64 * 4];
			InstanceSize = shader->MakeInstance(Compiler, Stage, StageParameter, InstanceBuffer);
			Compiled += Compiler->Instance(Geometry, InstanceBuffer, InstanceSize);
		} else {
			Compiled += Compiler->RenderGeometry(Geometry);
		}
	}
	return Compiled;
}


void RenderObject::SetMatrixPalette(Matrix4x4 * palette_, unsigned int NumMatrix_) {
	palette.Data = palette_;
	palette.Size = sizeof(Matrix4x4) * NumMatrix_;
}