#ifndef __IMRENDER_H__
#define __IMRENDER_H__
//
#include "windows.h"
#include "MathLib.h"


#define CONTEXT_DIM  5             // render context dimention
#define CONTEXT_TECH	 0
#define CONTEXT_PASS	 1
#define CONTEXT_MATERIAL 2
#define CONTEXT_TEXTURE  3
#define CONTEXT_GEOMETRY 4

typedef struct _COLOR
{
	_COLOR(float r,float g,float b,float a):fr(r),fg(g),fb(b),fa(a){};
	float fr,fg,fb,fa;
}COLOR;

// shaders,technique and pass describe table
typedef struct PassDesc
{
	char * name;
	int   VTypeId;                       // index in VTypedesc Table
	int   Primitive;
	int   DepthId;
	int   ResterizerId;
	int   BlendId;
	PassDesc * next;
	char * VSFile, *HSFile, *DSFile, *GSFile, * PSFile;               // source filenames
	char * VSMain, *HSMain, *DSMain, *GSMain, * PSMain;               // entry point
	char * VSData, *HSData, *DSData, *GSData, * PSData;				// source files in memory
}PassDesc;

typedef struct TechniqueDesc
{
	char * name;
	TechniqueDesc * next;
	PassDesc *      pass;
}TechniqueDesc;

typedef struct DepthStencilDesc
{
	char * name;
	DepthStencilDesc * next;
	BOOL  DepthEnable;
	DWORD DepthWriteMask;
	char * DepthFunc;
	BOOL  StencilEnable;
	UINT8 StencilReadMask;
	UINT8 StencilWriteMask;
	char * FrontStencilFailOp;
	char * FrontStencilDepthFailOp;
	char * FrontStencilPassOp;
	char * FrontStencilFunc;
	char * BackStencilFailOp;
	char * BackStencilDepthFailOp;
	char * BackStencilPassOp;
	char * BackStencilFunc;
	int    StencilRef;
}DepthStencilDesc;

typedef struct RasterizerDesc
{
	char * name;
	RasterizerDesc * next;
	char * FillMode;
	char * CullMode;
	BOOL   FrontCounterClockwise;
	int    DepthBias;
	float  DepthBiasClamp;
	float  SlopeScaledDepthBias;
	BOOL   DepthClipEnable;
	BOOL   ScissorEnable;
	BOOL   MultisampleEnable;
	BOOL   AntialiasedLineEnable;
}RasterizeDesc;

typedef struct BlendDesc
{
	char * name;
	BlendDesc * next;
	BOOL   BlendEnable;
	char * SrcBlend;
	char * DestBlend;
	char * BlendOp;
	char * SrcBlendAlpha;
	char * DestBlendAlpha;
	char * BlendOpAlpha;
	char * RenderTargetWriteMask;
}BlendDesc;





class IMRender
{
public:
	virtual HWND Init(HWND Window,int Width,int Height,BOOL FullScreen) = 0;
	virtual void BeginRender(DWORD color) = 0;
	virtual void EndRender() = 0;
	virtual void Release() = 0;
	virtual void Render(int VertexType,void * VBuffer,DWORD VSize,WORD * IBuffer,DWORD INum) = 0;
	virtual void RenderGeometry(int ID) = 0;
	virtual int  SetSkin(short SKinID) = 0;
	virtual short CreateSkin(COLOR * diffuse,COLOR * ambient,COLOR * specular,COLOR * emissive,float sharpness,wchar_t ** TextureList) = 0;
	virtual int CreateTexture(wchar_t ** TextureList, void ** RawData, int Scale, int Type) = 0;
	virtual int  GetDynamicTBuffer(int ID, void ** Data, DWORD * RowPitch) = 0;
	virtual int  PutDynamicTBuffer(int ID) = 0;
	virtual int  RenderInstance(int InstanceType,void * VBuffer,DWORD VSize,short GeometryID) = 0;
	virtual void * GetDevice() = 0;
	virtual void Flush()= 0;
	virtual int  CreateEffect(wchar_t * File) = 0;
	virtual int  BindVertexFormat(int Type,int EffectIndex,int TechIndex,int PassIndex) = 0;
	virtual int  CreateBoneData(int FrameNum,Matrix4x4 * BoneMatrix,int BoneNum) = 0;
	virtual int  UseBoneData(int ID) = 0;
	virtual int  UsePass(int TechIndex,int PassIndex) = 0;
	virtual int		CreateDynamicGeometry(void * VBuffer,DWORD VSize,WORD * IBuffer,DWORD INum) = 0;
	virtual void *  GetDynamicVBuffer(int ID) = 0;
	virtual int  ChangeDynamicBufferSize(int ID,DWORD VSize,DWORD INum) = 0;

	virtual int  UpdateDynamicGemometry(void * GHandle, void * VBuffer,DWORD VSize,WORD * IBuffer,DWORD INum) = 0;
	virtual int  UpdateTexture(void ** THandle, void ** RawData, int Scale, int Type) = 0;
	virtual void * GetDynamicGeometryHandle(int ID) = 0;
	virtual int GetTextureHandle(int ID, void ** Handle) = 0;
	virtual WORD *  GetDynamicIBuffer(int ID) = 0;
	virtual int  PutDyanmicBuffer(int ID) = 0;
	//query interface
	virtual int  IssueOcclusionQuery(void * Resulte) = 0;
	virtual int  EndOcclusionQuery() = 0;
	virtual int  CheckOcclusionQuery() = 0;

	// MRT Support
	virtual int UsePostShaderResource() = 0;
	virtual int UseMRT() = 0;
	virtual int UseSRT() = 0;
	virtual int PostProcess() = 0;

	// lighting
	virtual int DrawScreenQuad(void * Vertex) = 0;
	virtual int PerformeHDR() = 0;

	// Render States
	virtual int EnableLightPrePass() = 0;
	virtual int EnableGeometryPass() = 0;
	virtual int EnableLightingPass() = 0;
	virtual int EnableHDRPass() = 0;
	virtual int EnablePostPass() = 0;
	virtual int EnableShadowPass(int Index) = 0;
	virtual int UseScreenQuad() = 0;

	// shader system
	virtual int InitShader(TechniqueDesc * TechDesc,DepthStencilDesc * DSDesc,RasterizerDesc * RSDesc,BlendDesc * BDesc) = 0;

	//set render context
	virtual int   SetRenderContext(int * Context) = 0;
	virtual int * GetRenderContext() = 0; 
	virtual int  SetViewMatrix(Matrix4x4 * ViewMatrix,Vector3 * ViewPoint) = 0;
	virtual int  SetWorldMatrix(Matrix4x4 * WorldMatrix) = 0;
	virtual int  SetLightViewMatrix(Matrix4x4 * ViewMatrix) = 0;
	virtual int  SetProjectionMatrix(Matrix4x4 * ProjectionMatrix) = 0;
	virtual int  SetSkinningMatrix(Matrix4x4 * SkinningMatrix,int Num) = 0;
	virtual int  SetHeightMap(int TextureID) = 0;
	virtual int  SetLightParameter(Vector4 * Position,Vector4 * Color, float Radius,float Intensity, int ShadowMapIndex) = 0;

};


#endif