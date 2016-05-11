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
	Parameter["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
	Parameter["gLightColor"].as<Vector3>() = Color;
	Parameter["gScreenSize"].as<Vector2>() = Vector2(1920,1080);
}


RenderLight::~RenderLight()
{
}

void RenderLight::SetRadius(float r) {
	Radius = r;
	Parameter["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
	CullingObj.UniformScale(r);
}
// set color
void RenderLight::SetColor(Vector3& Color_) {
	Color = Color_;
	Parameter["gLightColor"].as<Vector3>() = Color;
}
// set intensity
void RenderLight::SetIntensity(float Intensity_) {
	Intensity = Intensity_;
	Parameter["gRadiusIntensity"].as<Vector3>() = Vector3(Radius, Intensity, 0);
}
// set shdowcast disable/enable
void RenderLight::SetShadowCast(int Flag) {
	ShadowCast = Flag;
}

void RenderLight::SetShadowMap(int id) {
	Parameter["gShadowMap"].as<int>() = id;
}

RenderingCamera * RenderLight::GetLightCamera() {
	Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.25f * 3.1415926f, 1.0f, 1, Radius);
	LightCamera.FromLight(Position, Rotation, Projection);
	return &LightCamera;
}


int RenderLight::Compile(BatchCompiler * Compiler, int Stage, int Lod, RenderingCamera * Camera, RenderContext * Context){
	Stage = 0;
	// process matrix
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Scale;
	Scale.Scale(Vector3(Radius, Radius, Radius));
	Transform = Scale * Transform;
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
	// light parameters
	Parameter["gLightPosition"].as<Vector3>() = Position * Camera->GetViewMatrix();

	// process material
	int Compiled = 0;
	// stencil-pass
	Shader * shader = 0;
	if (material) {
		Compiled += material->Compile(Compiler, Stage, Lod);
		// process shader
		shader = material->GetShader();
		Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), Parameter, Context);
	}
	int Geometry = GetRenderMesh(Stage, Lod);
	if (Geometry != -1) {
		//Compiled += Compiler->SetTransform(Transform);
		Compiled += Compiler->RenderGeometry(Geometry);
	}
	// lighting-pass
	Stage = 1;
	Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), Parameter, Context);
	// full screen quad
	Compiled += Compiler->Quad();
	return Compiled;
}
