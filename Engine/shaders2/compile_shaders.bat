set PATH=%PATH%;%WindowsSDK_ExecutablePath_x64%

cd %ProjectDir%shaders\

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

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/gbuffer.ps ^
	/D RENDER_GBUFFER=1^
	rasterization/common_ps.hlsl

fxc /T ps_5_1 /E PSMain  /Fo ../shaders/terrain-gbuffer.ps ^
	/D RENDER_GBUFFER=1^
	/D VS_CLIPMAP=1^
	rasterization/common_ps.hlsl

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

fxc /T cs_5_1 /E CSMain  /Fo ../shaders/light_culling.cs^
	/D IBL=1^
	compute/light_culling_cs.hlsl
