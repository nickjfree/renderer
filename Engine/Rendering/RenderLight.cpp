#include "RenderLight.h"
#include "RenderQueue.h"
#include "Core\StringTable.h"


USING_ALLOCATER(RenderLight);

//cbuffer Light : register(b5)
//{
//	float4 gLightPosition;
//	float4 gLightColor;
//	float4 gRadiusIntensity;
//};


RenderLight::RenderLight() : Radius(1.0f), Intensity(1.0f), Color(Vector3(0.0f, 0.1f, 0.0f)), ShadowCast(0)
{
	Type = Node::LIGHT;
	LightType = RenderLight::POINT;
	// lightCamera
	LightCamera = new RenderingCamera();
}


RenderLight::~RenderLight()
{
	delete LightCamera;
}

void RenderLight::SetLightType(int Type) {
	LightType = Type;
}

void RenderLight::SetRadius(float r) {
	Radius = r;
	//Parameter[hash_string::gRadiusIntensity].as<Vector3>() = Vector3(Radius, Intensity, 0);
	CullingObj.UniformScale(r);
}
// set color
void RenderLight::SetColor(Vector3& Color_) {
	Color = Color_;
	//Parameter[hash_string::gLightColor].as<Vector3>() = Color;
}
// set intensity
void RenderLight::SetIntensity(float Intensity_) {
	Intensity = Intensity_;
	//Parameter[hash_string::gRadiusIntensity].as<Vector3>() = Vector3(Radius, Intensity, 0);
}
// set direction
void RenderLight::SetDirection(Vector3& Direction_) {
	Direction = Direction_;
	//Parameter[hash_string::gLightDirection].as<Vector3>() = Direction;
}
// set shdowcast disable/enable
void RenderLight::SetShadowCast(int Flag) {
	ShadowCast = Flag;
}

void RenderLight::SetShadowMap(int id) {
	// Parameter[hash_string::gShadowMap].as<int>() = id;
	ShadowMap = id;
}

RenderingCamera* RenderLight::GetLightCamera() {
	Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.25f * 3.1415926f, 1.0f, 1, Radius);
	LightCamera->FromLight(Position, Rotation, Projection);
	return LightCamera;
}

int RenderLight::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context) {
	Stage = 0;
	// process matrix
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Scale;
	Scale.Scale(Vector3(Radius, Radius, Radius));
	Transform = Scale * Transform;
	// perlight position
	Matrix4x4::Tranpose(Transform * Camera->GetViewProjection(), &StageParameter["gWorldViewProjection"].as<Matrix4x4>());
	Matrix4x4::Tranpose(Transform * Camera->GetViewMatrix(), &StageParameter["gWorldViewMatrix"].as<Matrix4x4>());
	// light parameters
	StageParameter["gLightPosition"].as<Vector3>() = Position * Camera->GetViewMatrix();
	StageParameter["gLightDirection"].as<Vector3>() = Direction.RotateBy(Camera->GetViewMatrix());
	StageParameter["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
	StageParameter["gLightColor"].as<Vector3>() = Color;
	StageParameter["gShadowMap"].as<int>() = ShadowMap;
	// StageParameter["gDiffuseMap0"].as<int>() = Context->GetResource("gRtReflection")->as<int>();
	Matrix4x4::Tranpose(LightCamera->GetViewProjection(), &StageParameter["gLightViewProjection"].as<Matrix4x4>());
	// process material
	int Compiled = 0;
	// stencil-pass
	Shader* shader = 0;
	if (material) {
		Compiled += material->Compile(Compiler, Stage, Lod);
		// process shader
		shader = material->GetShader();
		Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
	}
	int Geometry = GetRenderMesh(Stage, Lod);
	if (Geometry != -1 && LightType == POINT) {
		//Compiled += Compiler->SetTransform(Transform);
		Compiled += Compiler->RenderGeometry(Geometry);
	}
	// lighting-pass
	// get quad shader stage types
	switch (LightType) {
	case POINT:
		Stage = 1;
		//return Compiled;
		break;
	case DIRECTION:
		Stage = 2;
		//return Compiled;
		break;
	case ENV:
		Stage = 3;
		// return Compiled;
		break;
	default:
		Stage = 1;
	}
	Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), StageParameter, Context);
	// full screen quad
	Compiled += Compiler->Quad();
	return Compiled;
}

int RenderLight::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Scale;
	Scale.Scale(Vector3(Radius, Radius, Radius));
	Transform = Scale * Transform;
	// draw sphere geometry for point light
	//{
	//	if (LightType == POINT) {
	//		auto cmd = cmdBuffer->AllocCommand();
	//		auto& cmdParameters = cmd->cmdParameters;
	//		// perlight position
	//		Matrix4x4::Tranpose(Transform * camera->GetViewProjection(), &cmdParameters["gWorldViewProjection"].as<Matrix4x4>());
	//		Matrix4x4::Tranpose(Transform * camera->GetViewMatrix(), &cmdParameters["gWorldViewMatrix"].as<Matrix4x4>());
	//		// light parameters
	//		cmdParameters["gLightPosition"].as<Vector3>() = Position * camera->GetViewMatrix();
	//		cmdParameters["gLightDirection"].as<Vector3>() = Direction.RotateBy(camera->GetViewMatrix());
	//		cmdParameters["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
	//		cmdParameters["gLightColor"].as<Vector3>() = Color;
	//		cmdParameters["gShadowMap"].as<int>() = ShadowMap;
	//		Matrix4x4::Tranpose(LightCamera->GetViewProjection(), &cmdParameters["gLightViewProjection"].as<Matrix4x4>());
	//		auto mesh = model->MeshResource[lod];
	//		cmdBuffer->Draw(cmd, mesh, GetMaterial(), 0);
	//	}
	//}
	{
		switch (LightType) {
		case POINT:
			stage = 1;
			return 0;
			break;
		case DIRECTION:
			stage = 2;
			return 0;
			break;
		case ENV:
			stage = 3;
			// return Compiled;
			break;
		default:
			stage = 1;
		}
		// draw full screen quad
		auto cmd = cmdBuffer->AllocCommand();
		auto& cmdParameters = cmd->cmdParameters;
		// light parameters
		cmdParameters["gLightPosition"].as<Vector3>() = Position * camera->GetViewMatrix();
		cmdParameters["gLightDirection"].as<Vector3>() = Direction.RotateBy(camera->GetViewMatrix());
		cmdParameters["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
		cmdParameters["gLightColor"].as<Vector3>() = Color;
		cmdParameters["gShadowMap"].as<int>() = ShadowMap;
		Matrix4x4::Tranpose(LightCamera->GetViewProjection(), &cmdParameters["gLightViewProjection"].as<Matrix4x4>());
		cmdBuffer->Draw(cmd, nullptr, GetMaterial(), stage);
	}
	return 0;
}


int RenderLight::UpdateRaytracingStructure(CommandBuffer* cmdBuffer, RenderingCamera* camera, RenderContext* renderContext) {
	return 0;
}


// get lightdata
LightData RenderLight::GetLightData() 
{
	auto ret = LightData{};
	ret.position = Position;
	ret.direction = Direction;
	ret.radius = Radius;
	ret.color = Color;
	ret.intensity = Intensity;
	ret.type = LightType;
	return ret;
}

Vector3 RenderLight::GetDesc()
{
	if (LightType == DIRECTION) {
		return Vector3(Position.x, Position.y, Position.z, -1.0f);
	} else {
		return Vector3(Position.x, Position.y, Position.z, Radius);
	}
	
}