#ifndef __RENDER_PASS_FSR__
#define __RENDER_PASS_FSR__


/*
*	FidelityFX Super Resolution 
*	input:
*		hdr buffer
*   output:
*       backbuffer
* 
*/
template <class T>
auto AddFSRPass(FrameGraph& frameGraph, RenderContext* renderContext, T&hdrPassData)
{

	typedef struct PassData {
		// input
		RenderResource hdr;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto fsrPass = frameGraph.AddRenderPass<PassData>("fsr",
		[&](GraphBuilder& builder, PassData& passData) {
			// read hdr input
			passData.hdr = builder.Read(&hdrPassData.hdr);
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {

			Variant* Value = renderContext->GetResource("Material\\Materials\\fsr.xml\\0");
			Material* fsrMaterial = nullptr;
			if (Value) {
				fsrMaterial = Value->as<Material*>();
			}
			if (fsrMaterial) {				
				{
					// set backbuffer as render target
					auto cmd = cmdBuffer->AllocCommand();
					auto target = 0;
					cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, renderContext->FrameWidth * 2, renderContext->FrameHeight * 2);
					// quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gInput"].as<int>() = passData.hdr.GetActualResource();
					// cmdBuffer->Dispatch(cmd, fsrMaterial, 0, 64, 64, 64);
					cmdBuffer->Quad(cmd, fsrMaterial, 0);
				}
			} else {
				return;
			}
		});
	return fsrPass;
}


#endif