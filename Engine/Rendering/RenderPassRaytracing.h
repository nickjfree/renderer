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

#endif