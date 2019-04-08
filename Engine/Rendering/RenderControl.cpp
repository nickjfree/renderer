#include "RenderControl.h"
#include "CullingTask.h"
#include "PrepassStage.h"
#include "PostpassStage.h"
#include "ShadowMapStage.h"


RenderControl::RenderControl(RenderContext * Context_) :Context(Context_)
{
    memset(RenderPath, 0, MAX_RENDER_PATH * sizeof(void*));
}


RenderControl::~RenderControl()
{
}


int RenderControl::Initialize() {
    RenderQueue_ = new RenderQueue();
    // init render processer
    RenderProcesser_ = new RenderProcesser(Context);
    RenderingPath * LightPrePass = new RenderingPath(Context);
    RenderStage * Stage = 0;
    // initialize shadow map pass
    Stage = new ShadowMapStage(Context);
    LightPrePass->Stages.PushBack(Stage);
    // initialize light-pre pass
    Stage = new PrepassStage(Context);
    LightPrePass->Stages.PushBack(Stage);
    // initialize post pass
    Stage = new PostpassStage(Context);
    LightPrePass->Stages.PushBack(Stage);
    RenderPath[LIGHT_PRE] = LightPrePass;
    return 0;
}

int RenderControl::Execute() {
    int numCamera = Cameras.Size();



    while (numCamera--) {
        RenderingCamera * cam = Cameras[numCamera];
        StartCamera(cam);
    }

    // wait for tasks finish
    int Count = Events.Size();
    OsEvent::Join(Count, &Events[0], 1);
    Events.Empty();
    // end stages
    RenderingPath * path = RenderPath[LIGHT_PRE];
    int stages = path->Stages.Size();
    for (int i = 0; i < stages; i++) {
        RenderStage * Stage = path->Stages[i];
        Stage->End();
    }
    RenderQueue_->Execute(RenderProcesser_);
    return 0;
}

void RenderControl::StartCamera(RenderingCamera * Camera) {
    // light pre pass only
    RenderingPath * path = RenderPath[LIGHT_PRE];
    int stages = path->Stages.Size();
    for (int i = 0; i < stages; i++) {
        RenderStage * Stage = path->Stages[i];
        Stage->Execute(Camera, spatial, RenderQueue_, Queue, Events);
    }
}

int RenderControl::AddCamera(RenderingCamera * Camera) {
    float pi = 3.141592654f;
    Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.3f*pi, (float)FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT, 1, 1000);
    Camera->SetProjection(Projection);
    Cameras.PushBack(Camera);
    return 0;
}

int RenderControl::RemoveCamera(RenderingCamera * Camera) {

    return 0;
}