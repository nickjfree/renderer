#include "GIVolume.h"


USING_ALLOCATER(GIVolume);

GIVolume::GIVolume()
{
	Type = Node::GIVOLUME;
	giVolume.numRaysPerProbe = gi_volume_probe_num_rays;
	giVolume.origin = {};
	giVolume.probeGridSpacing = { 1, 2, 1 };
	giVolume.probeGridCounts = { 1, 1, 1 };
	giVolume.probeMaxRayDistance = gi_volume_probe_ray_distance;
	giVolume.probeNumIrradianceTexels = gi_volume_probe_irradiance_texels;
	giVolume.probeNumDistanceTexels = gi_volume_probe_distance_texels;
	giVolume.rayRotation = Matrix4x4();
	giVolume.normalBias = gi_volume_normal_bias;
	giVolume.viewBias = gi_volume_view_bias;
	giVolume.hysteresis = gi_volume_hysteresis;
	giVolume.distanceExponent = 50.0f;
	giVolume.backfaceThreshold = 0.2f;
	giVolume.brightnessThreshold = 10.0f;
	giVolume.minFrontfaceDistance = 0.3f;
	// set default scale
	SetScale(Vector3(32, 10, 32));
}


int GIVolume::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	// ensure resource create
	if (irradianceBuffer == -1) {
		createResources(renderContext);
		// clear buffer
		int targets[] = { irradianceMap, distanceMap };
		cmdBuffer->RenderTargets(&targets[0], 1, -1, true, false, renderContext->FrameWidth, renderContext->FrameHeight);
		cmdBuffer->RenderTargets(&targets[1], 1, -1, true, false, renderContext->FrameWidth, renderContext->FrameHeight);
		return 0;
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
	updateRandomRotation();

	if (rtMaterial && giMaterial) {

		auto& position = GetPosition();
		giVolume.origin = { position.x, position.y, position.z };
		// do probe tracing
		{
			cmdBuffer->DispatchRays(2, rtMaterial, gi_volume_probe_num_rays, numProbes)
				.SetShaderConstant(CB_SLOT(CBFrame), camera->GetCBFrame(), sizeof(CBFrame))
				.SetShaderConstant(CB_SLOT(CBGIVolume), &giVolume, sizeof(CBGIVolume))
				.SetRWShaderResource(SLOT_RT_GI_IRRADIANCE_OUTPUT, irradianceBuffer)
				// bind the irrandiance and distance map for "infinite bounce"
				.SetShaderResource(SLOT_RT_GI_IRRADIANCE_MAP, irradianceMap)
				.SetShaderResource(SLOT_RT_GI_DISTANCE_MAP, distanceMap)
				.SetShaderResource(SLOT_RT_GI_STATE_MAP, stateMap);
		}
		// update probe state
		{
			cmdBuffer->Dispatch(giMaterial, 4, giVolume.probeGridCounts.y * giVolume.probeGridCounts.x / 8, giVolume.probeGridCounts.z / 4, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, irradianceBuffer)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_OUTPUT, stateMap);
		}
		// blend irradiance and distance
		{
			cmdBuffer->Dispatch(giMaterial, 0, numProbes, 1, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, irradianceBuffer)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_OUTPUT, irradianceMap);

			cmdBuffer->Dispatch(giMaterial, 1, numProbes, 1, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, irradianceBuffer)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_OUTPUT, distanceMap);
		}
		// fix the border
		{
			// irrandiance 
			cmdBuffer->Dispatch(giMaterial, 2, giVolume.probeGridCounts.y * giVolume.probeGridCounts.x, giVolume.probeGridCounts.z, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, irradianceMap);
			// distance
			cmdBuffer->Dispatch(giMaterial, 3, giVolume.probeGridCounts.y * giVolume.probeGridCounts.x, giVolume.probeGridCounts.z, 1)
				.SetRWShaderResource(SLOT_RT_GI_BLEND_INPUT, distanceMap);
		}
		// done
	}
	return 0;
}

void GIVolume::SetScale(Vector3 scale)
{
	// uniform  grid spacing 
	giVolume.probeGridCounts.x = (int)scale.x;
	giVolume.probeGridCounts.y = (int)scale.y;
	giVolume.probeGridCounts.z = (int)scale.z;
	// num probes
	numProbes = giVolume.probeGridCounts.x * giVolume.probeGridCounts.y * giVolume.probeGridCounts.z;
}

void GIVolume::createResources(RenderContext* renderContext)
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
	auto numPlanes = giVolume.probeGridCounts.y;
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
	// state map
	desc.Width = numPlanes * giVolume.probeGridCounts.x;
	desc.Height = giVolume.probeGridCounts.z;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.DebugName = L"gi-state-map";
	stateMap = renderInterface->CreateTexture2D(&desc);
}

void GIVolume::updateRandomRotation()
{
	constexpr float x_2pi = 3.1419565f;


	float u1 = x_2pi * rand()/ (RAND_MAX + 1.0f);
	float cos1 = std::cosf(u1);
	float sin1 = std::sinf(u1);
	
	float u2 = x_2pi * rand() / (RAND_MAX + 1.0f);
	float cos2 = std::cosf(u2);
	float sin2 = std::sinf(u2);

	float u3 = rand() / (RAND_MAX + 1.0f);
	float sq3 = 2.f * std::sqrtf(u3 * (1.f - u3));

	float s2 = 2.f * u3 * sin2 * sin2 - 1.f;
	float c2 = 2.f * u3 * cos2 * cos2 - 1.f;
	float sc = 2.f * u3 * sin2 * cos2;

	// Create the random rotation matrix
	float _11 = cos1 * c2 - sin1 * sc;
	float _12 = sin1 * c2 + cos1 * sc;
	float _13 = sq3 * cos2;

	float _21 = cos1 * sc - sin1 * s2;
	float _22 = sin1 * sc + cos1 * s2;
	float _23 = sq3 * sin2;

	float _31 = cos1 * (sq3 * cos2) - sin1 * (sq3 * sin2);
	float _32 = sin1 * (sq3 * cos2) + cos1 * (sq3 * sin2);
	float _33 = 1.f - 2.f * u3;
	
	Matrix4x4 transform;
	transform.matrix.r[0] = { _11, _12, _13, 0.f };
	transform.matrix.r[1] = { _21, _22, _23, 0.f };
	transform.matrix.r[2] = { _31, _32, _33, 0.f };
	transform.matrix.r[3] = { 0.f, 0.f, 0.f, 1.f };

	giVolume.rayRotation = transform;
	// Matrix4x4::Tranpose(transform, &giVolume.rayRotation);
}