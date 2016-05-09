#include "PostpassStage.h"



PostpassStage::PostpassStage(RenderContext * Context): RenderStage(Context), ssaoMaterial(0) {
	Initiallize();
}


PostpassStage::~PostpassStage() {
}

void PostpassStage::InitPostSchema() {
	R_DEPTH_STENCIL_DESC depth = {};
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
	depth.StencilEnable = 0;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::KEEP;
	depth.StencilFuncFront = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::KEEP;
	depth.StencilFuncBack = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilRef = 0;
	DepthStat[0] = Interface->CreateDepthStencilStatus(&depth);
	Context->RegisterRenderState(String("NoZTest"), DepthStat[0]);
}

void PostpassStage::CreatePingPongBuffer() {
	R_TEXTURE2D_DESC desc = {};
	desc.Width = 1920;
	desc.Height = 1080;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	PingPong[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.Format = FORMAT_R8G8B8A8_UNORM;
	PingPong[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);

	//// resgister targets
	Context->RegisterRenderTarget(String("gPostBuffer"), PingPong[0]);
	Context->RegisterRenderTarget(String("gFinalBuffer"), PingPong[1]);

	Variant ScreenSize;
	ScreenSize.as<Vector2>() = Vector2(1920, 1080);
	Context->SetResource(String("gScreenSize"), ScreenSize);
}

void PostpassStage::Initiallize() {
	CreatePingPongBuffer();
	InitPostSchema();
}

int PostpassStage::SSAO(BatchCompiler * Compiler) {
	int Compiled = 0;
	if (ssaoMaterial) {
		Shader * shader = ssaoMaterial->GetShader();
		Compiled += shader->Compile(Compiler, 0, 0, ssaoMaterial->GetParameter(), ssaoMaterial->GetParameter(), Context);
		// draw full screen quad
		Compiled += Compiler->Quad();
	}
	else {
		Variant * Value = Context->GetResource(String("Material\\Materials\\ssao.xml\\0"));
		if (Value) {
			ssaoMaterial = Value->as<Material*>();
		}
	}

	return Compiled;
}

int PostpassStage::Execute(RenderingCamera * Camera, Spatial * spatial, RenderQueue* renderQueue, WorkQueue * Queue, Vector<OsEvent*>& Events) {
	RenderView * renderview = RenderView::Create();
	
	renderview->Camera = Camera;
	renderview->Depth = 0;
	renderview->Type = R_STAGE_POST;
	renderview->Index = 0;
	renderview->Queue = renderQueue;
	// set render target
	renderview->TargetCount = 1;
	renderview->Targets[0] = 0;
	renderview->Depth = Context->GetRenderTarget(String("Depth"));
	renderview->ClearDepth = 0;
	BatchCompiler * Compiler = renderview->Compiler;
	char * Buffer = (char*)renderview->CommandBuffer;
	Compiler->SetBuffer(Buffer);
	renderview->Compile();
	SSAO(Compiler);
	// submit to queue
	renderview->QueueCommand();
	renderview->Recycle();
	return 0;
}

int PostpassStage::End() {
	return 0;
}
