set PATH=%PATH%;%WindowsSDK_ExecutablePath_x64%

cd %ProjectDir%shaders\

fxc /T vs_5_1 /E VS_Basic_GBuffer  /Fo basic-gbuffer.vs  common/basic.hlsl
fxc /T vs_5_1 /E VS_Basic_Simple /Fo basic-simple.vs  common/basic.hlsl

fxc /T ps_5_1 /E PS_GBuffer  /Fo gbuffer.ps  common/basic.hlsl
fxc /T ps_5_1 /E PS_Simple /Fo simple.ps  common/basic.hlsl


fxc /T vs_5_1 /E VS_Instancing_Simple /Fo instance-simple.vs  common/basic.hlsl
fxc /T vs_5_1 /E VS_Instancing_GBuffer  /Fo instance-gbuffer.vs  common/basic.hlsl

fxc /T vs_5_1 /E VS_Skinning_Simple /Fo skin-simple.vs  animation/skinning.hlsl
fxc /T vs_5_1 /E VS_Skinning_GBuffer /Fo skin-gbuffer.vs  animation/skinning.hlsl

fxc /T vs_5_1 /E VS_BlendShape_Simple /Fo blendshape-simple.vs  animation/blendshape.hlsl
fxc /T vs_5_1 /E VS_BlendShape_GBuffer /Fo blendshape-gbuffer.vs  animation/blendshape.hlsl

fxc /T vs_5_1 /E VS_Terrain_GBuffer /Fo terrain-gbuffer.vs  terrain/terrain.hlsl
fxc /T vs_5_1 /E VS_Terrain_Simple /Fo terrain-simple.vs  terrain/terrain.hlsl

fxc /T ps_5_1 /E PS_Terrain_GBuffer /Fo terrain-gbuffer.ps  terrain/terrain.hlsl

fxc /T vs_5_1 /E VS /Fo terrain-virtual.vs  terrain/virtual_texture.hlsl
fxc /T ps_5_1 /E PS /Fo terrain-virtual.ps  terrain/virtual_texture.hlsl


fxc /T vs_5_1 /E VS /Fo quad.vs  common/quad.hlsl


fxc /T ps_5_1 /E PS_Point_Light /Fo point_light.ps lighting/lighting.hlsl
fxc /T ps_5_1 /E PS_Direction_Light /Fo direction_light.ps lighting/lighting.hlsl
fxc /T ps_5_1 /E PS_ImageBased_Light /Fo ibl.ps lighting/lighting.hlsl


fxc /T ps_5_1 /E PS /Fo emissive.ps  post/emissive.hlsl

fxc /T ps_5_1 /E PS /Fo ssao.ps  post/ssao.hlsl


fxc /T ps_5_1 /E PS_Log /Fo hdr_scale.ps  post/hdr.hlsl
fxc /T ps_5_1 /E PS_Avg /Fo hdr_avg.ps  post/hdr.hlsl
fxc /T ps_5_1 /E PS_Adapt /Fo hdr_adapt.ps  post/hdr.hlsl
fxc /T ps_5_1 /E PS_BrightPass /Fo hdr_bright.ps  post/hdr.hlsl
fxc /T ps_5_1 /E PS_GaussBloom5x5 /Fo hdr_bloom.ps  post/hdr.hlsl
fxc /T ps_5_1 /E PS_ToneMapping /Fo hdr_tone.ps  post/hdr.hlsl

fxc /T ps_5_1 /E PS  /D dohdr=1  /Fo oit-init.ps  oit/oit.hlsl
fxc /T vs_5_1 /E VS  /D dohdr=1  /Fo oit-quad.vs  oit/oit.hlsl
fxc /T ps_5_1 /E AOITSPResolvePS /D dohdr=1   /Fo oit-resolve.ps  oit/oit.hlsl
fxc /T ps_5_1 /E AOITSPClearPS /D dohdr=1   /Fo oit-clear.ps  oit/oit.hlsl

fxc /T ps_5_1 /E PS  /Fo resolve.ps  post/resolve.hlsl

dxc  -Zi -Fo rt-reflection.cso -T lib_6_3  -nologo raytracing\rt-reflection.hlsl
dxc  -Zi -Fo rt-lighting.cso -T lib_6_3  -nologo raytracing\rt-lighting.hlsl

fxc /T ps_5_1 /E PS_TemporalAccumulation  /Fo temporal_accumulation.ps  raytracing/denoising.hlsl

fxc /T ps_5_1 /E PS_Filter /Fo wavelet_filter.ps  raytracing/denoising.hlsl

fxc /T ps_5_1 /E PS_FilterVariant /Fo variance_filter.ps  raytracing/denoising.hlsl
