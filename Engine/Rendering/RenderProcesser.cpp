#include "RenderProcesser.h"
#include "RenderingSystem.h"
#include "Opcode.h"
#include "stdio.h"

#define MAX_INSTANCE_NUM 1024

RenderProcesser::RenderProcesser(RenderContext* context_) :context(context_)
{
	Interface = context->GetRenderInterface();
	// set up command table
	Cmds[OP_RENDER_TARGET].cmd = &RenderProcesser::SetRenderTargets;
	Cmds[OP_DEPTH_STENCIL].cmd = &RenderProcesser::SetDepthBuffer;
	Cmds[OP_SHADER_VS].cmd = &RenderProcesser::SetVertexShader;
	Cmds[OP_SHADER_GS].cmd = &RenderProcesser::SetPixelShader;
	Cmds[OP_SHADER_HS].cmd = &RenderProcesser::SetHullShader;
	Cmds[OP_SHADER_DS].cmd = &RenderProcesser::SetDomainShader;
	Cmds[OP_SHADER_PS].cmd = &RenderProcesser::SetPixelShader;
	Cmds[OP_SET_CONSTANT].cmd = &RenderProcesser::SetShaderParameter;
	Cmds[OP_UP_CONSTANT].cmd = &RenderProcesser::UpdateConstant;
	Cmds[OP_UP_ARRAY].cmd = &RenderProcesser::UpdateArray;
	Cmds[OP_DEPTH_STAT].cmd = &RenderProcesser::SetDepthStencil;
	Cmds[OP_RASTER_STAT].cmd = &RenderProcesser::SetRasterizer;
	Cmds[OP_BLEND_STAT].cmd = &RenderProcesser::SetBlend;
	Cmds[OP_TEXTURE].cmd = &RenderProcesser::SetTexture;
	Cmds[OP_BUFFER].cmd = &RenderProcesser::SetShaderResourceBuffer;
	Cmds[OP_UAVBUF].cmd = &RenderProcesser::SetUnordedAccessBuffer;
	Cmds[OP_UAVTEX].cmd = &RenderProcesser::SetUnordedAccessTexture;
	Cmds[OP_RENDER_GEO].cmd = &RenderProcesser::RenderGeometry;
	Cmds[OP_END_EXECUTE].cmd = &RenderProcesser::EndBuffer;
	Cmds[OP_INPUT_LAYOUT].cmd = &RenderProcesser::SetInputLayout;
	Cmds[OP_CLEAR_DEPTH].cmd = &RenderProcesser::ClearDepthStencil;
	Cmds[OP_CLEAR_TARGET].cmd = &RenderProcesser::ClearRenderTarget;
	Cmds[OP_VIEWPORT].cmd = &RenderProcesser::SetViewport;
	Cmds[OP_PRESENT].cmd = &RenderProcesser::Present;
	Cmds[OP_QUAD].cmd = &RenderProcesser::RenderQuad;
	Cmds[OP_INSTANCE].cmd = &RenderProcesser::Instance;
	Cmds[OP_BUILD_RTSCENE].cmd = &RenderProcesser::BuildRaytracingScene;
	Cmds[OP_TRACERAY].cmd = &RenderProcesser::TraceRay;
}


RenderProcesser::~RenderProcesser()
{
}

int RenderProcesser::ExecuteCommand(unsigned char cmd, void* data) {
	if (cmd < 0 || cmd > 256) {
		printf("cmd buffer corupt\n");
		return 0;
	}
	rendercmd cmd_func = Cmds[cmd].cmd;
	return (this->*cmd_func)(data);
}

int RenderProcesser::Execute(void* CommandBuffer) {
	// do the real render stuff, by use rendernterface
	/*for (int i = 0; i < 1000; i++) {
		printf("%02x ", ((unsigned char*)CommandBuffer)[i]);
	}
	printf("\n");*/
	ip = (unsigned char*)CommandBuffer;
	char cmd = 0;
	// begin rendering
	void* Data = 0;
	int flag = 1;
	while (flag) {
		cmd = *(unsigned char*)ip;
		Data = ip + 1;
		flag = ExecuteCommand(cmd, Data);
	}
	return 0;
}


int RenderProcesser::EndBuffer(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip++;
	return 0;
}

int RenderProcesser::RenderGeometry(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;

	// render
	int Geometry = *(int*)data;
	Interface->Draw(Geometry);
	return 1;
}

int RenderProcesser::Instance(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + sizeof(int) + sizeof(int) + 1;
	// render
	char* p = (char*)data;
	int Geometry = *(int*)p;
	p += sizeof(unsigned int);
	int Size = *(unsigned int*)p;
	p += sizeof(unsigned int);
	int Count = *(unsigned int*)p;
	p += sizeof(unsigned int);
	void* InstanceData = (void*)p;
	p += (Size * Count);
	ip += (Size * Count);
	// split by MAX_INSTANCE_NUM instance per draw call
	while (Count > 0) {
		int Num = MAX_INSTANCE_NUM;
		if (Count < MAX_INSTANCE_NUM) {
			Num = Count;
		}
		Interface->DrawInstance(Geometry, InstanceData, Size, Num);
		InstanceData = (char*)InstanceData + MAX_INSTANCE_NUM * Size;
		Count -= MAX_INSTANCE_NUM;
	}
	return 1;
}

int RenderProcesser::RenderQuad(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip++;
	// render
	Interface->Quad();
	return 1;
}

int RenderProcesser::BuildRaytracingScene(void* data) {
	ip++;
	Interface->BuildRaytracingScene();
	return 1;
}

// traceray
int RenderProcesser::TraceRay(void* data) {
	ip++;
	Interface->TraceRay();
	return 1;
}

int RenderProcesser::SetVertexShader(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetVertexShader(Id);
	return 1;
}

int RenderProcesser::SetGeometryShader(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	return 1;
}

int RenderProcesser::SetHullShader(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	return 1;
}

int RenderProcesser::SetDomainShader(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	return 1;
}

int RenderProcesser::SetPixelShader(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetPixelShader(Id);
	return 1;
}

int RenderProcesser::SetTexture(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 2;
	unsigned char* p = (unsigned char*)data;
	int slot = *p;
	p += 1;
	int Id = *(int*)p;
	Interface->SetTexture(slot, &Id, 1);
	return 1;
}
int RenderProcesser::SetShaderResourceBuffer(void* data) {
	ip += sizeof(int) + 2;
	unsigned char* p = (unsigned char*)data;
	int slot = *p;
	p += 1;
	int Id = *(int*)p;
	Interface->SetBuffer(slot, &Id, 1);
	return 1;
}
// set UnorderedAccessBuffer
int RenderProcesser::SetUnordedAccessBuffer(void* data) {
	ip += sizeof(int) + 2;
	unsigned char* p = (unsigned char*)data;
	int slot = *p;
	p += 1;
	int Id = *(int*)p;
	Interface->SetUnorderedAccessBuffer(slot, &Id, 1);
	return 1;
}

// set UnorderedAccessTexture
int RenderProcesser::SetUnordedAccessTexture(void* data) {
	ip += sizeof(int) + 2;
	unsigned char* p = (unsigned char*)data;
	int slot = *p;
	p += 1;
	int Id = *(int*)p;
	Interface->SetUnorderedAccessTexture(slot, &Id, 1);
	return 1;
}

int RenderProcesser::SetDepthStencil(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetDepthStencilStatus(Id);
	return 1;
}

int RenderProcesser::SetRasterizer(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetRasterizerStatus(Id);
	return 1;
}

int RenderProcesser::SetBlend(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetBlendStatus(Id);
	return 1;
}

int RenderProcesser::SetRenderTargets(void* data) {
	//printf("%s\n", __FUNCTION__);
	int count = *(char*)data;
	int* targets = (int*)((char*)data + 1);
	ip += sizeof(int) * count + 2;
	Interface->SetRenderTargets(count, targets);
	return 1;
}

int RenderProcesser::SetDepthBuffer(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetDepthStencil(Id);
	return 1;
}

int RenderProcesser::SetInputLayout(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += sizeof(int) + 1;
	int Id = *(int*)data;
	Interface->SetInputLayout(Id);
	return 1;
}

int RenderProcesser::SetShaderParameter(void* data) {
	//printf("%s\n", __FUNCTION__);
	char* p = (char*)data;
	char slot = *(char*)p;
	p += 1;
	unsigned int offset = *(int*)p;
	p += sizeof(int);
	unsigned int size = *(int*)p;
	p += sizeof(int);
	void* parameter = p;
	ip += (p - (char*)data) + size + 1;
	context->SetParameter(slot, parameter, offset, size);
	return 1;
}

int RenderProcesser::UpdateConstant(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += 2;
	int Slot = *(unsigned char*)data;
	context->UpdateConstant(Slot);
	return 1;
}

int RenderProcesser::UpdateArray(void* data) {
	char* p = (char*)data;
	char slot = *(char*)p;
	p += 1;
	unsigned int size = *(int*)p;
	p += sizeof(int);
	void* parameter = *(void**)p;
	ip += 1 + 1 + sizeof(int) + sizeof(void*);
	//context->SetParameter(slot, parameter, offset, size);
	Interface->SetConstant(slot, 0, parameter, size);
	return 1;
}

int RenderProcesser::ClearRenderTarget(void* data) {
	//printf("%s\n", __FUNCTION__);
	Interface->ClearRenderTarget();
	ip += 1;
	return 1;
}

int RenderProcesser::ClearDepthStencil(void* data) {
	//printf("%s\n", __FUNCTION__);
	Interface->ClearDepth(1, 0);
	ip += 1;
	return 1;
}

int RenderProcesser::SetViewport(void* data) {
	ip += 1;
	float* p = (float*)data;
	ip += sizeof(float) * 6;
	Interface->SetViewPort(p[0], p[1], p[2], p[3], p[4], p[5]);
	return 1;
}

int RenderProcesser::Present(void* data) {
	//printf("%s\n", __FUNCTION__);
	ip += 1;
	Interface->Present();
	context->EndFrame();
	return 1;
}
