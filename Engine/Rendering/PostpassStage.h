#ifndef __POST_PASS_STAGE__
#define __POST_PASS_STAGE__

#include "RenderStage.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"


class PostpassStage : public RenderStage {

private:
	// ping pong buffers
	int PingPong[2];
	// SSAO Matrial
	Material * ssaoMaterial;
	// depthstencial
	int DepthStat[8];
private:
	void CreatePingPongBuffer();
	void InitPostSchema();
	void Initiallize();
	int SSAO(BatchCompiler * Compiler);

public:
	PostpassStage(RenderContext * Context);
	virtual ~PostpassStage();

	// execute stage
	virtual int Execute(RenderingCamera * Camera, Spatial * spatial, RenderQueue* renderQueue, WorkQueue * Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};

#endif


