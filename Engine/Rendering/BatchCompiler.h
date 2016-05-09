#ifndef __BATCH_COMPILTER__
#define __BATCH_COMPILTER__


#include "Core\Allocater.h"
#include "Container\RecyclePool.h"
#include "Core\Shortcuts.h"
#include "RenderContext.h"
#include "Math\LinearMath.h"
//#include "RenderObject.h"
//#include "RenderingSystem.h"

/*
	compilter, compile render batchs to byte code whitch excuted by renderprocesser
*/
# define BYTECODE_SIZE 2048

class BatchCompiler
{
	DECLAR_ALLOCATER(BatchCompiler);
private:
	char ByteCode[BYTECODE_SIZE];
	// curretn status
	int PrevGeometry;
	// buffer
	char * CommandBuffer;
	// buffer pointer
	char * Offset;
public:
	BatchCompiler();
	~BatchCompiler();
	// end buffer
	int EndBuffer();
	// set buffer
	int SetBuffer(char * CommandBuffer_);
	// render geometry
	int RenderGeometry(int Geometry);
	// set transform
	int SetTransform(Matrix4x4& Transform);
	// set vs
	int SetVertexShader(int Shader);
	// set gs
	int SetGeometryShader(int Shader);
	// set hs
	int SetHullShader(int Shader);
	// set ds
	int SetDomainShader(int Shader);
	// set ps
	int SetPixelShader(int Shader);
	// set texture
	int SetTexture(int Slot, int Texture);
	// set depth
	int SetDepthStencil(int Id);
	// set rasterizer
	int SetRasterizer(int Id);
	// set blend
	int SetBlend(int Id);
	// set render target
	int SetRenderTargets(int count, int * Targets);
	// set deptehbuffer
	int SetDepthBuffer(int Id);
	// set inputlayout
	int SetInputLayout(int Id);
	// set constane parameter
	int SetShaderParameter(int Slot, int Offset, int Size, void * Data);
	// update constant
	int UpdateConstant(int Slot);
	// clear rendertarget
	int ClearRenderTarget();
	// clear depth stencial
	int ClearDepthStencil();
	// present
	int Present();
	// quad
	int Quad();
};


#endif
