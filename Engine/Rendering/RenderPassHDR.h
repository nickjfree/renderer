#ifndef __RENDER_PASS_HDR__
#define __RENDER_PASS_HDR__




/*
*	do hdr.  
*	input:
*		lighting buffer
*/
template <class T>
auto AddHDRPass(FrameGraph& frameGraph, RenderContext* renderContext, T&resolvedPassData)
{

	constexpr auto max_hdr_lum_buffer = 8;
	constexpr auto scale_array_size = 5;

	typedef struct PassData {
		// input
		RenderResource resolved;
		// scale array to get avg lum
		RenderResource scaleArray[8];
		// adapt lum buffers of prev and current
		RenderResource adaptLum0;
		RenderResource adaptLum1;
		// bright pass
		RenderResource brightBuffer;
		// bloom
		RenderResource bloom0;
		RenderResource bloom1;
		RenderResource bloom2;
		// star
		RenderResource star;
		// hdr
		RenderResource hdr;
		// sample offsets
		float scaleOffset[max_hdr_lum_buffer][16];
		float brightOffset[16];
		float bloomOffset[2][16];
		float bloomWeight[16];
		// current time
		unsigned int time;

	}PassData;

	auto renderInterface = renderContext->GetRenderInterface();
	auto hdrPass = frameGraph.AddRenderPass<PassData>("hdr",
		[&](GraphBuilder& builder, PassData& passData) {
			// init time
			passData.time = GetCurrentTime();
			// read lighting input
			passData.resolved = builder.Read(&resolvedPassData.resolved);
			// create the scale array
			{
				int width = 4096;
				int height = 4096;
				R_TEXTURE2D_DESC desc = {};
				desc.ArraySize = 1;
				desc.CPUAccess = (R_CPU_ACCESS)0;
				desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
				desc.MipLevels = 1;
				desc.Usage = DEFAULT;
				desc.SampleDesc.Count = 1;

				for (auto i = 0; i < max_hdr_lum_buffer - 1; ++i) {
					width /= 4;
					height /= 4;
					desc.Width = width;
					desc.Height = height;
					desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
					desc.Format = FORMAT_R16_FLOAT;
					if (width < 4 && height < 4) {
						// 
						passData.adaptLum0 = builder.Create("hdr-adapt-lum-0",
							[=]() mutable {
								desc.DebugName = L"hdr-adapt-lum-0";
								return renderInterface->CreateTexture2D(&desc);
							});
						passData.adaptLum1 = builder.Create("hdr-adapt-lum-1",
							[=]() mutable {
								desc.DebugName = L"hdr-adapt-lum-1";
								return renderInterface->CreateTexture2D(&desc);
							});
						break;
					} else {
						passData.scaleArray[i] = builder.Create("hdr-downsample-buffer",
							[=]() mutable {
								desc.DebugName = L"hdr-downsample-buffer";
								return renderInterface->CreateTexture2D(&desc);
							});
					}
				}
			}
			// create bloom buffer and bright buffer
			{
				int width = renderContext->FrameWidth;
				int height = renderContext->FrameHeight;
				R_TEXTURE2D_DESC desc = {};
				desc.ArraySize = 1;
				desc.CPUAccess = (R_CPU_ACCESS)0;
				desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
				desc.MipLevels = 1;
				desc.Usage = DEFAULT;
				desc.SampleDesc.Count = 1;
				desc.Width = width / 2;
				desc.Height = height / 2;

				passData.brightBuffer = builder.Create("hdr-bright-pass-target",
					[=]() mutable {
						desc.Width = width / 2;
						desc.Height = height / 2;
						desc.DebugName = L"hdr-bright-pass-target";
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						return renderInterface->CreateTexture2D(&desc);
					});
				passData.bloom0 = builder.Create("hdr-bloom-pass-buffer-0",
					[=]() mutable {
						desc.Width = width / 8;
						desc.Height = height / 8;
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						desc.DebugName = L"hdr-bloom-pass-buffer-0";
						return renderInterface->CreateTexture2D(&desc);
					});
				passData.bloom1 = builder.Create("hdr-bloom-pass-buffer-1",
					[=]() mutable {
						desc.Width = width / 8;
						desc.Height = height / 8;
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						desc.DebugName = L"hdr-bloom-pass-buffer-1";
						return renderInterface->CreateTexture2D(&desc);
					});
				passData.bloom2 = builder.Create("hdr-bloom-pass-buffer-2",
					[=]() mutable {
						desc.Width = width / 8;
						desc.Height = height / 8;
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						desc.DebugName = L"hdr-bloom-pass-buffer-2";
						return renderInterface->CreateTexture2D(&desc);
					});
				passData.star = builder.Create("hdr-star",
					[=]() mutable {
						desc.Width = width / 8;
						desc.Height = height / 8;
						desc.DebugName = L"hdr-star";
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						return renderInterface->CreateTexture2D(&desc);
					});
			}
			// create hdr texture
			{
				int width = renderContext->FrameWidth;
				int height = renderContext->FrameHeight;
				R_TEXTURE2D_DESC desc = {};
				desc.ArraySize = 1;
				desc.CPUAccess = (R_CPU_ACCESS)0;
				desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
				desc.MipLevels = 1;
				desc.Usage = DEFAULT;
				desc.SampleDesc.Count = 1;
				desc.Width = width;
				desc.Height = height;

				passData.hdr = builder.Create("hdr",
					[=]() mutable {
						desc.DebugName = L"hdr";
						desc.Format = FORMAT_R16G16B16A16_FLOAT;
						return renderInterface->CreateTexture2D(&desc);
					});
			}
			// init sample offsets
			{
				int width = renderContext->FrameWidth;
				int height = renderContext->FrameHeight;
				for (int i = 0; i < max_hdr_lum_buffer; i++) {
					int index = 0;
					float tux = 1.0f / width;
					float tuy = 1.0f / height;
					for (int x = 0; x < 4; x += 2) {
						for (int y = 0; y < 4; y += 2) {
							passData.scaleOffset[i][index] = (x - 1) * tux;
							passData.scaleOffset[i][index + 1] = (y - 1) * tuy;
							index += 4;
						}
					}
					if (width == 1 && height == 1) {
						break;
					}
					if (i == 0) {
						width = 1024;
						height = 1024;
					} else {
						width /= 4;
						height /= 4;
					}
				}
				// bright pass offset
				float tux = 2.0f / renderContext->FrameWidth;
				float tuy = 2.0f / renderContext->FrameHeight;
				int index = 0;
				for (int x = 0; x < 4; x += 2) {
					for (int y = 0; y < 4; y += 2) {
						passData.brightOffset[index] = (x - 1) * tux;
						passData.brightOffset[index + 1] = (y - 1) * tuy;
						index += 4;
					}
				}
				// bloom gauss bloom offset
				tux = 8.0f / renderContext->FrameWidth;
				tuy = 8.0f / renderContext->FrameHeight;
				memset(passData.bloomOffset, 0, sizeof(float) * 32);
				memset(passData.bloomWeight, 0, sizeof(float) * 16);
				// horizion
				passData.bloomOffset[0][4] = (1.0f + 0.5f / 1.3f) * tux;
				passData.bloomOffset[0][8] = -passData.bloomOffset[0][4];
				// vertical
				passData.bloomOffset[1][5] = (1.0f + 0.5f / 1.3f) * tuy;
				passData.bloomOffset[1][9] = -passData.bloomOffset[1][5];
				// bloom weight
				passData.bloomWeight[0] = 1.0f;
				passData.bloomWeight[4] = passData.bloomWeight[8] = 1.3f;
			}
		},
		[=](PassData& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) {

			Variant* Value = renderContext->GetResource("Material\\Materials\\hdr.xml\\0");
			Material* hdrMaterial = nullptr;
			if (Value) {
				hdrMaterial = Value->as<Material*>();
			}
			if (hdrMaterial) {
				// scale by 4
				{
					auto cmd = cmdBuffer->AllocCommand();
					auto scaled = passData.scaleArray[0].GetActualResource();
					// scale lighting buffer by 1/4
					cmdBuffer->RenderTargets(cmd, &scaled, 1, -1, false, false, 1024, 1024);
					// draw quoad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.resolved.GetActualResource();
					cmdBuffer->Quad(cmd, hdrMaterial, 0);
				}
				// get avg lum
				{
					int width = 1024;  // Context->FrameWidth / 4.0f;
					int height = 1024; // Context->FrameHeight / 4.0f;
					for (int i = 1; i < scale_array_size; i++) {
						width /= 4;
						height /= 4;
						// set render target
						auto cmd = cmdBuffer->AllocCommand();
						auto target = passData.scaleArray[i].GetActualResource();
						cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, width, height);
						// set the previous lumbuffer as gPostBuffer
						cmd = cmdBuffer->AllocCommand();
						cmd->cmdParameters["gPostBuffer"].as<int>() = passData.scaleArray[i-1].GetActualResource();
						memcpy_s(&cmd->cmdParameters["gSampleOffsets"], sizeof(Variant), passData.scaleOffset[i], sizeof(passData.scaleOffset[0]));
						cmdBuffer->Quad(cmd, hdrMaterial, 1);
					}
				}
				// adapt lum
				{
					// flip adaptLum with prev frame
					passData.adaptLum0.Flip(&passData.adaptLum1);
					auto cmd = cmdBuffer->AllocCommand();
					auto target = passData.adaptLum0.GetActualResource();
					cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, 1, 1);
					// set parameters
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.scaleArray[scale_array_size-1].GetActualResource();
					cmd->cmdParameters["gDiffuseMap0"].as<int>() = passData.adaptLum1.GetActualResource();
					cmd->cmdParameters["gTimeElapse"].as<int>() = GetCurrentTime() - passData.time;
					passData.time = GetCurrentTime();
					memcpy_s(&cmd->cmdParameters["gSampleOffsets"], sizeof(Variant), passData.scaleOffset[scale_array_size], sizeof(passData.scaleOffset[0]));
					cmdBuffer->Quad(cmd, hdrMaterial, 2);
				}
				// bright pass
				{
					int width = renderContext->FrameWidth / 2;   // Context->FrameWidth / 4.0f;
					int height = renderContext->FrameHeight / 2;; // Context->FrameHeight / 4.0f;
					// set targets
					auto cmd = cmdBuffer->AllocCommand();
					auto target = passData.brightBuffer.GetActualResource();
					cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, width, height);
					// set parameters
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.resolved.GetActualResource();
					cmd->cmdParameters["gDiffuseMap0"].as<int>() = passData.adaptLum0.GetActualResource();
					cmdBuffer->Quad(cmd, hdrMaterial, 3);
				}
				// bloom
				{
					int width = renderContext->FrameWidth / 8;   // Context->FrameWidth / 4.0f;
					int height = renderContext->FrameHeight / 8;; // Context->FrameHeight / 4.0f;
					// source --> bloom0  downsample
					{
						// set targets
						auto cmd = cmdBuffer->AllocCommand();
						auto target = passData.bloom0.GetActualResource();
						cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, width, height);
						// quad
						cmd = cmdBuffer->AllocCommand();
						memcpy_s(&cmd->cmdParameters["gSampleOffsets"], sizeof(Variant), passData.brightOffset, sizeof(passData.brightOffset));
						cmd->cmdParameters["gPostBuffer"].as<int>() = passData.brightBuffer.GetActualResource();
						cmdBuffer->Quad(cmd, hdrMaterial, 1);
					}
					// bloom0 --> bloom1  horizion
					{
						// set targets
						auto cmd = cmdBuffer->AllocCommand();
						auto target = passData.bloom1.GetActualResource();
						cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, width, height);
						// quad
						cmd = cmdBuffer->AllocCommand();
						memcpy_s(&cmd->cmdParameters["gSampleWeights"], sizeof(Variant), passData.bloomWeight, sizeof(passData.bloomWeight));
						memcpy_s(&cmd->cmdParameters["gSampleOffsets"], sizeof(Variant), passData.bloomOffset[0], sizeof(passData.bloomOffset[0]));
						cmd->cmdParameters["gPostBuffer"].as<int>() = passData.bloom0.GetActualResource();
						cmdBuffer->Quad(cmd, hdrMaterial, 4);
					}
					//  bloom0 --> bloom1  vertical
					{
						// set targets
						auto cmd = cmdBuffer->AllocCommand();
						auto target = passData.bloom2.GetActualResource();
						cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, width, height);
						// quad
						cmd = cmdBuffer->AllocCommand();
						memcpy_s(&cmd->cmdParameters["gSampleWeights"], sizeof(Variant), passData.bloomWeight, sizeof(passData.bloomWeight));
						memcpy_s(&cmd->cmdParameters["gSampleOffsets"], sizeof(Variant), passData.bloomOffset[1], sizeof(passData.bloomOffset[0]));
						cmd->cmdParameters["gPostBuffer"].as<int>() = passData.bloom1.GetActualResource();
						cmdBuffer->Quad(cmd, hdrMaterial, 4);
					}
				}
				// tone mapping
				{
					// set backbuffer as render target
					auto cmd = cmdBuffer->AllocCommand();
					auto target = passData.hdr.GetActualResource();
					cmdBuffer->RenderTargets(cmd, &target, 1, -1, false, false, renderContext->FrameWidth, renderContext->FrameHeight);
					// quad
					cmd = cmdBuffer->AllocCommand();
					cmd->cmdParameters["gPostBuffer"].as<int>() = passData.resolved.GetActualResource();
					cmd->cmdParameters["gDiffuseMap0"].as<int>() = passData.adaptLum0.GetActualResource();
					cmd->cmdParameters["gDiffuseMap1"].as<int>() = passData.bloom2.GetActualResource();
					cmdBuffer->Quad(cmd, hdrMaterial, 5);
				}
			} else {
				return;
			}
		});
	return hdrPass;
}


#endif