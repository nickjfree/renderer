#include "RenderLight.h"
#include "RenderQueue.h"


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
	//Parameter[hash_string::gRadiusIntensity]= Vector3(Radius, Intensity, 0);
	CullingObj.UniformScale(r);
}
// set color
void RenderLight::SetColor(Vector3& Color_) {
	Color = Color_;
	//Parameter[hash_string::gLightColor]= Color;
}
// set intensity
void RenderLight::SetIntensity(float Intensity_) {
	Intensity = Intensity_;
	//Parameter[hash_string::gRadiusIntensity]= Vector3(Radius, Intensity, 0);
}
// set direction
void RenderLight::SetDirection(Vector3& Direction_) {
	Direction = Direction_;
	//Parameter[hash_string::gLightDirection]= Direction;
}
// set shdowcast disable/enable
void RenderLight::SetShadowCast(int Flag) {
	ShadowCast = Flag;
}

void RenderLight::SetShadowMap(int id) {
	// Parameter[hash_string::gShadowMap]= id;
	ShadowMap = id;
}

RenderingCamera* RenderLight::GetLightCamera() {
	Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.25f * 3.1415926f, 1.0f, 1, Radius);
	LightCamera->FromLight(Position, Rotation, Projection);
	return LightCamera;
}

int RenderLight::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context) {
	return 0;
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
	//		cmdParameters["gLightPosition"] = Position * camera->GetViewMatrix();
	//		cmdParameters["gLightDirection"] = Direction.RotateBy(camera->GetViewMatrix());
	//		cmdParameters["gRadiusIntensity"] = Vector3(Radius, Intensity, 0);
	//		cmdParameters["gLightColor"] = Color;
	//		cmdParameters["gShadowMap"] = ShadowMap;
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
		cmdParameters["gLightPosition"] = Position * camera->GetViewMatrix();
		cmdParameters["gLightDirection"] = Direction.RotateBy(camera->GetViewMatrix());
		cmdParameters["gRadiusIntensity"] = Vector3(Radius, Intensity, 0);
		cmdParameters["gLightColor"] = Color;
		cmdParameters["gShadowMap"] = ShadowMap;
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
	ret.Position = Position;
	ret.Direction = Direction;
	ret.Radius = Radius;
	ret.Color = Color;
	ret.Intensity = Intensity;
	ret.Type = LightType;
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