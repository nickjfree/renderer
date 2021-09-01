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
template <class T, class U>
auto AddFSRPass(FrameGraph& frameGraph, RenderContext* renderContext, T& hdrPassData, U& debug)
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
		// fsr consts
		CBFSRConst const1;
		CBFSRConst const2;
		// textures to show
		int* debug;
	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();

	int renderWidth = renderContext->FrameWidth;
	int renderHeight = renderContext->FrameHeight;
	int displayWidth = renderContext->FrameWidth * 1.5f;
	int displayHeight = renderContext->FrameHeight * 1.5f;

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
			desc.BindFlag = (R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE| BIND_RENDER_TARGET);
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
			// debug
			passData.debug = &debug.irrandianceMap;
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
					auto& consts = passData.const1;
					FsrEasuCon(reinterpret_cast<AU1*>(&consts.Const0), reinterpret_cast<AU1*>(&consts.Const1), reinterpret_cast<AU1*>(&consts.Const2), reinterpret_cast<AU1*>(&consts.Const3), static_cast<AF1>(renderWidth), static_cast<AF1>(renderHeight), static_cast<AF1>(renderWidth), static_cast<AF1>(renderHeight), (AF1)displayWidth, (AF1)displayHeight);
					consts.Sample.x = 0;
					cmdBuffer->Dispatch(fsrMaterial, 0, dispatchX, dispatchY, 1)
						.SetShaderConstant(CB_SLOT(CBFSRConst), &consts, sizeof(CBFSRConst))
						.SetShaderResource(SLOT_FSR_INPUT, passData.hdr.GetActualResource())
						.SetRWShaderResource(SLOT_FSR_OUTPUT, passData.intermediary.GetActualResource());
				}
				// rcas
				{
					float rcasAttenuation = 0;
					auto& consts = passData.const2;
					FsrRcasCon(reinterpret_cast<AU1*>(&consts.Const0), 0);
					consts.Sample.x = 0;
					cmdBuffer->Dispatch(fsrMaterial, 1, dispatchX, dispatchY, 1)
						.SetShaderConstant(CB_SLOT(CBFSRConst), &consts, sizeof(CBFSRConst))
						.SetShaderResource(SLOT_FSR_INPUT, passData.intermediary.GetActualResource())
						.SetRWShaderResource(SLOT_FSR_OUTPUT, passData.output.GetActualResource());
				}
				// debug draw
				{
					if (*passData.debug != -1) {
						int targets[] = { passData.output.GetActualResource() };
						cmdBuffer->RenderTargets(targets, 1, -1, false, false, displayWidth, displayHeight);
						cmdBuffer->Quad(fsrMaterial, 2).SetShaderResource(SLOT_RT_GI_DEBUG, *passData.debug);
					}
				}
				// copy pass
				{
					// copy fsr output to backbuffer
					auto target = 0;
					cmdBuffer->CopyResource(target, passData.output.GetActualResource());
				}
			} else {
				return;
			}
		});
	return fsrPass;
}


#endif