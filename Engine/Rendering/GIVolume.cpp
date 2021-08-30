#include "GIVolume.h"


USING_ALLOCATER(GIVolume);




GIVolume::GIVolume()
{
	Type = Node::GIVOLUME;
	giVolume.numRaysPerProbe = gi_volume_probe_num_rays;
	giVolume.origin = {};
	giVolume.probeGridSpacing = { gi_volume_probe_spacing, gi_volume_probe_spacing, gi_volume_probe_spacing };
	giVolume.probeGridCounts = { 1, 1, 1 };
	giVolume.probeMaxRayDistance = gi_volume_probe_ray_distance;
	giVolume.probeNumIrradianceTexels = gi_volume_probe_irradiance_texels;
	giVolume.probeNumDistanceTexels = gi_volume_probe_distance_texels;
	giVolume.rayRotation = Matrix4x4();
	giVolume.normalBias = gi_volume_normal_bias;
	giVolume.viewBias = gi_volume_view_bias;
	// set default scale
	SetScale(Vector3(10, 10, 10));
}


int GIVolume::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	// ensure resource create
	if (irradianceBuffer == -1) {
		CreateResources(renderContext);
	}
	// get resources
	auto value = renderContext->GetResource("Material\\Materials\\raytracing.xml\\0");
	Material* rtMaterial = nullptr;
	Material* giMaterial = nullptr;
	if (value) {
		rtMaterial = value->as<Material*>();
	}
	value = renderContext->GetResource("Material\\Materials\\gi.xml\\0");
	if (value) {
		giMaterial = value->as<Material*>();
	}
	// random rotation
	Matrix4x4 randRotation;
	Matrix4x4::Tranpose(randRotation, &giVolume.rayRotation);
	if (rtMaterial && giMaterial) {
		// do probe tracing
		{
			cmdBuffer->DispatchRays(2, rtMaterial, gi_volume_probe_num_rays, numProbes)
				.SetShaderConstant(CB_SLOT(CBFrame), camera->GetCBFrame(), sizeof(CBFrame))
				.SetShaderConstant(CB_SLOT(CBGIVolume), &giVolume, sizeof(CBGIVolume))
				.SetRWShaderResource(SLOT_RT_GI_IRRADIANCE_OUTPUT, irradianceBuffer);
		}
		// blend irradiance and distance
		{
			/*cmdBuffer->Dispatch(giMaterial, 0, numProbes, 1, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, irradianceBuffer)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_OUTPUT, irradianceMap);*/
		}
	}
	return 0;
}

void GIVolume::SetPosition(Vector3& Position_)
{
	Node::SetPosition(Position_);
	// set origin
	giVolume.origin = { Position_.x, Position_.y, Position_.z };
}

void GIVolume::SetScale(Vector3 scale)
{
	// uniform  grid spacing 
	giVolume.probeGridCounts.x = (int)scale.x / gi_volume_probe_spacing;
	giVolume.probeGridCounts.y = (int)scale.y / gi_volume_probe_spacing;
	giVolume.probeGridCounts.z = (int)scale.z / gi_volume_probe_spacing;
	// num probes
	numProbes = giVolume.probeGridCounts.x * giVolume.probeGridCounts.y * giVolume.probeGridCounts.z;
}

void GIVolume::CreateResources(RenderContext* renderContext)
{
	auto renderInterface = renderContext->GetRenderInterface();
	// create irrandiance buffer
	R_TEXTURE2D_DESC desc = {};
	desc.Width = gi_volume_probe_num_rays;
	desc.Height = numProbes;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.DebugName = L"gi-irradiance-buffer";
	// irrandiance buffer
	irradianceBuffer = renderInterface->CreateTexture2D(&desc);
	// probes per plane
	auto probesPerPlane = giVolume.probeGridCounts.x * giVolume.probeGridCounts.z;
	auto numPlanes = giVolume.probeGridCounts.z;
	auto irrandianceWidth = numPlanes * giVolume.probeGridCounts.x * (gi_volume_probe_irradiance_texels + 2);
	auto irrandianceHeight = giVolume.probeGridCounts.z * (gi_volume_probe_irradiance_texels + 2);
	auto distanceWidth = numPlanes * giVolume.probeGridCounts.x * (gi_volume_probe_distance_texels + 2);
	auto distanceHeight = giVolume.probeGridCounts.z * (gi_volume_probe_distance_texels + 2);
	// irrandiance map
	desc.Width = irrandianceWidth;
	desc.Height = irrandianceHeight;
	desc.Format = FORMAT_R11G11B10_FLOAT;
	desc.DebugName = L"gi-irradiance-map";
	irradianceMap = renderInterface->CreateTexture2D(&desc);
	// distance map
	desc.Width = distanceWidth;
	desc.Height = distanceHeight;
	desc.Format = FORMAT_R16G16_FLOAT;
	desc.DebugName = L"gi-distance-map";
	distanceMap = renderInterface->CreateTexture2D(&desc);
}