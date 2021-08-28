#ifndef __RENDER_PASS_RAYTRACING__
#define __RENDER_PASS_RAYTRACING__


/*
	do  as build
*/
auto AddBuildASPass(FrameGraph& frameGraph, RenderContext* renderContext)
{
	typedef struct PassData {
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto buildAsPass = frameGraph.AddRenderPass<PassData>("build_as",
		[&](GraphBuilder& builder, PassData& passData) {
			// no input
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// build as
			static Vector<Node*> objects;
			objects.Reset();
			spatial->Query(objects, Node::RENDEROBJECT);
			for (auto iter = objects.Begin(); iter != objects.End(); iter++) {
				auto node = *iter;
				node->UpdateRaytracingStructure(cmdBuffer, cam, renderContext);
			}
		});
	return buildAsPass;
}


/*
	cull light
*/
auto AddLightCullingPass(FrameGraph& frameGraph, RenderContext* renderContext)
{
	typedef struct PassData {
		// culling result
		RenderResource culledLights;
		Vector<Node*> lightObjects;
		// lights and lights to cull
		CBLights  lights;
		CBLightsToCull lightsToCull;
	}PassData;

	constexpr int max_lights_per_cell = 16;
	constexpr int cell_scale = 10;
	constexpr int cell_count = 16;
	constexpr int buffer_size_per_cell = max_lights_per_cell * sizeof(unsigned int);
	constexpr int light_index_buffer_size = buffer_size_per_cell * cell_count * cell_count * cell_count;

	auto renderInterface = renderContext->GetRenderInterface();

	auto pass = frameGraph.AddRenderPass<PassData>("light-culling",
		[&](GraphBuilder& builder, PassData& passData) {
		
			// light index buffer
			passData.culledLights = builder.Create("light-index",
				[=]() mutable {
					R_BUFFER_DESC desc = {};
					desc.Size = light_index_buffer_size;
					desc.CPUAccessFlags = (R_CPU_ACCESS)0;
					desc.CPUData = nullptr;
					desc.Deformable = false;
					desc.StructureByteStride = buffer_size_per_cell;
					desc.Usage = DEFAULT;
					desc.DebugName = L"light-index";
					return renderInterface->CreateBuffer(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {

			// get light culling shaders
			auto value = renderContext->GetResource("Material\\Materials\\light_culling.xml\\0");
			Material* cullingMaterial = nullptr;
			if (value) {
				cullingMaterial = value->as<Material*>();
			}
			if (cullingMaterial) {
				// set up 
				{
					cmdBuffer->Setup(true).SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame));
				}
				// do light culling in compute shader
				// light position radius array
				{
					// set cell infos
					passData.lightsToCull.cellCount = cell_count;
					passData.lightsToCull.cellScale = cell_scale;
					passData.lightsToCull.lightsPerCell = max_lights_per_cell;

					passData.lightObjects.Reset();
					passData.lightsToCull.numLights = 0;
					spatial->Query(passData.lightObjects, Node::LIGHT);
					for (auto iter = passData.lightObjects.Begin(); iter != passData.lightObjects.End(); iter++) {
						auto light = (RenderLight*)*iter;
						if (light->GetLightType() == RenderLight::POINT || light->GetLightType() == RenderLight::DIRECTION) {
							auto index = passData.lightsToCull.numLights;
							passData.lightsToCull.lights[index] = light->GetDesc();
							passData.lights.gLights[index] = light->GetLightData();
							++passData.lightsToCull.numLights;
						}
					}
					// cull the lights
					cmdBuffer->Dispatch(cullingMaterial, 0, cell_count, cell_count, cell_count)
						.SetShaderConstant(CB_SLOT(CBLightsToCull), &passData.lightsToCull, sizeof(CBLightsToCull))
						.SetRWShaderResource(SLOT_LIGHT_CULLING_RESULT, passData.culledLights.GetActualResource());
				}
			}
		});
	return pass;
}

/*
	ray tracing
*/
template <class T, class U, class W>
auto AddRaytracedReflectionPass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData, U& lightCullingPassData, W& giPassData)
{
	typedef struct PassData {
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		RenderResource motion;
		// culling result
		RenderResource culledLights;
		// reflection
		RenderResource reflectionRaw;
		// svgf-color-0 svgf-color-1
		RenderResource color0;
		RenderResource color1;
		// svgf-moments-0 svgf-moments-1
		RenderResource moment0;
		RenderResource moment1;
		// lights
		CBLights*  lights;

	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto raytracingPass = frameGraph.AddRenderPass<PassData>("raytraced-reflection",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.motion = builder.Read(&gbufferPassData.motion);
			passData.culledLights = builder.Read(&lightCullingPassData.culledLights);
			// lights
			passData.lights = &lightCullingPassData.lights;
			// create the output buffers
			R_TEXTURE2D_DESC desc = {};
			desc.Width = renderContext->FrameWidth;
			desc.Height = renderContext->FrameHeight;

			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET);
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			desc.SampleDesc.Count = 1;

			// rt-result
			passData.reflectionRaw = builder.Create("rt-reflection-raw",
				[=]() mutable  {
					desc.DebugName = L"rt-reflection-raw";
					return renderInterface->CreateTexture2D(&desc);
				});
			// color0
			passData.color0 = builder.Create("svgf-color0",
				[=]() mutable {
					desc.DebugName = L"svgf-color0";
					return renderInterface->CreateTexture2D(&desc);
				});
			// color1
			passData.color1 = builder.Create("svgf-color1",
				[=]() mutable {
					desc.DebugName = L"svgf-color1";
					return renderInterface->CreateTexture2D(&desc);
				});
			// moment0
			passData.moment0 = builder.Create("svgf-moment-0",
				[=]() mutable {
					desc.DebugName = L"svgf-moment-0";
					return renderInterface->CreateTexture2D(&desc);
				});
			// moment1
			passData.moment1 = builder.Create("svgf-moment-1",
				[=]() mutable {
					desc.DebugName = L"svgf-moment-1";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// trace reflection ray
			Variant* value = renderContext->GetResource("Material\\Materials\\raytracing.xml\\0");
			Material* material = nullptr;
			if (value) {
				material = value->as<Material*>();
			}
			if (material) {
				auto reflectionRaw = passData.reflectionRaw.GetActualResource();
				// set up 
				{
					// setup
					cmdBuffer->Setup(true)
						.SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame))
						.SetShaderResource(SLOT_GBUFFER_COMPACT, passData.compact0.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_SPECULAR, passData.specular.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_DEPTH, passData.depth.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_MOTION, passData.motion.GetActualResource());
				}
				// disptach rays
				{
					cmdBuffer->DispatchRays(0, material, renderContext->FrameWidth, renderContext->FrameHeight)
						.SetShaderConstant(CB_SLOT(CBLights), passData.lights, sizeof(CBLights))
						.SetRWShaderResource(SLOT_RT_REFLECTION_TARGET, passData.reflectionRaw.GetActualResource())
						.SetShaderResource(SLOT_RT_LIGHTING_LIGHTS, passData.culledLights.GetActualResource());
				}
				// flip color & moment buffer
				passData.color0.Flip(&passData.color1);
				passData.moment0.Flip(&passData.moment1);
				// accumulation 
				{
					int targets[] = {
						passData.color0.GetActualResource(),
						passData.moment0.GetActualResource(),
					};
					cmdBuffer->RenderTargets(targets, 2, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(material, 0)
						// rebind gfx parameter
						.SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame))
						// svgf
						.SetShaderResource(SLOT_SVGF_PREV_COLOR, passData.color1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_PREV_MOMENT, passData.moment1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_INPUT, passData.reflectionRaw.GetActualResource());
				}
				// filter variance
				{
					// set render targets
					int targets[] = { passData.color1.GetActualResource() };
					cmdBuffer->RenderTargets(targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(material, 2)
						.SetShaderResource(SLOT_SVGF_FILTER_COLOR, passData.color0.GetActualResource())
						.SetShaderResource(SLOT_SVGF_FILTER_MOMENT, passData.moment0.GetActualResource());
				}
				{
					// filter
					// set render targets
					int targets[] = { passData.color0.GetActualResource() };
					cmdBuffer->RenderTargets(targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(material, 1)
						.SetShaderResource(SLOT_SVGF_FILTER_COLOR, passData.color1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_FILTER_MOMENT, passData.moment0.GetActualResource());
				}

			}
		});
	return raytracingPass;
}

/*
	reflection resolve
*/
template <class T, class U, class V, class W, class X>
auto AddResolvePass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData, U& lightingPassData, V& aoPassData, W& rtPassData, X& rtLightingPassData)
{
	typedef struct PassData {
		RenderResource lighting;
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		// ao
		RenderResource ao;
		// rt reflection
		RenderResource reflection;
		// rt lighting
		RenderResource rtLighting;
		// result
		RenderResource resolved;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto resolvePass = frameGraph.AddRenderPass<PassData>("resolve",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.lighting = builder.Read(&lightingPassData.lighting);
			passData.diffuse = builder.Read(&gbufferPassData.diffuse);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.ao = builder.Read(&aoPassData.ao);
			passData.reflection = builder.Read(&rtPassData.color0);
			passData.rtLighting = builder.Read(&rtLightingPassData.color0);

			// create the output buffers
			R_TEXTURE2D_DESC desc = {};
			desc.Width = renderContext->FrameWidth;
			desc.Height = renderContext->FrameHeight;

			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET);
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			desc.SampleDesc.Count = 1;

			// resolved
			passData.resolved = builder.Create("resolved",
				[=]() mutable {
					desc.DebugName = L"resolved";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// get shader
			Variant* value = renderContext->GetResource("Material\\Materials\\resolve.xml\\0");
			Material* material = nullptr;
			if (value) {
				material = value->as<Material*>();
			}
			if (material) {
				// set up 
				{
					cmdBuffer->Setup().SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame));
				}
				{
					// set render targets
					int targets[] = { passData.resolved.GetActualResource() };
					cmdBuffer->RenderTargets(targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(material, 0)
						// gbuffer
						.SetShaderResource(SLOT_GBUFFER_DIFFUSE, passData.diffuse.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_COMPACT, passData.compact0.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_SPECULAR, passData.specular.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_DEPTH, passData.depth.GetActualResource())
						// textures to combine
						.SetShaderResource(SLOT_RESOLVE_REFLECTION, passData.reflection.GetActualResource())
						.SetShaderResource(SLOT_RESOLVE_LIGHTING, passData.rtLighting.GetActualResource())
						.SetShaderResource(SLOT_RESOLVE_AO, passData.ao.GetActualResource())
						.SetShaderResource(SLOT_RESOLVE_POST, passData.lighting.GetActualResource());
				}
			}
		});
	return resolvePass;
}

/*
	ray traced lighting (direct lighting)
*/
template <class T, class U, class W>
auto AddRaytracedLightingPass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData, U& lightCullingPassData, W& giPassData)
{
	typedef struct PassData {
		RenderResource diffuse;
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		RenderResource motion;
		// culling result
		RenderResource culledLights;
		// rt-lighting
		RenderResource rtLighting;

		// svgf-color-0 svgf-color-1
		RenderResource color0;
		RenderResource color1;
		// svgf-moments-0 svgf-moments-1
		RenderResource moment0;
		RenderResource moment1;

		// lights
		CBLights* lights;
		
		// TODO: reused render targets
	}PassData;

	constexpr int max_lights_per_cell = 16;
	constexpr int cell_scale = 10;
	constexpr int cell_count = 16;
	constexpr int buffer_size_per_cell = max_lights_per_cell * sizeof(unsigned int);
	constexpr int light_index_buffer_size = buffer_size_per_cell * cell_count * cell_count * cell_count;

	auto renderInterface = renderContext->GetRenderInterface();

	auto raytracingPass = frameGraph.AddRenderPass<PassData>("raytraced-lighting",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.diffuse = builder.Read(&gbufferPassData.diffuse);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.motion = builder.Read(&gbufferPassData.motion);

			passData.culledLights = builder.Read(&lightCullingPassData.culledLights);
			passData.lights = &lightCullingPassData.lights;
			// create the output buffers


			// rt-result
			passData.rtLighting = builder.Create("rt-lighting",
				[=]() mutable {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					desc.DebugName = L"rt-lighting";
					return renderInterface->CreateTexture2D(&desc);
				});

			R_TEXTURE2D_DESC desc = {};
			desc.Width = renderContext->FrameWidth;
			desc.Height = renderContext->FrameHeight;

			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET);
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			desc.SampleDesc.Count = 1;
			
			// color0
			passData.color0 = builder.Create("svgf-color0",
				[=]() mutable {
					desc.DebugName = L"svgf-color0";
					return renderInterface->CreateTexture2D(&desc);
				});
			// color1
			passData.color1 = builder.Create("svgf-color1",
				[=]() mutable {
					desc.DebugName = L"svgf-color1";
					return renderInterface->CreateTexture2D(&desc);
				});
			// moment0
			passData.moment0 = builder.Create("svgf-moment-0",
				[=]() mutable {
					desc.DebugName = L"svgf-moment-0";
					return renderInterface->CreateTexture2D(&desc);
				});
			// moment1
			passData.moment1 = builder.Create("svgf-moment-1",
				[=]() mutable {
					desc.DebugName = L"svgf-moment-1";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			//get raytracing shaders
			Variant* value = renderContext->GetResource("Material\\Materials\\raytracing.xml\\0");
			Material* rtMaterial = nullptr;
			Material* cullingMaterial = nullptr;
			if (value) {
				rtMaterial = value->as<Material*>();
			}
			if (rtMaterial) {
				auto rtLighting = passData.rtLighting.GetActualResource();
				// set up 
				{
					cmdBuffer->Setup(true).SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame));
				}
				{
					// setup gbuffer for following cmd
					cmdBuffer->Setup(true)
						// gbuffer
						.SetShaderResource(SLOT_GBUFFER_DIFFUSE, passData.diffuse.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_COMPACT, passData.compact0.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_SPECULAR, passData.specular.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_DEPTH, passData.depth.GetActualResource())
						.SetShaderResource(SLOT_GBUFFER_MOTION, passData.motion.GetActualResource());
				}
				// disptach rays
				{
					cmdBuffer->DispatchRays(1, rtMaterial, renderContext->FrameWidth, renderContext->FrameHeight)
						.SetShaderConstant(CB_SLOT(CBLights), passData.lights, sizeof(CBLights))
						// culled light index
						.SetShaderResource(SLOT_RT_LIGHTING_LIGHTS, passData.culledLights.GetActualResource())
						// result
						.SetRWShaderResource(SLOT_RT_LIGHTING_TARGET, passData.rtLighting.GetActualResource());
				}
				// flip color & moment buffer
				passData.color0.Flip(&passData.color1);
				passData.moment0.Flip(&passData.moment1);
				// accumulation 
				{
					int targets[] = {
						passData.color0.GetActualResource(),
						passData.moment0.GetActualResource(),
					};
					cmdBuffer->RenderTargets(targets, 2, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(rtMaterial, 0)
						// rebind cbframe
						.SetShaderConstant(CB_SLOT(CBFrame), cam->GetCBFrame(), sizeof(CBFrame))
						// svgf
						.SetShaderResource(SLOT_SVGF_PREV_COLOR, passData.color1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_PREV_MOMENT, passData.moment1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_INPUT, passData.rtLighting.GetActualResource());
				}
				// filter variance
				{
					// set render targets
					int targets[] = { passData.color1.GetActualResource() };
					cmdBuffer->RenderTargets(targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(rtMaterial, 2)
						.SetShaderResource(SLOT_SVGF_FILTER_COLOR, passData.color0.GetActualResource())
						.SetShaderResource(SLOT_SVGF_FILTER_MOMENT, passData.moment0.GetActualResource());
				}
				{
					// filter
					// set render targets
					int targets[] = { passData.color0.GetActualResource() };
					cmdBuffer->RenderTargets(targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmdBuffer->Quad(rtMaterial, 1)
						.SetShaderResource(SLOT_SVGF_FILTER_COLOR, passData.color1.GetActualResource())
						.SetShaderResource(SLOT_SVGF_FILTER_MOMENT, passData.moment0.GetActualResource());
				}
			}
		});
	return raytracingPass;
}

#endif