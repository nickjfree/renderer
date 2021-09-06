set PATH=%PATH%;%WindowsSDK_ExecutablePath_x64%

cd %ProjectDir%shaders\

:: common vs
fxc /T vs_5_1 /E VSMain  /Fo ../shaders/basic-gbuffer.vs ^
	/D RENDER_GBUFFER=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/blendshape-gbuffer.vs ^
	/D RENDER_GBUFFER=1^
	/D VS_BLENDSHAPE=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/instance-gbuffer.vs ^
	/D RENDER_GBUFFER=1^
	/D VS_INSTANCING=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/skin-gbuffer.vs ^
	/D RENDER_GBUFFER=1^
	/D VS_SKINNING=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/terrain-gbuffer.vs ^
	/D RENDER_GBUFFER=1^
	/D VS_CLIPMAP=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/quad.vs ^
	/D RENDER_SCREEN=1^
	rasterization/common_vs.hlsl

fxc /T vs_5_1 /E VSMain  /Fo ../shaders/debug.vs ^
	/D RENDER_DEBUG=1^
	rasterization/common_vs.hlsl

:: common ps
fxc /T ps_5_1 /E PSMain  /Fo ../shaders/debug.ps ^
	/D RENDER_DEBUG=1^
	rasterization/ss_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/gbuffer.ps ^
	/D RENDER_GBUFFER=1^
	rasterization/common_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/terrain-gbuffer.ps ^
	/D RENDER_GBUFFER=1^
	/D VS_CLIPMAP=1^
	rasterization/common_ps.hlsl

:: post processing
fxc /T ps_5_1 /E PSMain  /Fo ../shaders/ibl.ps ^
	/D IBL=1^
	rasterization/ss_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/emissive.ps ^
	/D EMISSIVE=1^
	rasterization/ss_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/ssao.ps ^
	/D SSAO=1^
	rasterization/ss_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/resolve.ps ^
	/D RESOLVE=1^
	rasterization/ss_ps.hlsl


:: light culling
fxc /T cs_5_1 /E CSMain  /Fo ../shaders/light_culling.cs^
	/D IBL=1^
	compute/light_culling_cs.hlsl

:: gi
fxc /T cs_5_1 /E CSMain  /Fo ../shaders/gi_irradiance.cs^
	/D BLEND_IRRADIANCE^
	compute/probe_blending_cs.hlsl

fxc /T cs_5_1 /E CSMain  /Fo ../shaders/gi_distance.cs^
	/D BLEND_DISTANCE^
	compute/probe_blending_cs.hlsl

fxc /T cs_5_1 /E CSMain  /Fo ../shaders/gi_fix_irradiance.cs^
	/D BLEND_IRRADIANCE^
	compute/probe_fix_border_cs.hlsl

fxc /T cs_5_1 /E CSMain  /Fo ../shaders/gi_fix_distance.cs^
	/D BLEND_DISTANCE^
	compute/probe_fix_border_cs.hlsl

fxc /T cs_5_1 /E CSMain  /Fo ../shaders/gi_probe_state.cs^
	compute/probe_update_state_cs.hlsl

:: fsr
fxc /T cs_5_1 /E CS_Main /Fo ../shaders/fsr_easu.cs^
	/D SAMPLE_SLOW_FALLBACK=0 /D SAMPLE_EASU=1 /D SAMPLE_RCAS=0^
	rasterization/fsr/fsr.hlsl

fxc /T cs_5_1 /E CS_Main /Fo ../shaders/fsr_rcas.cs^
	/D SAMPLE_SLOW_FALLBACK=0 /D SAMPLE_EASU=0 /D SAMPLE_RCAS=1^
	rasterization/fsr/fsr.hlsl

:: hdr
fxc /T ps_5_1 /E PS_Log /Fo ../shaders/hdr_scale.ps  rasterization/post/hdr.hlsl
fxc /T ps_5_1 /E PS_Avg /Fo ../shaders/hdr_avg.ps  rasterization/post/hdr.hlsl
fxc /T ps_5_1 /E PS_Adapt /Fo ../shaders/hdr_adapt.ps  rasterization/post/hdr.hlsl
fxc /T ps_5_1 /E PS_BrightPass /Fo ../shaders/hdr_bright.ps  rasterization/post/hdr.hlsl
fxc /T ps_5_1 /E PS_GaussBloom5x5 /Fo ../shaders/hdr_bloom.ps  rasterization/post/hdr.hlsl
fxc /T ps_5_1 /E PS_ToneMapping /Fo ../shaders/hdr_tone.ps  rasterization/post/hdr.hlsl

:: svgf
fxc /T ps_5_1 /E PS_TemporalAccumulation  /Fo ../shaders/temporal_accumulation.ps^
	/D SVGF_ACC=1^
	rasterization/denoising/svgf.hlsl

fxc /T ps_5_1 /E PS_Filter /Fo ../shaders/wavelet_filter.ps rasterization/denoising/svgf.hlsl

fxc /T ps_5_1 /E PS_FilterVariant /Fo ../shaders/variance_filter.ps  rasterization/denoising/svgf.hlsl

:: raytracing
dxc  -Zi -Fo ../shaders/rt-reflection.cso -T lib_6_3   -nologo raytracing/reflection_rt.hlsl

dxc  -Zi -Fo ../shaders/rt-lighting.cso -T lib_6_3  -nologo raytracing/lighting_rt.hlsl

dxc  -Zi -Fo ../shaders/rt-probe.cso -T lib_6_3  -nologo raytracing/probe_tracing_rt.hlsl
