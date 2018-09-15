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
	Matrix4x4 InitMatrix;
	Parameter[hash_string::gRadiusIntensity].as<Vector3>() = Vector3(Radius, Intensity, 0);
	Parameter[hash_string::gLightColor].as<Vector3>() = Color;
	Parameter[hash_string::gScreenSize].as<Vector2>() = Vector2(0,0);
	Parameter[hash_string::gLightViewProjection].as<Matrix4x4>() = InitMatrix;
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
	Parameter[hash_string::gRadiusIntensity].as<Vector3>() = Vector3(Radius, Intensity, 0);
	CullingObj.UniformScale(r);
}
// set color
void RenderLight::SetColor(Vector3& Color_) {
	Color = Color_;
	Parameter[hash_string::gLightColor].as<Vector3>() = Color;
}
// set intensity
void RenderLight::SetIntensity(float Intensity_) {
	Intensity = Intensity_;
	Parameter[hash_string::gRadiusIntensity].as<Vector3>() = Vector3(Radius, Intensity, 0);
}
// set direction
void RenderLight::SetDirection(Vector3& Direction_) {
	Direction = Direction_;
	Parameter[hash_string::gLightDirection].as<Vector3>() = Direction;
}
// set shdowcast disable/enable
void RenderLight::SetShadowCast(int Flag) {
	ShadowCast = Flag;
}

void RenderLight::SetShadowMap(int id) {
	Parameter[hash_string::gShadowMap].as<int>() = id;
}

RenderingCamera * RenderLight::GetLightCamera() {
	Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.25f * 3.1415926f, 1.0f, 1, Radius);
	LightCamera->FromLight(Position, Rotation, Projection);
	return LightCamera;
}


void RenderLight::UpdateLightView() {
	Matrix4x4::Tranpose(LightCamera->GetViewProjection(), &Parameter[hash_string::gLightViewProjection].as<Matrix4x4>());
}

int RenderLight::Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera * Camera, RenderContext * Context){
	Stage = 0;
	// process matrix
	Matrix4x4 Transform = GetWorldMatrix();
	Matrix4x4 Scale;
	Scale.Scale(Vector3(Radius, Radius, Radius));
	Transform = Scale * Transform;
	Matrix4x4 Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewProjection(), &Tmp);
	Parameter[hash_string::gWorldViewProjection].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Transform * Camera->GetViewMatrix(), &Tmp);
	Parameter[hash_string::gWorldViewMatrix].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetInvertView(), &Tmp);
	Parameter[hash_string::gInvertViewMaxtrix].as<Matrix4x4>() = Tmp;
	Matrix4x4::Tranpose(Camera->GetProjection(), &Tmp);
	Parameter[hash_string::gProjectionMatrix].as<Matrix4x4>() = Tmp;
	Parameter[hash_string::gViewPoint].as<Vector3>() = Camera->GetViewPoint();
	// light parameters
	Parameter[hash_string::gLightPosition].as<Vector3>() = Position * Camera->GetViewMatrix();
	Parameter[hash_string::gLightDirection].as<Vector3>() = Direction.RotateBy(Camera->GetViewMatrix());
	Parameter[hash_string::gScreenSize].as<Vector2>() = Vector2(Context->FrameWidth, Context->FrameHeight);
	// light iewprojection
	UpdateLightView();
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
	if (Geometry != -1 && LightType == POINT) {
		//Compiled += Compiler->SetTransform(Transform);
		Compiled += Compiler->RenderGeometry(Geometry);
	}
	// lighting-pass
	// get quad shader stage types
	switch (LightType) {
	case POINT:
		Stage = 1;
		break;
	case DIRECTION:
		Stage = 2;
		break;
	case ENV:
		Stage = 3;
		break;
	default:
		Stage = 1;
	}
	Compiled += shader->Compile(Compiler, Stage, Lod, material->GetParameter(), Parameter, Context);
	// full screen quad
	Compiled += Compiler->Quad();
	return Compiled;
}
