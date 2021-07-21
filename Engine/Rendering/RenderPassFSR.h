#ifndef __RENDER_PASS_FSR__
#define __RENDER_PASS_FSR__

#define A_CPU
#include "ffx_a.h"
#include "ffx_fsr1.h"

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

	constexpr int threadGroupWorkRegionDim = 16;

	typedef struct FSRConstants
	{
		XMUINT4 Const0;
		XMUINT4 Const1;
		XMUINT4 Const2;
		XMUINT4 Const3;
		XMUINT4 Sample;
	}FSRConstants;

	typedef struct PassData {
		// input
		RenderResource hdr;
		// intermediary texture
		RenderResource intermediary;
		// output
		RenderResource output;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	int renderWidth = renderContext->FrameWidth;
	int renderHeight = renderContext->FrameHeight;
	int displayWidth = renderContext->FrameWidth * 2;
	int displayHeight = renderContext->FrameHeight * 2;

	int dispatchX = (displayWidth + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
	int dispatchY = (displayHeight + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;

	auto fsrPass = frameGraph.AddRenderPass<PassData>("fsr",
		[&](GraphBuilder& builder, PassData& passData) {
			// read hdr input
			passData.hdr = builder.Read(&hdrPassData.hdr);
			// create intermediary and output
			R_TEXTURE2D_DESC desc = {};
			desc.ArraySize = 1;
			desc.CPUAccess = (R_CPU_ACCESS)0;
			desc.BindFlag = (R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE);
			desc.MipLevels = 1;
			desc.Usage = DEFAULT;
			desc.SampleDesc.Count = 1;
			desc.Width = displayWidth;
			desc.Height = displayHeight;
			// intermediary texture
			passData.intermediary = builder.Create("intermediary",
				[=]() mutable {
					desc.DebugName = L"intermediary";
					desc.Format = FORMAT_R8G8B8A8_UNORM;
					return renderInterface->CreateTexture2D(&desc);
				});
			// output texture
			passData.output = builder.Create("intermediary",
				[=]() mutable {
					desc.DebugName = L"output";
					desc.Format = FORMAT_R8G8B8A8_UNORM;
					return renderInterface->CreateTexture2D(&desc);
				});
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {

			Variant* Value = renderContext->GetResource("Material\\Materials\\fsr.xml\\0");
			Material* fsrMaterial = nullptr;
			if (Value) {
				fsrMaterial = Value->as<Material*>();
			}
			if (fsrMaterial) {		
				// easu
				{
					FSRConstants  consts{};
					FsrEasuCon(reinterpret_cast<AU1*>(&consts.Const0), reinterpret_cast<AU1*>(&consts.Const1), reinterpret_cast<AU1*>(&consts.Const2), reinterpret_cast<AU1*>(&consts.Const3), static_cast<AF1>(renderWidth), static_cast<AF1>(renderHeight), static_cast<AF1>(renderWidth), static_cast<AF1>(renderHeight), (AF1)displayWidth, (AF1)displayHeight);
					consts.Sample.x = 0;
					auto cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["InputTexture"] = passData.hdr.GetActualResource();
					cmd->cmdParameters["OutputTexture"] = passData.intermediary.GetActualResource();
					cmd->cmdParameters["Const0"] = consts.Const0;
					cmd->cmdParameters["Const1"] = consts.Const1;
					cmd->cmdParameters["Const2"] = consts.Const2;
					cmd->cmdParameters["Const3"] = consts.Const3;
					cmd->cmdParameters["Sample"] = consts.Sample;
					cmdBuffer->Dispatch(cmd, fsrMaterial, 0, dispatchX, dispatchY, 1);
				}
				// rcas
				{
					float rcasAttenuation = 0;
					FSRConstants consts = {};
					FsrRcasCon(reinterpret_cast<AU1*>(&consts.Const0), 0);
					consts.Sample.x = 0;
					auto cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["InputTexture"] = passData.intermediary.GetActualResource();
					cmd->cmdParameters["OutputTexture"] = passData.output.GetActualResource();
					cmd->cmdParameters["Const0"] = consts.Const0;
					cmd->cmdParameters["Const1"] = consts.Const1;
					cmd->cmdParameters["Const2"] = consts.Const2;
					cmd->cmdParameters["Const3"] = consts.Const3;
					cmd->cmdParameters["Sample"] = consts.Sample;
					cmdBuffer->Dispatch(cmd, fsrMaterial, 1, dispatchX, dispatchY, 1);
				}
				// copy pass
				{
					// set backbuffer as render target
					auto cmd = cmdBuffer->AllocCommand();
					auto target = 0;
					cmdBuffer->CopyResource(cmd, target, passData.output.GetActualResource());
				}
			} else {
				return;
			}
		});
	return fsrPass;
}


#endif