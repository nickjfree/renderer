#ifndef __RENDERPROCESSER__
#define __RENDERPROCESSER__

#include "RenderContext.h"
/*
 a executor of render commands in the renderqueue
*/

class RenderProcesser;

typedef int (RenderProcesser::* rendercmd)(void * data);

typedef struct RenderCommand {
	int Opcode;
	// functions
	rendercmd cmd;
}RenderCommand;

#define MAX_RENDER_CMD 1024

class RenderProcesser
{
private:
	RenderContext * context;
	//
	RenderInterface * Interface;
	// command tables
	RenderCommand Cmds[MAX_RENDER_CMD];
	// stats just like registers
	unsigned char * ip;
private:
	int ExecuteCommand(unsigned char cmd, void * data);
protected:
	// end buffer
	int EndBuffer(void * data);
	// render geometry
	int RenderGeometry(void * data);
	// render quad
	int RenderQuad(void * data);
	// set vs
	int SetVertexShader(void * data);
	// set gs
	int SetGeometryShader(void * data);
	// set hs
	int SetHullShader(void * data);
	// set ds
	int SetDomainShader(void * data);
	// set ps
	int SetPixelShader(void * data);
	// set texture
	int SetTexture(void * data);
	// set depth
	int SetDepthStencil(void * data);
	// set rasterizer
	int SetRasterizer(void * data);
	// set blend
	int SetBlend(void * data);
	// set render target
	int SetRenderTargets(void * data);
	// set deptehbuffer
	int SetDepthBuffer(void * data);
	// set inputlayout
	int SetInputLayout(void * data);
	// set constane parameter
	int SetShaderParameter(void * data);
	// update constant
	int UpdateConstant(void * data);
	// clear rendertarget
	int ClearRenderTarget(void * data);
	// clear depth stencial
	int ClearDepthStencil(void * data);
	// set view port
	int SetViewport(void * data);
	// present
	int Present(void * data);
public:
	RenderProcesser(RenderContext * context);
	~RenderProcesser();
	// execute
	virtual int Execute(void * CommandBuffer);
};

#endif
