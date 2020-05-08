#include "BatchCompiler.h"
#include "Material.h"
#include "Shader.h"
#include "Opcode.h"



USING_ALLOCATER(BatchCompiler);

BatchCompiler::BatchCompiler()
{
	Instancing = 0;
	PrevGeometry = -1;
	PrevVS = -1;
	PrevPS = -1;
	PrevGS = -1;
	PrevDS = -1;
	PrevBlend = -1;
	PrevDepthStencil = -1;
	PrevRasterizer = -1;
	PrevLayout = -1;
	for (int i = 0; i < 32; i++) {
		PreTextures[i] = -1;
		PreUAVs[i] = -1;
		PreShaderBuffers[i] = -1;
		PreUAVBuffers[i] = -1;
	}
}


BatchCompiler::~BatchCompiler()
{
}

int BatchCompiler::SetBuffer(char* CommandBuffer_) {
	CommandBuffer = CommandBuffer_;
	Offset = CommandBuffer;
	return 0;
}


int BatchCompiler::RenderGeometry(int Geometry) {
	*Offset++ = OP_RENDER_GEO;
	*(int*)Offset = Geometry;
	Offset += sizeof(int);
	Instancing = 0;
	return sizeof(char) + sizeof(int);
}


int BatchCompiler::SetTransform(Matrix4x4& Transform) {
	return sizeof(char) + sizeof(Matrix4x4);
}

int BatchCompiler::SetVertexShader(int Shader) {
	*Offset++ = OP_SHADER_VS;
	*(int*)Offset = Shader;
	Offset += sizeof(int);
	if (Shader != PrevVS) {
		Instancing = 0;
	}
	PrevVS = Shader;
	return sizeof(char) + sizeof(int);
}
// set gs
int BatchCompiler::SetGeometryShader(int Shader) {
	*Offset++ = OP_SHADER_GS;
	*(int*)Offset = Shader;
	Offset += sizeof(int);
	if (Shader != PrevGS) {
		Instancing = 0;
	}
	PrevGS = Shader;
	return sizeof(char) + sizeof(int);
}
// set hs
int BatchCompiler::SetHullShader(int Shader) {
	*Offset++ = OP_SHADER_HS;
	*(int*)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char) + sizeof(int);
}
// set ds
int BatchCompiler::SetDomainShader(int Shader) {
	*Offset++ = OP_SHADER_DS;
	*(int*)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char) + sizeof(int);
}
// set ps
int BatchCompiler::SetPixelShader(int Shader) {
	*Offset++ = OP_SHADER_PS;
	*(int*)Offset = Shader;
	Offset += sizeof(int);
	if (Shader != PrevPS) {
		Instancing = 0;
	}
	PrevPS = Shader;
	return sizeof(char) + sizeof(int);
}

int BatchCompiler::SetTexture(int Slot, int Texture) {
	if (Texture == 0) {
		printf("fuck\n");
	}
	*Offset++ = OP_TEXTURE;
	*(unsigned char*)Offset++ = Slot;
	*(int*)Offset = Texture;
	Offset += sizeof(int);
	if (PreTextures[Slot] != Texture) {
		Instancing = 0;
	}
	PreTextures[Slot] = Texture;
	return sizeof(char) + sizeof(char) + sizeof(int);
}

int BatchCompiler::SetShaderResourceBuffer(int Slot, int Buffer) {
	if (Buffer == 0) {
		printf("fuck\n");
	}
	*Offset++ = OP_BUFFER;
	*(unsigned char*)Offset++ = Slot;
	*(int*)Offset = Buffer;
	Offset += sizeof(int);
	if (PreShaderBuffers[Slot] != Buffer) {
		Instancing = 0;
	}
	PreShaderBuffers[Slot] = Buffer;
	return sizeof(char) + sizeof(char) + sizeof(int);
}

// set UnorderedAccessBuffer
int BatchCompiler::SetUnordedAccessBuffer(int Slot, int Buffer) {
	if (Buffer == 0) {
		printf("fuck\n");
	}
	*Offset++ = OP_UAVBUF;
	*(unsigned char*)Offset++ = Slot;
	*(int*)Offset = Buffer;
	Offset += sizeof(int);
	if (PreUAVBuffers[Slot] != Buffer) {
		Instancing = 0;
	}
	PreUAVBuffers[Slot] = Buffer;
	return sizeof(char) + sizeof(char) + sizeof(int);
}

// set UnorderedAccessTexture
int BatchCompiler::SetUnordedAccessTexture(int Slot, int Texture) {
	if (Texture == 0) {
		printf("fuck\n");
	}
	*Offset++ = OP_UAVTEX;
	*(unsigned char*)Offset++ = Slot;
	*(int*)Offset = Texture;
	Offset += sizeof(int);
	if (PreUAVs[Slot] != Texture) {
		Instancing = 0;
	}
	PreUAVs[Slot] = Texture;
	return sizeof(char) + sizeof(char) + sizeof(int);
}

int BatchCompiler::EndBuffer() {
	*Offset++ = OP_END_EXECUTE;
	Instancing = 0;
	PrevGeometry = -1;
	PrevVS = -1;
	PrevPS = -1;
	PrevGS = -1;
	PrevDS = -1;
	PrevBlend = -1;
	PrevDepthStencil = -1;
	PrevRasterizer = -1;
	for (int i = 0; i < 32; i++) {
		PreTextures[i] = -1;
	}
	return sizeof(char);
}

int BatchCompiler::SetDepthStencil(int Id) {
	*Offset++ = OP_DEPTH_STAT;
	*(int*)Offset = Id;
	Offset += sizeof(int);
	if (Id != PrevDepthStencil) {
		Instancing = 0;
	}
	PrevDepthStencil = Id;
	return sizeof(char) + sizeof(int);
}

int BatchCompiler::SetRasterizer(int Id) {
	*Offset++ = OP_RASTER_STAT;
	*(int*)Offset = Id;
	Offset += sizeof(int);
	if (Id != PrevRasterizer) {
		Instancing = 0;
	}
	PrevRasterizer = Id;
	return sizeof(char) + sizeof(int);
}

int BatchCompiler::SetBlend(int Id) {
	*Offset++ = OP_BLEND_STAT;
	*(int*)Offset = Id;
	Offset += sizeof(int);
	if (Id != PrevBlend) {
		Instancing = 0;
	}
	PrevBlend = Id;
	return sizeof(char) + sizeof(int);
}

int BatchCompiler::SetRenderTargets(int Count, int* Targets) {
	*Offset++ = OP_RENDER_TARGET;
	int Total = Count;
	int Size = 0;
	*(unsigned char*)Offset++ = Count;
	while (Count--) {
		*(int*)Offset = Targets[Total - Count - 1];
		Size += sizeof(int);
		Offset += sizeof(int);
	}
	Instancing = 0;
	return sizeof(char) + sizeof(char) + Size;
}

int BatchCompiler::SetDepthBuffer(int Id) {
	*Offset++ = OP_DEPTH_STENCIL;
	*(int*)Offset = Id;
	Offset += sizeof(int);
	Instancing = 0;
	return sizeof(char) + sizeof(int);
}


int BatchCompiler::SetInputLayout(int Id) {
	*Offset++ = OP_INPUT_LAYOUT;
	*(int*)Offset = Id;
	Offset += sizeof(int);
	if (Id != PrevLayout) {
		Instancing = 0;
	}
	PrevLayout = Id;
	return sizeof(char) + sizeof(int);
}


int BatchCompiler::SetShaderParameter(int Slot, int offset, int Size, void* Data) {
	*Offset++ = OP_SET_CONSTANT;
	*(unsigned char*)Offset++ = Slot;
	*(int*)Offset = offset;
	Offset += sizeof(int);
	*(int*)Offset = Size;
	Offset += sizeof(int);
	memcpy(Offset, Data, Size);
	Offset += Size;
	return sizeof(char) + sizeof(char) + sizeof(int) + sizeof(int) + Size;
}


int BatchCompiler::SetViewport(float tlx, float tly, float width, float height, float minz, float maxz) {
	*Offset++ = OP_VIEWPORT;
	*(float*)Offset = tlx;
	Offset += sizeof(float);
	*(float*)Offset = tly;
	Offset += sizeof(float);
	*(float*)Offset = width;
	Offset += sizeof(float);
	*(float*)Offset = height;
	Offset += sizeof(float);
	*(float*)Offset = minz;
	Offset += sizeof(float);
	*(float*)Offset = maxz;
	Offset += sizeof(float);
	return sizeof(char) + sizeof(float) * 6;
}

int BatchCompiler::UpdateConstant(int Slot) {
	*Offset++ = OP_UP_CONSTANT;
	*(unsigned char*)Offset++ = Slot;
	return sizeof(char) + sizeof(char);
}

int BatchCompiler::UpdateArray(int Slot, int Size, void* Data) {
	*Offset++ = OP_UP_ARRAY;
	*(unsigned char*)Offset++ = Slot;
	*(unsigned int*)Offset = Size;
	Offset += sizeof(int);
	*(void**)Offset = Data;
	Offset += sizeof(void*);
	return sizeof(char) + sizeof(char) + sizeof(int) + sizeof(void*);
}


int  BatchCompiler::ClearRenderTarget() {
	*Offset++ = OP_CLEAR_TARGET;
	return sizeof(char);
}

int  BatchCompiler::ClearDepthStencil() {
	*Offset++ = OP_CLEAR_DEPTH;
	return sizeof(char);
}

int  BatchCompiler::Present() {
	*Offset++ = OP_PRESENT;
	return sizeof(char);
}

int BatchCompiler::Quad() {
	*Offset++ = OP_QUAD;
	return sizeof(char);
}

int BatchCompiler::TraceRay() {
	*Offset++ = OP_TRACERAY;
	return sizeof(char);
}

int BatchCompiler::BuildRaytracingScene() {
	*Offset++ = OP_BUILD_RTSCENE;
	return sizeof(char);
}

int BatchCompiler::Instance(int Geometry, void* InstanceData, int Size) {
	if (!Instancing) {
		Instancing = 1;
		// restart instancing opcode 
		*Offset++ = OP_INSTANCE;
		*(unsigned int*)Offset = Geometry;
		Offset += sizeof(unsigned int);
		*(unsigned int*)Offset = Size;
		Offset += sizeof(unsigned int);
		PrevInstanceStart = Offset;
		*(unsigned int*)Offset = 1;
		Offset += sizeof(unsigned int);
		memcpy(Offset, InstanceData, Size);
		Offset += Size;
		PrevInstanceEnd = Offset;

	}
	else {
		// inc size and append instance buffer from last instancing position
		unsigned int PrevCount = *(unsigned int*)PrevInstanceStart;
		*(unsigned int*)PrevInstanceStart = PrevCount + 1;
		Offset = PrevInstanceEnd;
		memcpy(Offset, InstanceData, Size);
		Offset += Size;
		PrevInstanceEnd = Offset;
	}
	return sizeof(char) + sizeof(int) + Size;
}


size_t BatchCompiler::GetCommandSize() {
	return this->Offset - CommandBuffer;
}