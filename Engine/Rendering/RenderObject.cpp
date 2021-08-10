#include "RenderObject.h"
#include "RenderQueue.h"
#include "ShaderLibrary.h"

USING_ALLOCATER(RenderObject);
RenderObject::RenderObject()
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

int RenderObject::SetMaterial(Material* material_) {
	material = material_;
	return 0;
}

void RenderObject::SetTransparent() {
	Type = Node::TRANS;
}

void RenderObject::SetClipmap() {
	Type |= Node::CLIPMAP;
}

int RenderObject::GetRenderMesh(int Stage, int Lod) const {
	// ignore stage
	Mesh* mesh = model->MeshResource[Lod];
	if (mesh) {
		return mesh->GetId();
	}
	return -1;
}

int RenderObject::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context) {
	return 0;
}

int RenderObject::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	// interface
	auto renderInterface = renderContext->GetRenderInterface();

	// set cbInstance
	Matrix4x4& Transform = GetWorldMatrix();
	Matrix4x4::Tranpose(Transform * camera->GetViewProjection(), &cbInstance.gWorldViewProjection);
	Matrix4x4::Tranpose(Transform * camera->GetViewMatrix(), &cbInstance.gWorldViewMatrix);
	Matrix4x4::Tranpose(Transform * camera->GetPrevViewProjection(), &cbInstance.gPrevWorldViewProjection);
	cbInstance.gObjectId = get_object_id() + 1;
	// instance data
	auto mesh = model->MeshResource[lod];
	// add to commandbuffer
	if (material->GetShader()->IsInstance(0)) {
		cmdBuffer->DrawInstanced(mesh, GetMaterial(), 0, &cbInstance, sizeof(CBInstance));
	} 
	else {
		auto& cmd = cmdBuffer->Draw(mesh, GetMaterial(), 0);
		cmd.SetShaderConstant(CB_SLOT(CBInstance), &cbInstance, sizeof(CBInstance));
		// has skinning matrices
		if (cbSkinningMatrices) {
			DeformableBuffer = renderInterface->CreateTransientGeometryBuffer(mesh->GetId());
			cmd.SetRWShaderResource(SLOT_ANIME_DEFORMED_MESH, DeformableBuffer);
			cmd.SetShaderConstant(CB_SLOT(CBSkinningMatrices), cbSkinningMatrices, sizeof(CBSkinningMatrices));
		}
		// terrain mesh
		if (Type & CLIPMAP) {
			DeformableBuffer = renderInterface->CreateTransientGeometryBuffer(mesh->GetId());
			cmd.SetRWShaderResource(SLOT_ANIME_DEFORMED_MESH, DeformableBuffer);
		}
		// if there are  blend shapes
		if (cbBlendShapes) {
			cmd.SetShaderResource(SLOT_ANIME_BLEND_SHAPES, BlendShape_->GetId());
			cmd.SetShaderConstant(CB_SLOT(CBBlendshape), cbBlendShapes, sizeof(CBBlendshape));
		}
	}
	return 0;
}

int RenderObject::UpdateRaytracingStructure(CommandBuffer* cmdBuffer, RenderingCamera* camera, RenderContext* renderContext) 
{
	auto mesh = model->MeshResource[0];
	cmdBuffer->BuildAccelerationStructure(mesh, GetMaterial(), GetWorldMatrix(), DeformableBuffer, 0, 0);
	// the deformableBuffer shoud be re-create each  frame
	DeformableBuffer = -1;
	return 0;
}

void RenderObject::SetMatrixPalette(Matrix4x4* palette_, unsigned int NumMatrix_) {
	palette.Data = palette_;
	palette.Size = sizeof(Matrix4x4) * NumMatrix_;

	cbSkinningMatrices = (CBSkinningMatrices*)palette_;
}

void RenderObject::SetBlendShapeDesc(BSDesc* desc) {
	blendshape_.Data = desc;
	blendshape_.Size = sizeof(BSWeight) * (int)desc->num_weiths + sizeof(float) * 4;

	cbBlendShapes = (CBBlendshape*)desc;
}

void RenderObject::SetBlendShape(BlendShape* Shape) {
	BlendShape_ = Shape;
}
