#include "RenderStage.h"


RenderStage::RenderStage(RenderContext* Context_) :Context(Context_)
{
	Interface = Context->GetRenderInterface();
	Initial();
}


RenderStage::~RenderStage()
{
}

int RenderStage::Initial() {
	// default stage, nothing is need
	return 0;
}


int RenderStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	return 0;
}
