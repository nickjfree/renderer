#ifndef __RENDERINTERFACE_H__
#define __RENDERINTERFACE_H__


#include "RenderDesc.h"
#include "windows.h"


/*
	renderer command context
*/

class RenderCommandContext
{
public:
	// set to compute mode
	virtual void SetComputeMode(bool enabled) = 0;
	// set shader resource
	virtual void SetSRV(int slot, int resourceId) = 0;
	// set uav
	virtual void SetUAV(int slot, int resourceId) = 0;
	// set render targets
	virtual void SetRenderTargets(int* targets, int numTargets, int depth) = 0;
	// set constant buffer
	virtual void UpdateConstantBuffer(int slot, unsigned int offset, void* buffer, unsigned int size) = 0;
	// update constant
	virtual void SetConstantBuffer(int slot, unsigned int size) = 0;
	// set renderstate
	virtual void SetRasterizer(int id) = 0;
	// set blend state
	virtual void SetBlendState(int id) = 0;
	// set depthstencilstate
	virtual void SetDepthStencilState(int id) = 0;
	// set viewport
	virtual void SetViewPort(int x, int y, int w, int h) = 0;
	// set vertext shader
	virtual void SetVertexShader(int id) = 0;
	// set pixel shader
	virtual void SetPixelShader(int id) = 0;
	// draw single geometry
	virtual void Draw(int geometryId) = 0;
	// draw instance
	virtual void DrawInstanced(int geometryId, void* instanceBuffer, unsigned int stride, unsigned int numInstances) = 0;
	// draw full screen quad
	virtual void Quad() = 0;
	// dispatch rays
	virtual void DispatchRays(int shaderId, int width, int height) = 0;
	// dispatch
	virtual void DispatchCompute(int width, int height) = 0;
};


/*
	Renderer interface. 
*/

class RenderInterface
{

private:
	int Debug(char* info);
public:
	RenderInterface();
	virtual ~RenderInterface();

	// init 
	virtual int Initialize(int Widthm, int Height) { return 0; }
	/* Resource Creation  */

	// create texture2d. with or without initial data.
	virtual int CreateTexture2D(R_TEXTURE2D_DESC* Desc, void* RawData, int Size, int DataFlag) { return -1; };

	// destroy texture 2D
	virtual int DestroyTexture2D(int Id) { return -1; };

	// create geometry. with raw vertext and index datas. the buffer pool is set to dynamic by default
	virtual int CreateGeometry(void* VBuffer, unsigned int VBSize, unsigned int VertexSize, void* IBuffer, unsigned int IBSize, R_FORMAT IndexFormat, R_PRIMITIVE_TOPOLOGY Top) {
		Debug("creategeometry\n");
		return -1;
	}

	// create geometry. with raw vertext and index datas. the buffer pool is set to dynamic by default
	virtual int CreateGeometry(R_GEOMETRY_DESC* desc) {
		Debug("creategeometry\n");
		return -1;
	}

	// create bottom level as
	virtual int CreateRaytracingGeometry(int GeometryId, bool Deformable, int* BufferId) { return -1; };
	
	// add raytracing instance
	virtual int AddRaytracingInstance(R_RAYTRACING_INSTANCE& instance) { return -1; };

	// destroy geometry data. free gpu resource
	virtual int DestroyGeometry(int Id) { return 0; };

	// destroy bottom level as. free gpu resource
	virtual int DestroyBottomLevelAS(int Id) { return 0; };

	// create constant buffer
	virtual int CreateConstantBuffer(unsigned int Size) { return -1; }
	// create just a buffer. may be used for ComputeShader and CUDA
	virtual int CreateBuffer(R_BUFFER_DESC* desc) { return -1; }

	// Destroy buffer
	virtual int DestroyBuffer(int Id) { return -1; }

	// create input layout
	virtual int CreateInputLayout(R_INPUT_ELEMENT* Element, int Count, void* ShaderCode, int Size) { return -1; };
	// create shaders 
	virtual int CreateVertexShader(void* ByteCode, unsigned int Size, int flag) { return -1; };
	virtual int CreateGeometryShader(void* ByteCode, unsigned int Size, int flag) { return -1; };
	virtual int CreateHullShader(void* ByteCode, unsigned int Size, int flag) { return -1; };
	virtual int CreateDomainShader(void* ByteCode, unsigned int Size, int flag) { return -1; };
	virtual int CreatePixelShader(void* ByteCode, unsigned int Size, int flag) { return -1; };
	virtual int CreateComputeShader(void* ByteCode, unsigned int Size, int flag) { return -1; };

	/*  Raytracing */
	// create libarary closest/any/intersect
	virtual int CreateRayTracingShader(void* ByteCode, unsigned int Size,
		const wchar_t* Raygen, const wchar_t* Miss,
		const wchar_t* HitGroup,
		const wchar_t* ClosestHit, const wchar_t* AnyHit, const wchar_t* Intersection) {
		return -1;
	};

	// Depth stencil status
	virtual int CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc) { return -1; }
	// Blend Status
	virtual int CreateBlendStatus(R_BLEND_STATUS* Desc) { return -1; }
	//Rasterizer status
	virtual int CreateRasterizerStatus(R_RASTERIZER_DESC* Desc) { return -1; }
	// create render target
	virtual int CreateRenderTarget(R_TEXTURE2D_DESC* Desc) { return -1; }
	// create depth stencil also used as shadow maps
	virtual int CreateDepthStencil(R_TEXTURE2D_DESC* Desc) { return  -1; }

	/*   Context Setup  */
		// set view port
	virtual void SetViewPort(float tlx, float tly, float width, float height, float minz, float maxz) {};

	virtual void SetUnorderedAccessTexture(int StartSlot, int* Textures, int Count) {};
	// set buffer as uav
	virtual void SetUnorderedAccessBuffer(int StartSlot, int* Buffers, int Count) {};
	// set texture as srv
	virtual void SetTexture(int StartSlot, int* Texture, int Count) {};
	// set buffer as srv
	virtual void SetBuffer(int StartSlot, int* Buffers, int Count) {};
	// set top level as
	virtual void SetTopLevelAS(int Slot) {};
	// Set Blend 
	virtual void SetBlendStatus(int Blend) {}
	// depth and stencil
	virtual void SetDepthStencilStatus(int DepthStencil) {}
	// set rendertarget
	virtual void SetRenderTargets(int Count, int* Targets) {}
	// set rendertarget
	virtual void SetDepthStencil(int Depth) {}
	// rasterizer
	virtual void SetRasterizerStatus(int Rasterizer) {}
	// set vertex shader
	virtual void SetVertexShader(int Shader) {}
	// set geometry shader
	virtual void SetGeometryShader(int Shader) {}
	// set hull shader
	virtual void SetHullShader(int Shader) {}
	// set domain shader
	virtual void SetDomainShader(int Shader) {}
	// set pixel shader
	virtual void SetPixelShader(int Shader) {}
	// set compute shader
	virtual void SetComputeShader(int Shader) {}
	// set constant buffer
	virtual void SetConstant(int Slot, int Buffer, void* CPUData, unsigned int Size) {};
	// set inputylayout
	virtual void SetInputLayout(int Id) {};

	/* Draw */
		// draw single geometry
	virtual void Draw(int Geometry) {};
	// draw instance geometry
	virtual void DrawInstance(int Geometry, void* InstanceBuffer, unsigned int BufferSize, unsigned int InstanceNum) {};
	// Draw raw data. Slow operation, cause the data must copy to GPU memory first
	virtual void DrawRaw(void* VBuffer, unsigned int VBSize, unsigned int VertexSize, void* IBuffer, unsigned int IBSize, R_FORMAT IndexFormat) {}
	// draw full screen quad
	virtual void Quad() {};
	// draw rect quad, not that usefull
	virtual void Rect() {};
	// trace ray
	virtual void TraceRay() {};
	// build raytracing scene
	virtual void BuildRaytracingScene() {};
	// clear depth
	virtual void ClearDepth(float depth, float stencil) {};
	// clear target
	virtual void ClearRenderTarget() {};
	// present
	virtual void Present() {};



	/*  Query  */
		// setup query
	virtual int IssueQuery(R_QUERY query, void* Result, unsigned int Len) { return 0; }
	// end query
	virtual void EndQuery() {}

	/* Profile */

	// begin event
	virtual int BeginEvent(UINT64 color, const char* message) { return 0; }
	// end event
	virtual int EndEvent() { return 0; }

	/* rendercommandcontext	*/
	virtual RenderCommandContext* BeginContext(bool asyncCompute) { return nullptr; }

	virtual UINT64 EndContext(RenderCommandContext* cmdContext, bool present) { return 0; }
};

#endif
