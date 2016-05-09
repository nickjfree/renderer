#include "BatchCompiler.h"
#include "Material.h"
#include "Shader.h"
#include "Opcode.h"



USING_ALLOCATER(BatchCompiler);

BatchCompiler::BatchCompiler()
{
}


BatchCompiler::~BatchCompiler()
{
}

int BatchCompiler::SetBuffer(char * CommandBuffer_) {
	CommandBuffer = CommandBuffer_;
	Offset = CommandBuffer;
	return 0;
}


int BatchCompiler::RenderGeometry(int Geometry) {
	*Offset++ = OP_RENDER_GEO;
	*(int *)Offset = Geometry;
	Offset += sizeof(int);
	return sizeof(char) + sizeof(int);
}


int BatchCompiler::SetTransform(Matrix4x4& Transform) {
	return sizeof(char)+sizeof(Matrix4x4);
}

int BatchCompiler::SetVertexShader(int Shader) {
	*Offset++ = OP_SHADER_VS;
	*(int *)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}
// set gs
int BatchCompiler::SetGeometryShader(int Shader) {
	*Offset++ = OP_SHADER_GS;
	*(int *)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}
// set hs
int BatchCompiler::SetHullShader(int Shader){
	*Offset++ = OP_SHADER_HS;
	*(int *)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}
// set ds
int BatchCompiler::SetDomainShader(int Shader){
	*Offset++ = OP_SHADER_DS;
	*(int *)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}
// set ps
int BatchCompiler::SetPixelShader(int Shader) {
	*Offset++ = OP_SHADER_PS;
	*(int *)Offset = Shader;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}

int BatchCompiler::SetTexture(int Slot, int Texture) {
	*Offset++ = OP_TEXTURE;
	*(unsigned char *)Offset++ = Slot;
	*(int *)Offset = Texture;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(char)+sizeof(int);
}

int BatchCompiler::EndBuffer() {
	*Offset++ = OP_END_EXECUTE;
	return sizeof(char);
}

int BatchCompiler::SetDepthStencil(int Id) {
	*Offset++ = OP_DEPTH_STAT;
	*(int *)Offset = Id;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}

int BatchCompiler::SetRasterizer(int Id) {
	*Offset++ = OP_RASTER_STAT;
	*(int *)Offset = Id;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}

int BatchCompiler::SetBlend(int Id) {
	*Offset++ = OP_BLEND_STAT;
	*(int *)Offset = Id;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}

int BatchCompiler::SetRenderTargets(int Count, int * Targets) {
	*Offset++ = OP_RENDER_TARGET;
	int Total = Count;
	int Size = 0;
	*(unsigned char *)Offset++ = Count;
	while (Count--) {
		*(int*)Offset = Targets[Total - Count - 1];
		Size += sizeof(int);
		Offset += sizeof(int);
	}
	return sizeof(char)+sizeof(char)+Size;
}

int BatchCompiler::SetDepthBuffer(int Id) {
	*Offset++ = OP_DEPTH_STENCIL;
	*(int *)Offset = Id;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}


int BatchCompiler::SetInputLayout(int Id) {
	*Offset++ = OP_INPUT_LAYOUT;
	*(int *)Offset = Id;
	Offset += sizeof(int);
	return sizeof(char)+sizeof(int);
}


int BatchCompiler::SetShaderParameter(int Slot, int offset, int Size, void * Data) {
	*Offset++ = OP_SET_CONSTANT;
	*(unsigned char *)Offset++ = Slot;
	*(int *)Offset = offset;
	Offset += sizeof(int);
	*(int *)Offset = Size;
	Offset += sizeof(int);
	memcpy(Offset, Data, Size);
	Offset += Size;
	return sizeof(char)+sizeof(char)+sizeof(int)+sizeof(int)+Size;
}

int BatchCompiler::UpdateConstant(int Slot) {
	*Offset++ = OP_UP_CONSTANT;
	*(unsigned char *)Offset++ = Slot;
	return sizeof(char)+sizeof(char);
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