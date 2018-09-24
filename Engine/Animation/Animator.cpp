#include "Animator.h"
#include "AnimationSystem.h"
#include "Rendering\Renderer.h"
#include "Rendering\MeshRenderer.h"


USING_ALLOCATER(Animator)

Animator::Animator(Context * context): Component(context) {
}


Animator::~Animator() {
}

void Animator::Update(float time) {
	// advance in time
	Stage->Advance(time);
	Stage->Apply();
	// get the result
	AnimationCache * Cache = Stage->GetAnimationCache();
	// set palette
	Cache->GeneratePalette(skeleton);
	MeshRenderer * renderer = (MeshRenderer *)Owner->GetComponent(String("Renderer"));
	renderer->SetMatrixPalette(Cache->Palette, Cache->Result.Size());
	// apply root motion
	Matrix4x4 Tanslation = Matrix4x4::FormPositionRotation(Stage->MotonDelta, Quaternion());
	Tanslation = Tanslation * Owner->GetTransform();
	Owner->SetTransform(Tanslation);
	//Owner->SetRotation(Stage->RootRotation);
}

void Animator::SetAnimationStage(int Layer, AnimationClip * Clip, unsigned char StartBone, float Scale) {
	Stage = new AnimationStage();
	Stage->SetAnimationClip(Clip);
	Stage->SetTime(0.0f);
	Stage->SetScale(Scale);
}

int Animator::OnAttach(GameObject * GameObj) {
	
	AnimationSystem * animationSys = context->GetSubsystem<AnimationSystem>();
	animationSys->AddAnimator(this);
	return 0;
}