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
	auto cmd = cmdBuffer->AllocCommand();
	auto& cmdParameters = cmd->cmdParameters;
	if (stage == R_STAGE_GBUFFER) {
		stage = 0;
	} else if (stage == R_STAGE_SHADOW) {
		stage = 2;
	} else if (stage == R_STAGE_OIT) {
		stage = 3;
	}
	// interface
	auto renderInterface = renderContext->GetRenderInterface();
	// prepare perObject constants
	Matrix4x4& Transform = GetWorldMatrix();
	// per-object position
	Matrix4x4::Tranpose(Transform * camera->GetViewProjection(), &cmdParameters["gWorldViewProjection"].as<Matrix4x4>());
	Matrix4x4::Tranpose(Transform * camera->GetViewMatrix(), &cmdParameters["gWorldViewMatrix"].as<Matrix4x4>());
	Matrix4x4::Tranpose(Transform * camera->GetPrevViewProjection(), &cmdParameters["gPrevWorldViewProjection"].as<Matrix4x4>());
	// instance data
	cmdParameters["InstanceWV"] = cmdParameters["gWorldViewMatrix"];
	cmdParameters["InstanceWVP"] = cmdParameters["gWorldViewProjection"];
	// constexpr String PWVP("InstancePWVP");
	cmdParameters["InstancePWVP"] = cmdParameters["gPrevWorldViewProjection"];
	// object id
	cmdParameters["gObjectId"] = get_object_id() + 1;
	cmdParameters["InstanceObjectId"] = get_object_id() + 1;
	auto mesh = model->MeshResource[lod];
	if (palette.Size || Type & CLIPMAP) {
		cmdParameters["gSkinMatrix"] = palette;
		if (stage == 0) {
			DeformableBuffer = renderInterface->CreateTransientGeometryBuffer(mesh->GetId());
			cmdParameters["gDeformableBuffer"] = DeformableBuffer;
		}
	}
	// if there are  blend shapes
	if (BlendShape_) {
		cmdParameters["gBlendShapes"] = BlendShape_->GetId();
		cmdParameters["gWeightsArray"] = blendshape_;
	} 
	// add to commandbuffer
	if (material->GetShader()->IsInstance(stage)) {
		cmdBuffer->DrawInstanced(cmd, mesh, GetMaterial(), stage);
	} else {
		cmdBuffer->Draw(cmd, mesh, GetMaterial(), stage);
	}
	return 0;
}

int RenderObject::UpdateRaytracingStructure(CommandBuffer* cmdBuffer, RenderingCamera* camera, RenderContext* renderContext) 
{
	auto cmd = cmdBuffer->AllocCommand();
	auto mesh = model->MeshResource[0];
	cmdBuffer->BuildAccelerationStructure(cmd, mesh, GetMaterial(), GetWorldMatrix(), DeformableBuffer, 0, 0);
	// the deformableBuffer shoud be re-create each  frame
	DeformableBuffer = -1;
	return 0;
}

void RenderObject::SetMatrixPalette(Matrix4x4* palette_, unsigned int NumMatrix_) {
	palette.Data = palette_;
	palette.Size = sizeof(Matrix4x4) * NumMatrix_;
}

void RenderObject::SetBlendShapeDesc(BSDesc* desc) {
	blendshape_.Data = desc;
	blendshape_.Size = sizeof(BSWeight) * (int)desc->num_weiths + sizeof(float) * 4;
}

void RenderObject::SetBlendShape(BlendShape* Shape) {
	BlendShape_ = Shape;
}
