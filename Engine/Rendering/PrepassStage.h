#ifndef __PREPASS_STAGE__
#define __PREPASS_STAGE__

#include "RenderStage.h"
/*
	light pre-pass stage
*/

class PrepassStage : public RenderStage
{
private:
	int Targets[8];
	int ABuffers[8];
	int Depth;
	int DepthStat[8];
	int RasterStat[8];
	int BlendStat[8];
	int CompactData[2];
	Vector<RenderView*> RenderViews;
	// currant frames
	int Frames = 0;
private:
	// create g-buffer for deferred rendering
	void CreateGBuffer();
	// create a-buffer for oit
	void CreateABuffer();
	// cretae renderstat
	void CreateRenderState();
	void PrePass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	void LigthingPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	void ShadingPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);

	void OITInitPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);


	// initial stages
	int Initial();
public:
	PrepassStage(RenderContext* Context);
	virtual ~PrepassStage();
	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};


#endif