#ifndef __RENDER_PASS_GI__
#define __RENDER_PASS_GI__


#include "GIVolume.h"
/*
	gi
*/
template <class T>
auto AddGIPass(FrameGraph& frameGraph, RenderContext* renderContext, T& lightCullingPassData)
{
	typedef struct PassData {
		// culling result
		RenderResource culledLights;
		// lights
		CBLights* lights;
		// volumes
		Vector<Node*> volumes;
		// irrandiance map
		int irradianceMap;
		int distanceMap;
		int stateMap;
		// volume
		CBGIVolume* volume;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	auto pass = frameGraph.AddRenderPass<PassData>("gi",
		[&](GraphBuilder& builder, PassData& passData) {
			// nothing
			passData.culledLights = builder.Read(&lightCullingPassData.culledLights);
			passData.lights = &lightCullingPassData.lights;
			
			srand(12345);
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {
			
			// render all the volumes
			passData.volumes.Reset();
			spatial->Query(passData.volumes, Node::GIVOLUME);

			// set lights for  probe tracing pass
			cmdBuffer->Setup(true)
				.SetShaderConstant(CB_SLOT(CBLights), passData.lights, sizeof(CBLights))
				.SetShaderResource(SLOT_RT_LIGHTING_LIGHTS, passData.culledLights.GetActualResource());

			for (auto iter = passData.volumes.Begin(); iter != passData.volumes.End(); iter++) {
				auto volume = *iter;
				volume->Render(cmdBuffer, 0, 0, cam, renderContext);
				passData.irradianceMap = ((GIVolume*)volume)->GetIrradianceMap();
				passData.distanceMap = ((GIVolume*)volume)->GetDistanceMap();
				passData.volume = ((GIVolume*)volume)->GetGIVolume();
				passData.stateMap = ((GIVolume*)volume)->GetStateMap();
			}
		});
	return pass;
}

#endif
