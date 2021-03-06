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
	ray tracing
*/
template <class T, class U>
auto AddRaytracedReflectionPass(FrameGraph& frameGraph, RenderContext* renderContext, T& lightingPassData, U& gbufferPassData)
{
	typedef struct PassData {
		RenderResource lighting;
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		RenderResource motion;
		// reflection
		RenderResource reflectionRaw;
		// svgf-color-0 svgf-color-1
		RenderResource color0;
		RenderResource color1;
		// svgf-moments-0 svgf-moments-1
		RenderResource moment0;
		RenderResource moment1;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto raytracingPass = frameGraph.AddRenderPass<PassData>("raytraced-reflection",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.lighting = builder.Read(&lightingPassData.lighting);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.motion = builder.Read(&gbufferPassData.motion);

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
			// setup
			cmdBuffer->SetupFrameParameters(cam, renderContext);
			// trace reflection ray
			Variant* value = renderContext->GetResource("Material\\Materials\\raytracing.xml\\0");
			Material* material = nullptr;
			// add frame number
			static int frameNumber = 0;
			++frameNumber;
			if (value) {
				material = value->as<Material*>();
			}
			if (material) {
				auto reflectionRaw = passData.reflectionRaw.GetActualResource();
				// set up 
				{
					Variant compact0, specular, depth, motion;
					compact0.as<int>() = passData.compact0.GetActualResource();
					specular.as<int>() = passData.specular.GetActualResource();
					depth.as<int>() = passData.depth.GetActualResource();
					motion.as<int>() = passData.motion.GetActualResource();
					cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
					cmdBuffer->SetGlobalParameter("gDepthBuffer", depth);
					cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
					cmdBuffer->SetGlobalParameter("gMotionVector", motion);
				}
				// disptach rays
				{
					auto cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["RenderTarget"].as<int>() = passData.reflectionRaw.GetActualResource();
					cmd->cmdParameters["gFrameNumber"].as<int>() = frameNumber;
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.lighting.GetActualResource();
					cmdBuffer->DispatchRays(cmd, 0, material, renderContext->FrameWidth, renderContext->FrameHeight);
				}
				// flip color & moment buffer
				passData.color0.Flip(&passData.color1);
				passData.moment0.Flip(&passData.moment1);
				// accumulation 
				{
					auto cmd = cmdBuffer->AllocCommand();
					int targets[] = {
						passData.color0.GetActualResource(),
						passData.moment0.GetActualResource(),
					};
					cmdBuffer->RenderTargets(cmd, targets, 2, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gPrevColor"].as<int>() = passData.color1.GetActualResource();
					cmd->cmdParameters["gPrevMoment"].as<int>() = passData.moment1.GetActualResource();
					cmd->cmdParameters["gCurrentColor"].as<int>() = passData.reflectionRaw.GetActualResource();
					cmdBuffer->Quad(cmd, material, 0);
				}
				// filter variance
				{
					// set render targets
					auto cmd = cmdBuffer->AllocCommand();
					int targets[] = { passData.color1.GetActualResource() };
					cmdBuffer->RenderTargets(cmd, targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gColor"].as<int>() = passData.color0.GetActualResource();
					cmd->cmdParameters["gMoment"].as<int>() = passData.moment0.GetActualResource();
					cmdBuffer->Quad(cmd, material, 2);
				}
				{
					// filter
					// set render targets
					auto cmd = cmdBuffer->AllocCommand();
					int targets[] = { passData.color0.GetActualResource() };
					cmdBuffer->RenderTargets(cmd, targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gColor"].as<int>() = passData.color1.GetActualResource();
					cmd->cmdParameters["gMoment"].as<int>() = passData.moment0.GetActualResource();
					cmdBuffer->Quad(cmd, material, 1);
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
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.ao = builder.Read(&aoPassData.ao);
			passData.reflection = builder.Read(&rtPassData.color0);
			passData.rtLighting = builder.Read(&rtLightingPassData.rtLighting);

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
			// setup
			cmdBuffer->SetupFrameParameters(cam, renderContext);
			// get shader
			Variant* value = renderContext->GetResource("Material\\Materials\\resolve.xml\\0");
			Material* material = nullptr;
			// add frame number
			static int frameNumber = 0;
			++frameNumber;
			if (value) {
				material = value->as<Material*>();
			}
			if (material) {
				// set up 
				{
					Variant compact0, specular, depth;
					compact0.as<int>() = passData.compact0.GetActualResource();
					specular.as<int>() = passData.specular.GetActualResource();
					depth.as<int>() = passData.depth.GetActualResource();
					cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
					cmdBuffer->SetGlobalParameter("gDepthBuffer", depth); 
					cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
				}
				{
					// set render targets
					auto cmd = cmdBuffer->AllocCommand();
					int targets[] = { passData.resolved.GetActualResource() };
					cmdBuffer->RenderTargets(cmd, targets, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// draw quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gRaytracedReflection"].as<int>() = passData.reflection.GetActualResource();
					cmd->cmdParameters["gRaytracedLighting"].as<int>() = passData.rtLighting.GetActualResource();
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.lighting.GetActualResource();
					cmd->cmdParameters["gAO"].as<int>() = passData.ao.GetActualResource();
					cmdBuffer->Quad(cmd, material, 0); 
				}

			}
		});
	return resolvePass;
}

/*
	ray traced lighting (direct lighting)
*/
template <class T>
auto AddRaytracedLightingPass(FrameGraph& frameGraph, RenderContext* renderContext, T& gbufferPassData)
{
	typedef struct PassData {
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		RenderResource motion;
		// culling result
		RenderResource culledLights;
		// rt-lighting
		RenderResource rtLighting;
		// TODO: reused render targets
	}PassData;

	typedef struct LightInfos
	{
		int numLights;
		int lightsPerCell;
		int cellScale;
		int pad;
		Vector<Vector3> lights;
		Vector<LightData> lightData;
	}LightInfos;

	auto renderInterface = renderContext->GetRenderInterface();

	auto raytracingPass = frameGraph.AddRenderPass<PassData>("raytraced-lighting",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			passData.motion = builder.Read(&gbufferPassData.motion);

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


			// light index buffer
			passData.culledLights = builder.Create("light-index",
				[=]() mutable {
					R_BUFFER_DESC desc = {};
					desc.Size = 1024;
					desc.CPUAccessFlags = (R_CPU_ACCESS)0;
					desc.CPUData = nullptr;
					desc.Deformable = false;
					desc.StructureByteStride = 64;
					desc.Usage = DEFAULT;
					desc.DebugName = L"light-index";
					return renderInterface->CreateBuffer(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// setup
			cmdBuffer->SetupFrameParameters(cam, renderContext);
			//get raytracing shaders
			Variant* value = renderContext->GetResource("Material\\Materials\\raytracing.xml\\0");
			Material* rtMaterial = nullptr;
			Material* cullingMaterial = nullptr;
			// add frame number
			static int frameNumber = 0;
			++frameNumber;
			if (value) {
				rtMaterial = value->as<Material*>();
			}
			// get light culling shaders
			value = renderContext->GetResource("Material\\Materials\\light_culling.xml\\0");
			if (value) {
				cullingMaterial = value->as<Material*>();
			}
			if (rtMaterial && cullingMaterial) {
				auto rtLighting = passData.rtLighting.GetActualResource();
				// set up 
				{
					Variant compact0, specular, depth, motion;
					compact0.as<int>() = passData.compact0.GetActualResource();
					specular.as<int>() = passData.specular.GetActualResource();
					depth.as<int>() = passData.depth.GetActualResource();
					motion.as<int>() = passData.motion.GetActualResource();
					cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
					cmdBuffer->SetGlobalParameter("gDepthBuffer", depth);
					cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
					cmdBuffer->SetGlobalParameter("gMotionVector", motion);
				}
				// do light culling in compute shader
				{
					// lights in scene
					static Vector<Node*> lights;
					lights.Reset();
					// light position radius array
					static LightInfos lightInfos{};
					lightInfos.lights.Reset();
					lightInfos.lightData.Reset();
					spatial->Query(lights, Node::LIGHT);
					for (auto iter = lights.Begin(); iter != lights.End(); iter++) {
						auto light = (RenderLight*)*iter;
						lightInfos.lightData.PushBack(light->GetLightData());
						lightInfos.lights.PushBack(light->GetDesc());
					}
					lightInfos.cellScale = 16;
					lightInfos.lightsPerCell = 15;
					lightInfos.numLights = lightInfos.lights.Size();
					auto cmd = cmdBuffer->AllocCommand();
					// use lighting fot test
					cmd->cmdParameters["lights"].as<void*>() = lightInfos.lights.GetData();
					cmd->cmdParameters["cellScale"].as<void*>() = &lightInfos.cellScale;
					cmd->cmdParameters["lightsPerCell"].as<void*>() = &lightInfos.lightsPerCell;
					cmd->cmdParameters["numLights"].as<void*>() = &lightInfos.numLights;
					cmd->cmdParameters["CulledLights"].as<int>() = passData.culledLights.GetActualResource();
					cmdBuffer->Dispatch(cmd, cullingMaterial, 0, 16, 16, 1);
				}
				// disptach rays
				{
					auto cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["RenderTarget"].as<int>() = passData.rtLighting.GetActualResource();
					cmd->cmdParameters["gFrameNumber"].as<int>() = frameNumber;
					cmdBuffer->DispatchRays(cmd, 1, rtMaterial, renderContext->FrameWidth, renderContext->FrameHeight);
				}
			}
		});
	return raytracingPass;
}
#endif