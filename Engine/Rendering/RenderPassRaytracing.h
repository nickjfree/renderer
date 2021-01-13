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
auto AddRaytracingPass(FrameGraph& frameGraph, RenderContext* renderContext, T& lightingPassData, U& gbufferPassData)
{
	typedef struct PassData {
		RenderResource lighting;
		RenderResource compact0;
		RenderResource specular;
		RenderResource depth;
		// reflection
		RenderResource reflectionRaw;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto raytracingPass = frameGraph.AddRenderPass<PassData>("raytracing",
		[&](GraphBuilder& builder, PassData& passData) {
			// input
			passData.lighting = builder.Read(&lightingPassData.lighting);
			passData.compact0 = builder.Read(&gbufferPassData.compact0);
			passData.depth = builder.Read(&gbufferPassData.depth);
			passData.specular = builder.Read(&gbufferPassData.specular);
			// create raytracing result buffer
			passData.reflectionRaw = builder.Create("rt-reflection-raw",
				[=]() {
					R_TEXTURE2D_DESC desc = {};
					desc.Width = renderContext->FrameWidth;
					desc.Height = renderContext->FrameHeight;

					desc.ArraySize = 1;
					desc.CPUAccess = (R_CPU_ACCESS)0;
					desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE);
					desc.MipLevels = 1;
					desc.Usage = DEFAULT;
					desc.Format = FORMAT_R16G16B16A16_FLOAT;
					desc.SampleDesc.Count = 1;
					// linearz
					desc.DebugName = L"rt-reflection-raw";
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			// setup
			cmdBuffer->SetupFrameParameters(cam, renderContext);
			// trace reflection ray
			Variant* value = renderContext->GetResource("Material\\Materials\\reflection.xml\\0");
			Material* material = nullptr;
			// add frame number
			static int frameNumber = 0;
			++frameNumber;
			if (value) {
				material = value->as<Material*>();
			}
			if (material) {
				auto reflectionRaw = passData.reflectionRaw.GetActualResource();
				// 
				Variant compact0, specular, depth;
				compact0.as<int>() = passData.compact0.GetActualResource();
				specular.as<int>() = passData.specular.GetActualResource();
				depth.as<int>() = passData.depth.GetActualResource();
				cmdBuffer->SetGlobalParameter("gCompactBuffer", compact0);
				cmdBuffer->SetGlobalParameter("gDepthBuffer", depth);
				cmdBuffer->SetGlobalParameter("gSpecularBuffer", specular);
				// disptach rays
				auto cmd = cmdBuffer->AllocCommand();
				cmd->cmdParameters["RenderTarget"].as<int>() = passData.reflectionRaw.GetActualResource();
				cmd->cmdParameters["gFrameNumber"].as<int>() = frameNumber;
				cmd->cmdParameters["gPostBuffer"].as<int>() = passData.lighting.GetActualResource();
				cmdBuffer->DispatchRays(cmd, "default", material, renderContext->FrameWidth, renderContext->FrameHeight);
			}
		});
	return raytracingPass;
}

#endif