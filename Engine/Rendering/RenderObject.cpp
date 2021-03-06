#include "RenderObject.h"
#include "RenderQueue.h"
#include "Core\StringTable.h"
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
	Matrix4x4::Tranpose(Transform * Camera->GetPrevViewProjection(), &StageParameter["gPrevWorldViewProjection"].as<Matrix4x4>());
	// instance data
	StageParameter[hash_string::InstanceWV].as<Matrix4x4>() = StageParameter[hash_string::gWorldViewMatrix].as<Matrix4x4>();
	StageParameter[hash_string::InstanceWVP].as<Matrix4x4>() = StageParameter[hash_string::gWorldViewProjection].as<Matrix4x4>();
	// constexpr String PWVP("InstancePWVP");
	StageParameter["InstancePWVP"].as<Matrix4x4>() = StageParameter["gPrevWorldViewProjection"].as<Matrix4x4>();
	// object id
	StageParameter["gObjectId"].as<int>() = get_object_id() + 1;
	StageParameter["InstanceObjectId"].as<int>() = StageParameter["gObjectId"].as<int>();
	// get geometry
	int Geometry = GetRenderMesh(Stage, Lod);
	// if there is a skinning matrix or is a terrain.
	if (palette.Size || Type & CLIPMAP) {
		StageParameter["gSkinMatrix"].as<ShaderParameterArray>() = palette;
		// deformabled buffer
		if (Stage == 0 && DeformableBuffer != -1) {
			StageParameter["gDeformableBuffer"].as<unsigned int>() = DeformableBuffer;
		} else if (Stage == 0 && Geometry != -1) {
			RaytracingGeometry = Context->GetRenderInterface()->CreateRaytracingGeometry(Geometry, true, &DeformableBuffer);
		}
	}
	// if there are  blend shapes
	if (BlendShape_) {
		StageParameter["gBlendShapes"].as<unsigned int>() = BlendShape_->GetId();
		StageParameter["gWeightsArray"].as<ShaderParameterArray>() = blendshape_;
	}

	int Compiled = 0;
	int Instance = 0;
	int InstanceSize = 0;
	Shader* shader = 0;
	// prepare material 
	if (material) {
		Compiled += material->Compile(Compiler, Stage, Lod);
		// process shader
		shader = material->GetShader();
		Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
		Instance = shader->IsInstance(Stage);
	}
	// prepare batch
	if (Geometry != -1) {
		if (Instance) {
			unsigned char InstanceBuffer[64 * 4];
			InstanceSize = shader->MakeInstance(Stage, StageParameter, InstanceBuffer);
			Compiled += Compiler->Instance(Geometry, InstanceBuffer, InstanceSize);
		}
		else {
			Compiled += Compiler->RenderGeometry(Geometry);
		}
	}
	return Compiled;
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
	cmdParameters["InstanceWV"].as<Matrix4x4>() = cmdParameters["gWorldViewMatrix"].as<Matrix4x4>();
	cmdParameters["InstanceWVP"].as<Matrix4x4>() = cmdParameters["gWorldViewProjection"].as<Matrix4x4>();
	// constexpr String PWVP("InstancePWVP");
	cmdParameters["InstancePWVP"].as<Matrix4x4>() = cmdParameters["gPrevWorldViewProjection"].as<Matrix4x4>();
	// object id
	cmdParameters["gObjectId"].as<int>() = get_object_id() + 1;
	cmdParameters["InstanceObjectId"].as<int>() = get_object_id() + 1;
	auto mesh = model->MeshResource[lod];
	if (palette.Size || Type & CLIPMAP) {
		cmdParameters["gSkinMatrix"].as<ShaderParameterArray>() = palette;
		if (stage == 0) {
			DeformableBuffer = renderInterface->CreateTransientGeometryBuffer(mesh->GetId());
			cmdParameters["gDeformableBuffer"].as<int>() = DeformableBuffer;
		}
	}
	// if there are  blend shapes
	if (BlendShape_) {
		cmdParameters["gBlendShapes"].as<unsigned int>() = BlendShape_->GetId();
		cmdParameters["gWeightsArray"].as<ShaderParameterArray>() = blendshape_;
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
