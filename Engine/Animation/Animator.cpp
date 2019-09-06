#include "Animator.h"
#include "AnimationSystem.h"
#include "Rendering\Renderer.h"
#include "Rendering\MeshRenderer.h"


USING_ALLOCATER(Animator)

Animator::Animator(Context * context) : Component(context), BlendNode(nullptr) {
}


Animator::~Animator() {
}

void Animator::Update(float time) {
    // advance in time
    //Stage->Advance(time);
    //Stage->Apply();
    if (BlendNode) {
        BlendNode->Advance(time);
        BlendNode->Apply();
        // get the result
        AnimationCache * Cache = BlendNode->GetAnimationCache();
        // set palette
        Cache->GeneratePalette(skeleton);
        MeshRenderer * renderer = (MeshRenderer *)Owner->GetComponent("Renderer");
        renderer->SetMatrixPalette(Cache->Palette, Cache->Result.Size());
    }
    // apply root motion
    //Matrix4x4 Tanslation = Matrix4x4::FormPositionRotation(Stage->MotionDelta, Quaternion());
    //Tanslation = Tanslation * Owner->GetTransform();
    //Owner->SetTransform(Tanslation);
    //Owner->SetRotation(Stage->RootRotation);
}

void Animator::SetAnimationStage(int Layer, AnimationClip * Clip, unsigned char StartBone, float Scale) {
    Stage = new AnimationStage();
    Stage->SetAnimationClip(Clip);
    Stage->SetTime(0.0f);
    Stage->SetScale(Scale);
}

void Animator::SetBlendingNode(BlendingNode * Node) {
    BlendNode = Node;
}

BlendingNode * Animator::GetBlendingNode() {
    return BlendNode;
}

int Animator::OnAttach(GameObject * GameObj) {

    AnimationSystem * animationSys = context->GetSubsystem<AnimationSystem>();
    animationSys->AddAnimator(this);
    return 0;
}

int Animator::OnDestroy(GameObject * GameObj) {
    // call parent
    AnimationSystem * animationSys = context->GetSubsystem<AnimationSystem>();
    animationSys->RemoveAnimator(this);
    Component::OnDestroy(GameObj);
    if (BlendNode) {
        BlendNode->DecRef();
    }
    return 0;
}
