#ifndef __POST_PASS_STAGE__
#define __POST_PASS_STAGE__

#include "RenderStage.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"

#define MAX_HDR_LUM 8

class PostpassStage : public RenderStage {

private:
	// ping pong buffers
	int PingPong[2];
	// SSAO Matrial
	Material * ssaoMaterial;
	// depthstencial
	int DepthStat[8];
	// Parameters fot post stage
	Dict Parameter;
	// HDR buffers and parameters
	float ScaleOffset[MAX_HDR_LUM][16];
	float BrightOffset[16];
	float BloomOffset[2][16];
	float BloomWeight[16];
	int AvgIter;
	int LumBufferWidth;
	int LumBufferHeight;
	int LumScaleArray[MAX_HDR_LUM];
	int AdaptLum[2];
	int Bright;
	int Bloom[3];
	int Star;
	// current time
	unsigned int Time;
	// frame num
	int Frames;
	Shader * HDRShader;
    Shader * OITShader;
private:
	void CreatePingPongBuffer();
	void CreateHDRBuffer();
	void InitSampleOffset();
	void InitPostSchema();
	void Initiallize();
	// frame delta
	unsigned int GetFrameDelta();
	// pingpong
	void SwapPingPong();

	// SSAO
	int SSAO(BatchCompiler * Compiler);
    // OIT
    int OIT(BatchCompiler * Compiler);
	// HDR
	int ScaleBy4(BatchCompiler * Compiler);
	int CalcAvgLum(BatchCompiler * Compiler);
	int CalcAdaptLum(BatchCompiler * Compiler);
	int BrightPass(BatchCompiler * Compiler);
	int BloomPass(BatchCompiler * COmpiler);
	int ToneMapping(BatchCompiler * Compiler);
	int HDR(BatchCompiler * Compiler);
public:
	PostpassStage(RenderContext * Context);
	virtual ~PostpassStage();

	// execute stage
	virtual int Execute(RenderingCamera * Camera, Spatial * spatial, RenderQueue* renderQueue, WorkQueue * Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};

#endif


