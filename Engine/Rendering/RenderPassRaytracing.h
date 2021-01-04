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
			auto cmd = cmdBuffer->AllocCommand();
			// build as
			
		});
	return buildAsPass;
}

#endif