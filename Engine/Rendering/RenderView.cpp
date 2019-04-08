#include "RenderView.h"


USING_ALLOCATER(RenderView);
USING_RECYCLE(RenderView);

RenderView::RenderView() : ClearTargets(1)
{
    CommandBuffer = new char[COMMANDBUFFER_SIZE];
    Compiler = new BatchCompiler();
    Event = OsEvent::Create();
}


RenderView::~RenderView()
{
    Event->Recycle();
    delete CommandBuffer;
    delete Compiler;
}

int RenderView::Compile(RenderContext * Context) {
    int Compiled = 0;
    // config rendertargets    
    Compiled += Compiler->SetDepthBuffer(Depth);
    Compiled += Compiler->SetRenderTargets(TargetCount, Targets);
    if (ClearTargets) {
        Compiled += Compiler->ClearRenderTarget();
    }
    if (ClearDepth) {
        Compiled += Compiler->ClearDepthStencil();
    }
    // config perframe. eg. camera
    Parameters.Clear();
    Matrix4x4::Tranpose(Camera->GetInvertView(), &Parameters["gInvertViewMaxtrix"].as<Matrix4x4>());
    Matrix4x4::Tranpose(Camera->GetProjection(), &Parameters["gProjectionMatrix"].as<Matrix4x4>());
    Parameters["gViewPoint"].as<Vector3>() = Camera->GetViewPoint();
    Parameters["gScreenSize"].as<Vector2>() = Vector2(static_cast<float>(Context->FrameWidth), static_cast<float>(Context->FrameHeight));
    return 0;
}


int RenderView::QueueCommand() {
    int Stage = Type + Index;
    if (TargetCount && !Targets[0]) {
        // only backbuffer need present
        Compiler->Present();
    }
    Compiler->EndBuffer();
    if (Compiler->GetCommandSize()) {
        Queue->PushCommand(Stage, CommandBuffer);
    }
    return 1;
}
