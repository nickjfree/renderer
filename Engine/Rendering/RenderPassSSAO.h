#ifndef __RENDER_PASS_SSAO__
#define __RENDER_PASS_SSAO__


/*
	do  ssao 
*/
template <class T, class U>
auto AddSSAOPass(FrameGraph& frameGraph, RenderContext* renderContext, T& lightingPassData, U& gbufferPassData)
{
	typedef struct PassData {
		// lighting input
		// RenderResource lighting;
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource depth;
		// ssao
		RenderResource ao;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto ssaoPass = frameGraph.AddRenderPass<PassData>("ssao",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			// passData.lighting = builder.Read(&lightingPassData.lighting);
			passData.diffuse = builder.Read(&gbufferPassData.diffuse);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			// create ssao final buffer
			passData.ao = builder.Create("ssao", 
				[=](){
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16_FLOAT;
					desc.SampleDesc.Count = 1; 
					// linearz
					desc.DebugName = L"ssao";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			Variant* Value = renderContext->GetResource("Material\\Materials\\ssao.xml\\0");
			Material* ssaoMaterial = nullptr;
			if (Value) {
				ssaoMaterial = Value->as<Material*>();
			}
			if (ssaoMaterial) {
				auto cmd = cmdBuffer->AllocCommand();
				auto target = passData.ao.GetActualResource();
				// scale lighting buffer by 1/4
				cmdBuffer->RenderTargets(cmd, &target, 1, -1, true, false, renderContext->FrameWidth, renderContext->FrameHeight);
				// draw quoad
				cmd = cmdBuffer->AllocCommand();
				// cmd->cmdParameters["gPostBuffer"]= passData.lighting.GetActualResource();
				cmdBuffer->Quad(cmd, ssaoMaterial, 0);
			}
		});
	return ssaoPass;
}

#endif