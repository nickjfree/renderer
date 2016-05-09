#include "RenderView.h"


USING_ALLOCATER(RenderView);
USING_RECYCLE(RenderView);

RenderView::RenderView()
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

int RenderView::Compile() {
	int Compiled = 0;
	Compiled += Compiler->SetDepthBuffer(Depth);
	Compiled += Compiler->SetRenderTargets(TargetCount, Targets);
	Compiled += Compiler->ClearRenderTarget();
	if (ClearDepth) {
		Compiled += Compiler->ClearDepthStencil();
	}
	return 0;
}


int RenderView::QueueCommand() {
	int Stage = Type + Index;
	if (!Targets[0]) {
		// only baclbuffer need present
		Compiler->Present();
	}
	Compiler->EndBuffer();
	Queue->PushCommand(Stage, CommandBuffer);
	return 1;
}
