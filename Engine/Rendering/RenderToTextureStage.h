#ifndef __RTT_STAGE__
#define __RTT_STAGE__

#include "RenderStage.h"


class RenderToTextureStage : public RenderStage {


private:
	// terrain virtual texture
	int terrainVirtualTexture_;
	// terrain page table
	int terrainPageTable_;

private:
	// initialization
	void Initialize();
	// create textures for terrain rendering
	void CreateTerrainTextures();
	// terrain virtual texture pass
	int TerrainVTPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);

public:
	RenderToTextureStage(RenderContext* context);
	virtual ~RenderToTextureStage();
};

#endif
