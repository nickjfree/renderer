#ifndef __RENDER_PASS__
#define __RENDER_PASS__

#include "FrameGraph.h"
#include "Node.h"


/*
*  create render states
*/
void CreateRenderState(RenderContext* renderContext) {

	auto renderInterface = renderContext->GetRenderInterface();

	R_DEPTH_STENCIL_DESC depth = {};
	// two side stencil for liging pass
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::DECR;
	depth.StencilPassFront = R_STENCIL_OP::KEEP;
	depth.StencilFuncFront = R_CMP_FUNC::ALWAYS;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::INCR;
	depth.StencilPassBack = R_STENCIL_OP::KEEP;
	depth.StencilFuncBack = R_CMP_FUNC::ALWAYS;
	depth.StencilRef = 1;
	renderContext->RegisterRenderState("TwoSideStencil", renderInterface->CreateDepthStencilStatus(&depth));
	// shading pass depth
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::ZERO;
	depth.StencilFuncFront = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::ZERO;
	depth.StencilFuncBack = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilRef = 0;
	renderContext->RegisterRenderState("NoZ", renderInterface->CreateDepthStencilStatus(&depth));
	// normal depth
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 1;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 0;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::ZERO;
	depth.StencilFuncFront = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::ZERO;
	depth.StencilFuncBack = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilRef = 0;
	renderContext->RegisterRenderState("Depth", renderInterface->CreateDepthStencilStatus(&depth));
	// transparent pass depth stencil
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::REPLACE;
	depth.StencilFuncFront = R_CMP_FUNC::ALWAYS;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::REPLACE;
	depth.StencilFuncBack = R_CMP_FUNC::ALWAYS;
	depth.StencilRef = 1;
	renderContext->RegisterRenderState("TransDepth", renderInterface->CreateDepthStencilStatus(&depth));
	// oit resolve/clear pass depth stencil
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 0;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::KEEP;
	depth.StencilFuncFront = R_CMP_FUNC::EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::KEEP;
	depth.StencilFuncBack = R_CMP_FUNC::EQUAL;
	depth.StencilRef = 1;
	renderContext->RegisterRenderState("ResolveDepth", renderInterface->CreateDepthStencilStatus(&depth));
	// light culling pass blend state
	R_BLEND_STATUS blend = {};
	blend.Enable = 0;
	blend.AlphaToCoverage = 0;
	blend.SrcBlend = R_BLEND::BLEND_ONE;
	blend.DestBlend = R_BLEND::BLEND_ONE;
	blend.BlendOp = R_BLEND_OP::BLEND_OP_ADD;
	blend.SrcBlendAlpha = R_BLEND::BLEND_ONE;
	blend.DestBlendAlpha = R_BLEND::BLEND_ONE;
	blend.BlendOpAlpha = R_BLEND_OP::BLEND_OP_ADD;
	blend.Mask = R_BLEND_MASK::DISABLE_ALL;
	renderContext->RegisterRenderState("NoFrame", renderInterface->CreateBlendStatus(&blend));
	// lighting pass addtive  blend
	blend.Enable = 1;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	renderContext->RegisterRenderState("Additive", renderInterface->CreateBlendStatus(&blend));
	// default blend stat
	blend.Enable = 0;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	renderContext->RegisterRenderState("Blend", renderInterface->CreateBlendStatus(&blend));
	// alpha blend
	blend.Enable = 1;
	blend.SrcBlend = R_BLEND::BLEND_ONE;
	blend.DestBlend = R_BLEND::BLEND_SRC_ALPHA;
	blend.BlendOp = R_BLEND_OP::BLEND_OP_ADD;
	blend.SrcBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.DestBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.BlendOpAlpha = R_BLEND_OP::BLEND_OP_ADD;
	renderContext->RegisterRenderState("AlphaBlend", renderInterface->CreateBlendStatus(&blend));
	// a2c
	blend.Enable = 0;
	blend.AlphaToCoverage = 1;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	renderContext->RegisterRenderState("A2C", renderInterface->CreateBlendStatus(&blend));
	// alpha blend layer
	blend.Enable = 1;
	blend.AlphaToCoverage = 0;
	blend.SrcBlend = R_BLEND::BLEND_SRC_ALPHA;
	blend.DestBlend = R_BLEND::BLEND_INV_SRC_ALPHA;
	blend.BlendOp = R_BLEND_OP::BLEND_OP_ADD;
	blend.SrcBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.DestBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.BlendOpAlpha = R_BLEND_OP::BLEND_OP_ADD;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	renderContext->RegisterRenderState("Overlay", renderInterface->CreateBlendStatus(&blend));
	// raster
	R_RASTERIZER_DESC raster = {};
	raster.CullMode = R_CULL::NONE;
	raster.FillMode = R_FILL::R_SOLID;
	raster.FrontCounterClockwise = 1;
	raster.AntialiasedLineEnable = 1;
	raster.MultisampleEnable = 1;
	renderContext->RegisterRenderState("NoCull", renderInterface->CreateRasterizerStatus(&raster));
	raster.CullMode = R_CULL::BACK;
	raster.FillMode = R_FILL::R_SOLID;
	raster.FrontCounterClockwise = 1;
	renderContext->RegisterRenderState("Rasterizer", renderInterface->CreateRasterizerStatus(&raster));
}


/*
	gbuffer pass


	Texture2D gDiffuseBuffer: register(t8);
	Texture2D gCompactBuffer: register(t9);         // xy: normal  z: linearZ(half)  w: objectId,
	Texture2D gDepthBuffer : register(t10);         // linearZ
	Texture2D gSpecularBuffer: register(t11);
	Texture2D gMotionVector: register(t12);
	Texture2D gPrevCompactBuffer: register(t13);

*/
auto AddGBufferPass(FrameGraph& frameGraph, RenderContext* renderContext) {

	typedef struct GBufferPassData {
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource compact1;
		RenderResource depth;
		RenderResource specular;
		RenderResource motion;

		RenderResource zBuffer;
	}GBufferPassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto gBufferPass = frameGraph.AddRenderPass<GBufferPassData>("gbuffer",
		[&](GraphBuilder& builder, GBufferPassData& passData) {
			passData.depth = builder.Create("linear_depth",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R32_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"linear_depth";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.diffuse = builder.Create("diffuse",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R8G8B8A8_UNORM_SRGB;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"diffuse";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.compact0 = builder.Create("compact_buffer-0",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"compact_buffer-0";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.compact1 = builder.Create("compact_buffer-1",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"compact_buffer-1";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.specular = builder.Create("specular",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R8G8B8A8_UNORM;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"specular";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.motion = builder.Create("motion",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"motion";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
			passData.zBuffer = builder.Create("zbuffer",
				[&]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(BIND_DEPTH_STENCIL);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_D24_UNORM_S8_UINT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"zbuffer";
					return renderInterface->CreateTexture2D(&desc, 0, 0, 0);

				});
		},
		[=](GBufferPassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// set gbuffer as render target
			int targets[] = {
				passData.diffuse.GetActualResource(),
				passData.compact0.GetActualResource(),
				passData.depth.GetActualResource(),
				passData.specular.GetActualResource(),
				passData.motion.GetActualResource(),
			};
			auto compact1 = passData.compact1.GetActualResource();
			auto zbuffer = passData.zBuffer.GetActualResource();
			auto cmd = cmdBuffer->AllocCommand();
			cmdBuffer->RenderTargets(cmd, targets, sizeof(targets), zbuffer, true, true, renderContext->FrameWidth, renderContext->FrameHeight);
			// render objects
			Vector<Node*> objects;
			spatial->Query(cam->GetFrustum(), objects, Node::RENDEROBJECT);

			for (auto iter = objects.Begin(); iter != objects.End(); iter++) {
				auto obj = *iter;
				obj->Render(cmdBuffer, R_STAGE_GBUFFER, 0, cam, renderContext);
			}
			// flip compact buffer
			passData.compact0.Flip(&passData.compact1);
		});
	return gBufferPass;
}


#endif