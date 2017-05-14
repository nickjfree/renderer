fxc /T vs_5_0 /E VS_LPP_Normal  /Fo basic-pre.vs  LPP.fx
fxc /T ps_5_0 /E PS_LPP_Normal  /Fo basic-pre.ps  LPP.fx

fxc /T vs_5_0 /E VS_LPP_Normal_Instance  /Fo basic-pre-instance.vs  LPP.fx
fxc /T ps_5_0 /E PS_LPP_Normal           /Fo basic-pre-instance.ps  LPP.fx

fxc /T vs_5_0 /E VS /Fo basic.vs  normal.fx
fxc /T ps_5_0 /E PS /Fo basic.ps  normal.fx

fxc /T vs_5_0 /E VS_Instance /Fo basic-instance.vs  normal.fx
fxc /T ps_5_0 /E PS          /Fo basic-instance.ps  normal.fx


fxc /T vs_5_0 /E VS_ScreenQuadLight /Fo pointlight.vs  light.fx
fxc /T ps_5_0 /E PS_ScreenQuadLightShadow /Fo pointlight.ps  light.fx
fxc /T ps_5_0 /E PS_PointLightShadow /Fo light_pbr.ps pbr.fx
fxc /T ps_5_0 /E PS_DirectionLight /Fo light_direction.ps pbr.fx
fxc /T ps_5_0 /E PS_ImageBasedLight /Fo ibl.ps pbr.fx

fxc /T vs_5_0 /E VS /Fo ssao.vs  post_screen.fx
fxc /T ps_5_0 /E PS /Fo ssao.ps  post_screen.fx


fxc /T vs_5_0 /E VS /Fo hdr.vs  hdr.fx


fxc /T ps_5_0 /E PS_Log /Fo hdr_scale.ps  hdr.fx
fxc /T ps_5_0 /E PS_Avg /Fo hdr_avg.ps  hdr.fx
fxc /T ps_5_0 /E PS_Adapt /Fo hdr_adapt.ps  hdr.fx
fxc /T ps_5_0 /E PS_BrightPass /Fo hdr_bright.ps  hdr.fx
fxc /T ps_5_0 /E PS_GaussBloom5x5 /Fo hdr_bloom.ps  hdr.fx
fxc /T ps_5_0 /E PS_ToneMapping /Fo hdr_tone.ps  hdr.fx
