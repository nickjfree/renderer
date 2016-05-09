#include "RenderObject.h"
#include "RenderQueue.h"

USING_ALLOCATER(RenderObject);
RenderObject::RenderObject()
{
	Type = Node::RENDEROBJECT;
	Matrix4x4 InitMatrix;
	// init the need keys in dict. so the rendereing process wouln't create them. in case the multi-thread creation problem
	Parameter["gWorldViewProjection"].as<Matrix4x4>() = InitMatrix;
	Parameter["gWorldViewMatrix"].as<Matrix4x4>() = InitMatrix;
	Parameter["gInvertViewMaxtrix"].as<Matrix4x4>() = InitMatrix;
	Parameter["gProjectionMatrix"].as<Matrix4x4>() = InitMatrix;
	Parameter["gViewPoint"].as<Vector3>() = Vector3(0,0,0);
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

int RenderObject::GetRenderMesh(int Stage, int Lod) {
	// ignore stage
	Mesh * mesh = model->MeshResource[Lod];
	if (mesh) {
		return mesh->GetId();
	}
	return -1;
}

int RenderObject::Compile(BatchCompiler * Compiler, int Stage, int Lod, RenderingCamera * Camera, RenderContext * Context){
	if (Stage == R_STAGE_PREPASSS) {
		Stage = 0;
	}
	if (Stage == R_STAGE_SHADING) {
		Stage = 1;
	}
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewProjection(), &Tmp);
	Parameter["gWorldViewProjection"].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewMatrix(), &Tmp);
	Parameter["gWorldViewMatrix"].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetInvertView(), &Tmp);
	Parameter["gInvertViewMaxtrix"].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetProjection(), &Tmp);
	Parameter["gProjectionMatrix"].as<Matrix4x4>() = Tmp;
	Parameter["gViewPoint"].as<Vector3>() = Camera->GetViewPoint();
	// process matrix
	// process material
	int Compiled = 0;
	if (material) {
		Compiled += material->Compile(Compiler, Stage, Lod);
		// process shader
		Shader * shader = material->GetShader();
		Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), Parameter, Context);
	}
	int Geometry = GetRenderMesh(Stage, Lod);
	if (Geometry != -1) {
		//Compiled += Compiler->SetTransform(Transform);
		Compiled += Compiler->RenderGeometry(Geometry);
	}
	return Compiled;
}