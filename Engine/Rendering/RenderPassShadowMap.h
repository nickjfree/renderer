#ifndef __RENDER_PASS_SHADOWMAP__
#define __RENDER_PASS_SHADOWMAP__

/*
*	do shadow map rendering.
*/
auto AddShadowMapPass(FrameGraph& frameGraph, RenderContext* renderContext) 
{

	constexpr auto max_shadow_maps = 8;

	typedef struct PassData 
	{
		// shadow maps
		RenderResource shadowMaps[8];
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto shadowMapPass = frameGraph.AddRenderPass<PassData>("shadowmap",
		[=](GraphBuilder& builder, PassData& passData) {
			// create all the shaow maps
			R_TEXTURE2D_DESC desc = {};
			desc.Width = renderContext->FrameWidth;
			desc.Height = renderContext->FrameHeight;
			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.BindFlag = (R_BIND_FLAG)(BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE);
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.SampleDesc.Count = 1;
			desc.Format = FORMAT_R32_TYPELESS;
			desc.DebugName = L"shadowmap";
			for (auto i = 0; i < max_shadow_maps; ++i) {
				passData.shadowMaps[i] = builder.Create("shadowmap",
					[=]() mutable {
						return renderInterface->CreateTexture2D(&desc);
					});
			}
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// render shadows for each lights
			cmdBuffer->SetupFrameParameters(cam, renderContext);
			static Vector<Node*> lights;
			lights.Reset();
			spatial->Query(cam->GetFrustum(), lights, Node::LIGHT);
			auto index = 0;
			for (auto iter = lights.Begin(); iter != lights.End(); iter++, index++) {
				auto light = (RenderLight*)(*iter);
				auto shadowMap = passData.shadowMaps[index].GetActualResource();
				light->SetShadowMap(shadowMap);
				// set lights's depth buffer
				auto cmd = cmdBuffer->AllocCommand();
				cmdBuffer->RenderTargets(cmd, nullptr, 0, shadowMap, false, true, renderContext->FrameWidth, renderContext->FrameHeight);
				auto lightCam = light->GetLightCamera();
				// render in light's view
				static Vector<Node*> occluders;
				occluders.Reset();
				spatial->Query(lightCam->GetFrustum(), occluders, Node::RENDEROBJECT);
				for (auto iter = occluders.Begin(); iter != occluders.End(); iter++) {
					auto occluder = (*iter);
					occluder->Render(cmdBuffer, R_STAGE_SHADOW, 0, lightCam, renderContext);
				}
			}
		});
	return shadowMapPass;
}

#endif