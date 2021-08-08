#ifndef __RENDER_PASS_RASTER__
#define __RENDER_PASS__

#include "FrameGraph.h"
#include "Node.h"


/*
*  create render states
*/
void CreateRenderState(RenderContext* renderContext) 
{

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
	renderContext->RegisterRenderState("Stencil", renderInterface->CreateDepthStencilStatus(&depth));
	// shading pass depth
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
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
	renderContext->RegisterRenderState("NoZTest", renderInterface->CreateDepthStencilStatus(&depth));
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
auto AddGBufferPass(FrameGraph& frameGraph, RenderContext* renderContext) 
{

	typedef struct PassData {
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource compact1;
		RenderResource depth;
		RenderResource specular;
		RenderResource motion;
		// depth buffer
		RenderResource zBuffer;
		// per-frame constant
		ShaderConstant<PerFrameData> perFrameConstant;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto gBufferPass = frameGraph.AddRenderPass<PassData>("gbuffer",
		[&](GraphBuilder& builder, PassData& passData) {

			R_TEXTURE2D_DESC desc = {};
			desc.Width = renderContext->FrameWidth;
			desc.Height = renderContext->FrameHeight;
			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.SampleDesc.Count = 1;
			desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);

			passData.depth = builder.Create("linear_depth",
				[=]() mutable {
					desc.Format = FORMAT_R32_FLOAT;
					desc.DebugName = L"linear_depth";
					return renderInterface->CreateTexture2D(&desc);
				});
			passData.diffuse = builder.Create("diffuse",
				[=]() mutable {
					desc.Format = FORMAT_R8G8B8A8_UNORM_SRGB;
					desc.DebugName = L"diffuse";
					return renderInterface->CreateTexture2D(&desc);
				});
			
			passData.compact0 = builder.Create("compact_buffer-0",
				[=]() mutable {
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.DebugName = L"compact_buffer-0";
					return renderInterface->CreateTexture2D(&desc);
				});
			passData.compact1 = builder.Create("compact_buffer-1",
				[=]() mutable  {
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.DebugName = L"compact_buffer-1";
					return renderInterface->CreateTexture2D(&desc);

				});
			passData.specular = builder.Create("specular",
				[=]() mutable {
					desc.Format = FORMAT_R8G8B8A8_UNORM;
					desc.DebugName = L"specular";
					return renderInterface->CreateTexture2D(&desc);

				});
			passData.motion = builder.Create("motion",
				[=]() mutable {
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.DebugName = L"motion";
					return renderInterface->CreateTexture2D(&desc);

				});
			passData.zBuffer = builder.Create("zbuffer",
				[=]() mutable {
					desc.BindFlag = (R_BIND_FLAG)(BIND_DEPTH_STENCIL);
					desc.Format = FORMAT_D24_UNORM_S8_UINT;
					desc.DebugName = L"zbuffer";
					return renderInterface->CreateTexture2D(&desc);

				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// cmdBuffer global pramerers setup
			passData.perFrameConstant = cmdBuffer->GetFrameParameters(cam, renderContext);
			// setup pass
			cmdBuffer->PassSetup()->AddShaderInput(&passData.perFrameConstant);
			// flip compact buffer
			passData.compact0.Flip(&passData.compact1);
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
			// record command
			auto cmd = cmdBuffer->AllocCommand();
			cmdBuffer->RenderTargets(cmd, targets, 5, zbuffer, true, true, renderContext->FrameWidth, renderContext->FrameHeight);
			// set prev compact buffer
			Variant prevCompactBuffer;
			prevCompactBuffer= compact1;
			cmdBuffer->SetGlobalParameter("gPrevCompactBuffer", prevCompactBuffer);
			// render objects
			static Vector<Node*> objects;
			objects.Reset();
			spatial->Query(cam->GetFrustum(), objects, Node::RENDEROBJECT);
			for (auto iter = objects.Begin(); iter != objects.End(); iter++) {
				auto obj = *iter;
				obj->Render(cmdBuffer, R_STAGE_GBUFFER, 0, cam, renderContext);
			}
		});
	return gBufferPass;
}


/*
	do simple lighiting with gbuffer data
*/
template <class T>
auto AddLightingPass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData) 
{
	typedef struct PassData {
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource depth;
		RenderResource specular;
		RenderResource lighting;

		RenderResource zBuffer;

		// per-frame constant
		ShaderConstant<PerFrameData> perFrameConstant;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto lightingPass = frameGraph.AddRenderPass<PassData>("lighting",
		[&](GraphBuilder& builder, PassData& passData) {
			// read input from gbuffer
			passData.diffuse = builder.Read(&gbufferPassData.diffuse);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.zBuffer = builder.Write(&gbufferPassData.zBuffer);
			// create the lighting buffer
			passData.lighting = builder.Create("lighting",
				[=]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"lighting";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// set lighting as render target
			passData.perFrameConstant = cmdBuffer->GetFrameParameters(cam, renderContext);
			// setup pass
			cmdBuffer->PassSetup()->AddShaderInput(&passData.perFrameConstant);
			int targets[] = {
				passData.lighting.GetActualResource(),
			};
			auto zbuffer = passData.zBuffer.GetActualResource();
			auto cmd = cmdBuffer->AllocCommand();
			cmdBuffer->RenderTargets(cmd, targets, 1, zbuffer, true, false, renderContext->FrameWidth, renderContext->FrameHeight);
			// set gbuffer as input
			Variant diffuse, compact0, specular, depth;
			diffuse = passData.diffuse.GetActualResource();
			compact0 = passData.compact0.GetActualResource();
			specular = passData.specular.GetActualResource();
			depth = passData.depth.GetActualResource();
			cmdBuffer->SetGlobalParameter("gDiffuseBuffer", diffuse);
			cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
			cmdBuffer->SetGlobalParameter("gDepthBuffer", depth);
			cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
			// render all the lights
			static Vector<Node*> lights;
			lights.Reset();
			spatial->Query(cam->GetFrustum(), lights, Node::LIGHT);
			for (auto iter = lights.Begin(); iter != lights.End(); iter++) {
				auto light = *iter;
				light->Render(cmdBuffer, 0, 0, cam, renderContext);
			}
		});
	return lightingPass;
}


/*
	 emissive pass
*/
template <class T, class U>
auto AddEmissivePass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData, U& lightingPassData)
{
	typedef struct PassData {
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource depth;
		RenderResource specular;
		RenderResource lighting;

		RenderResource zBuffer;

		// per-frame constant
		ShaderConstant<PerFrameData> perFrameConstant;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto lightingPass = frameGraph.AddRenderPass<PassData>("emiisive",
		[&](GraphBuilder& builder, PassData& passData) {
			// read input from gbuffer
			passData.diffuse = builder.Read(&gbufferPassData.diffuse);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.zBuffer = builder.Write(&gbufferPassData.zBuffer);
			// create the lighting buffer
			passData.lighting = builder.Write(&lightingPassData.lighting);
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			Variant* Value = renderContext->GetResource("Material\\Materials\\emissive.xml\\0");
			Material* emissiveMaterial = nullptr;
			if (Value) {
				emissiveMaterial = Value->as<Material*>();
			}
			if (emissiveMaterial) {
				// set lighting as render target
				passData.perFrameConstant = cmdBuffer->GetFrameParameters(cam, renderContext);
				cmdBuffer->PassSetup()->AddShaderInput(&passData.perFrameConstant);

				int targets[] = {
					passData.lighting.GetActualResource(),
				};
				// set gbuffer as input
				Variant diffuse, compact0, specular, depth;
				diffuse= passData.diffuse.GetActualResource();
				compact0= passData.compact0.GetActualResource();
				specular= passData.specular.GetActualResource();
				depth= passData.depth.GetActualResource();
				cmdBuffer->SetGlobalParameter("gDiffuseBuffer", diffuse);
				cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
				cmdBuffer->SetGlobalParameter("gDepthBuffer", depth);
				cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
				// emissive pass
				auto cmd = cmdBuffer->AllocCommand();
				cmdBuffer->RenderTargets(cmd, targets, 1, -1, true, false, renderContext->FrameWidth, renderContext->FrameHeight);
				// draw quad
				cmdBuffer->Quad(cmd, emissiveMaterial, 0);
			}

		});
	return lightingPass;
}


#endif