#ifndef __RENDER_DESC_H__
#define __RENDER_DESC_H__

#include "../Math/LinearMath.h"


/*
	structures used by renderer interface. enum value is the same with d3d for easy use. but a pain in the ass for openGL
*/
#define RENDER_MAX_TARGET 8


/*
	resource usage
*/
enum R_RESOURCE_USAGE {
	DEFAULT = 0,
	IMMUTABLE = 1,
	DYNAMIC = 2,
	STAGING = 3
};

/*
	shader resource types
*/
enum R_SHADER_RESOURCE {
	R_SR_CBUFFER,
	R_SR_TBUFFER,
	R_SR_TEXTURE
};


/*
	Bind Flag
*/
enum R_BIND_FLAG {
	BIND_VERTEX_BUFFER = 0x1L,
	BIND_INDEX_BUFFER = 0x2L,
	BIND_CONSTANT_BUFFER = 0x4L,
	BIND_SHADER_RESOURCE = 0x8L,
	BIND_STREAM_OUTPUT = 0x10L,
	BIND_RENDER_TARGET = 0x20L,
	BIND_DEPTH_STENCIL = 0x40L,
	BIND_UNORDERED_ACCESS = 0x80L,
	BIND_DECODER = 0x200L,
	BIND_VIDEO_ENCODER = 0x400L
};

/*
	CPU Access flag
*/
enum R_CPU_ACCESS {
	CPU_ACCESS_WRITE = 0x10000L,
	CPU_ACCESS_READ = 0x20000L
};

/*
	Misc Flag
*/
enum R_MISC {
	RESOURCE_MISC_GENERATE_MIPS = 0x1L,
	RESOURCE_MISC_SHARED = 0x2L,
	RESOURCE_MISC_TEXTURECUBE = 0x4L,
	RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
	RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
	RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
	RESOURCE_MISC_RESOURCE_CLAMP = 0x80L,
	RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x100L,
	RESOURCE_MISC_GDI_COMPATIBLE = 0x200L,
	RESOURCE_MISC_SHARED_NTHANDLE = 0x800L,
	RESOURCE_MISC_RESTRICTED_CONTENT = 0x1000L,
	RESOURCE_MISC_RESTRICT_SHARED_RESOURCE = 0x2000L,
	RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER = 0x4000L,
	RESOURCE_MISC_GUARDED = 0x8000L,
	RESOURCE_MISC_TILE_POOL = 0x20000L,
	RESOURCE_MISC_TILED = 0x40000L

};

/*
	buffer description
*/

typedef struct R_BUFFER_DESC {
	unsigned int Size;
	R_RESOURCE_USAGE   Usage;
	R_BIND_FLAG        BindFlags;
	R_CPU_ACCESS       CPUAccessFlags;
	R_MISC             MiscFlags;
	unsigned int       StructureByteStride;
	void* CPUData;
	bool Deformable;
	// debug name
	wchar_t * DebugName;
}R_BUFFER_DESC;



/*
	Format Desriptions
*/

enum R_FORMAT {
	FORMAT_UNKNOWN = 0,
	FORMAT_R32G32B32A32_TYPELESS = 1,
	FORMAT_R32G32B32A32_FLOAT = 2,
	FORMAT_R32G32B32A32_UINT = 3,
	FORMAT_R32G32B32A32_SINT = 4,
	FORMAT_R32G32B32_TYPELESS = 5,
	FORMAT_R32G32B32_FLOAT = 6,
	FORMAT_R32G32B32_UINT = 7,
	FORMAT_R32G32B32_SINT = 8,
	FORMAT_R16G16B16A16_TYPELESS = 9,
	FORMAT_R16G16B16A16_FLOAT = 10,
	FORMAT_R16G16B16A16_UNORM = 11,
	FORMAT_R16G16B16A16_UINT = 12,
	FORMAT_R16G16B16A16_SNORM = 13,
	FORMAT_R16G16B16A16_SINT = 14,
	FORMAT_R32G32_TYPELESS = 15,
	FORMAT_R32G32_FLOAT = 16,
	FORMAT_R32G32_UINT = 17,
	FORMAT_R32G32_SINT = 18,
	FORMAT_R32G8X24_TYPELESS = 19,
	FORMAT_D32_FLOAT_S8X24_UINT = 20,
	FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	FORMAT_R10G10B10A2_TYPELESS = 23,
	FORMAT_R10G10B10A2_UNORM = 24,
	FORMAT_R10G10B10A2_UINT = 25,
	FORMAT_R11G11B10_FLOAT = 26,
	FORMAT_R8G8B8A8_TYPELESS = 27,
	FORMAT_R8G8B8A8_UNORM = 28,
	FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	FORMAT_R8G8B8A8_UINT = 30,
	FORMAT_R8G8B8A8_SNORM = 31,
	FORMAT_R8G8B8A8_SINT = 32,
	FORMAT_R16G16_TYPELESS = 33,
	FORMAT_R16G16_FLOAT = 34,
	FORMAT_R16G16_UNORM = 35,
	FORMAT_R16G16_UINT = 36,
	FORMAT_R16G16_SNORM = 37,
	FORMAT_R16G16_SINT = 38,
	FORMAT_R32_TYPELESS = 39,
	FORMAT_D32_FLOAT = 40,
	FORMAT_R32_FLOAT = 41,
	FORMAT_R32_UINT = 42,
	FORMAT_R32_SINT = 43,
	FORMAT_R24G8_TYPELESS = 44,
	FORMAT_D24_UNORM_S8_UINT = 45,
	FORMAT_R24_UNORM_X8_TYPELESS = 46,
	FORMAT_X24_TYPELESS_G8_UINT = 47,
	FORMAT_R8G8_TYPELESS = 48,
	FORMAT_R8G8_UNORM = 49,
	FORMAT_R8G8_UINT = 50,
	FORMAT_R8G8_SNORM = 51,
	FORMAT_R8G8_SINT = 52,
	FORMAT_R16_TYPELESS = 53,
	FORMAT_R16_FLOAT = 54,
	FORMAT_D16_UNORM = 55,
	FORMAT_R16_UNORM = 56,
	FORMAT_R16_UINT = 57,
	FORMAT_R16_SNORM = 58,
	FORMAT_R16_SINT = 59,
	FORMAT_R8_TYPELESS = 60,
	FORMAT_R8_UNORM = 61,
	FORMAT_R8_UINT = 62,
	FORMAT_R8_SNORM = 63,
	FORMAT_R8_SINT = 64,
	FORMAT_A8_UNORM = 65,
	FORMAT_R1_UNORM = 66,
	FORMAT_R9G9B9E5_SHAREDEXP = 67,
	FORMAT_R8G8_B8G8_UNORM = 68,
	FORMAT_G8R8_G8B8_UNORM = 69,
	FORMAT_BC1_TYPELESS = 70,
	FORMAT_BC1_UNORM = 71,
	FORMAT_BC1_UNORM_SRGB = 72,
	FORMAT_BC2_TYPELESS = 73,
	FORMAT_BC2_UNORM = 74,
	FORMAT_BC2_UNORM_SRGB = 75,
	FORMAT_BC3_TYPELESS = 76,
	FORMAT_BC3_UNORM = 77,
	FORMAT_BC3_UNORM_SRGB = 78,
	FORMAT_BC4_TYPELESS = 79,
	FORMAT_BC4_UNORM = 80,
	FORMAT_BC4_SNORM = 81,
	FORMAT_BC5_TYPELESS = 82,
	FORMAT_BC5_UNORM = 83,
	FORMAT_BC5_SNORM = 84,
	FORMAT_B5G6R5_UNORM = 85,
	FORMAT_B5G5R5A1_UNORM = 86,
	FORMAT_B8G8R8A8_UNORM = 87,
	FORMAT_B8G8R8X8_UNORM = 88,
	FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	FORMAT_B8G8R8A8_TYPELESS = 90,
	FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	FORMAT_B8G8R8X8_TYPELESS = 92,
	FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	FORMAT_BC6H_TYPELESS = 94,
	FORMAT_BC6H_UF16 = 95,
	FORMAT_BC6H_SF16 = 96,
	FORMAT_BC7_TYPELESS = 97,
	FORMAT_BC7_UNORM = 98,
	FORMAT_BC7_UNORM_SRGB = 99,
	FORMAT_AYUV = 100,
	FORMAT_Y410 = 101,
	FORMAT_Y416 = 102,
	FORMAT_NV12 = 103,
	FORMAT_P010 = 104,
	FORMAT_P016 = 105,
	FORMAT_420_OPAQUE = 106,
	FORMAT_YUY2 = 107,
	FORMAT_Y210 = 108,
	FORMAT_Y216 = 109,
	FORMAT_NV11 = 110,
	FORMAT_AI44 = 111,
	FORMAT_IA44 = 112,
	FORMAT_P8 = 113,
	FORMAT_A8P8 = 114,
	FORMAT_B4G4R4A4_UNORM = 115,
	FORMAT_FORCE_UINT = 0xffffffffUL
};

/*
	Texture sample desc
*/
typedef struct R_SAMPLE_DESC {
	unsigned int Count;
	unsigned int Quality;
} R_SAMPLE_DESC;

/*
	2D Texture Describe
*/
typedef struct R_TEXTURE2D_DESC {
	unsigned int Width;
	unsigned int Height;
	unsigned int MipLevels;
	unsigned int ArraySize;
	R_FORMAT Format;
	R_RESOURCE_USAGE Usage;
	//Sample
	R_SAMPLE_DESC SampleDesc;
	R_BIND_FLAG BindFlag;
	R_CPU_ACCESS CPUAccess;
	R_MISC MiscFlag;
	// cpudata
	void* CpuData;
	unsigned int Size;
	// debug name
	wchar_t * DebugName;
}R_TEXTURE2D_DESC;



/*
	 Input layout
*/


/*
	Instance type
*/
enum R_INPUT_TYPE {
	R_VERTEX = 0,
	R_INSTANCE = 1,
};

/*
	Input element
*/

typedef struct R_INPUT_ELEMENT {
	char* Semantic;
	int SemanticIndex;
	R_FORMAT Format;
	int Slot;
	int Offset;
	R_INPUT_TYPE Type;
}R_INPUT_ELEMENT;


/*
	Depth and stencil status
*/

/*
	Depth test func
*/
enum R_CMP_FUNC {
	NEVER = 1,
	LESS = 2,
	EQUAL = 3,
	LESS_EQUAL = 4,
	GREATER = 5,
	NOT_EQUAL = 6,
	GREATER_EQUAL = 7,
	ALWAYS = 8
};

typedef enum R_STENCIL_OP {
	KEEP = 1,
	ZERO = 2,
	REPLACE = 3,
	INCR_SAT = 4,
	DECR_SAT = 5,
	INVERT = 6,
	INCR = 7,
	DECR = 8
}R_STENCIL_OP;

/*
	Depth stencil desc
*/

typedef struct R_DEPTH_STENCIL_DESC {
	bool ZWriteEnable;
	bool ZTestEnable;
	R_CMP_FUNC DepthFunc;
	bool StencilEnable;
	bool StencilRead;
	bool StencilWrite;
	R_STENCIL_OP StencilFailBack;
	R_STENCIL_OP DepthFailBack;
	R_STENCIL_OP StencilPassBack;
	R_CMP_FUNC StencilFuncBack;
	R_STENCIL_OP StencilFailFront;
	R_STENCIL_OP DepthFailFront;
	R_STENCIL_OP StencilPassFront;
	R_CMP_FUNC StencilFuncFront;
	unsigned char StencilRef;
}R_DEPTH_STENCIL_DESC;

/*
 Blend Status for a render target
 */

 /*
 typedef struct _RENDER_TARGET_BLEND_DESC1 {
 BOOL           BlendEnable;
 BOOL           LogicOpEnable;
 BLEND    SrcBlend;
 BLEND    DestBlend;
 BLEND_OP BlendOp;
 BLEND    SrcBlendAlpha;
 BLEND    DestBlendAlpha;
 BLEND_OP BlendOpAlpha;
 LOGIC_OP LogicOp;
 UINT8          RenderTargetWriteMask;
 } RENDER_TARGET_BLEND_DESC1;
 */

enum R_BLEND {
	BLEND_ZERO = 1,
	BLEND_ONE = 2,
	BLEND_SRC_COLOR = 3,
	BLEND_INV_SRC_COLOR = 4,
	BLEND_SRC_ALPHA = 5,
	BLEND_INV_SRC_ALPHA = 6,
	BLEND_DEST_ALPHA = 7,
	BLEND_INV_DEST_ALPHA = 8,
	BLEND_DEST_COLOR = 9,
	BLEND_INV_DEST_COLOR = 10,
	BLEND_SRC_ALPHA_SAT = 11,
	BLEND_BLEND_FACTOR = 14,
	BLEND_INV_BLEND_FACTOR = 15,
	BLEND_SRC1_COLOR = 16,
	BLEND_INV_SRC1_COLOR = 17,
	BLEND_SRC1_ALPHA = 18,
	BLEND_INV_SRC1_ALPHA = 19
};

enum R_BLEND_OP {
	BLEND_OP_ADD = 1,
	BLEND_OP_SUBTRACT = 2,
	BLEND_OP_REV_SUBTRACT = 3,
	BLEND_OP_MIN = 4,
	BLEND_OP_MAX = 5
};

enum R_BLEND_MASK {
	DISABLE_ALL = 0,
	ENABLE_ALL = 0xf,
};

/*
	BLEND desc
*/
typedef struct R_BLEND_STATUS {
	bool  Enable;
	bool  AlphaToCoverage;
	R_BLEND    SrcBlend;
	R_BLEND    DestBlend;
	R_BLEND_OP BlendOp;
	R_BLEND    SrcBlendAlpha;
	R_BLEND    DestBlendAlpha;
	R_BLEND_OP BlendOpAlpha;
	R_BLEND_MASK       Mask;
}R_BLEND_STATUS;


/*
	Raster Status
*/


/*
	fill mode
*/
enum R_FILL {
	R_WIREFRAME = 2,
	R_SOLID = 3
};

/*
	cull mode
*/

enum R_CULL {
	NONE = 1,
	FRONT = 2,
	BACK = 3
};

typedef struct R_RASTERIZER_DESC {
	R_FILL FillMode;
	R_CULL CullMode;
	bool           FrontCounterClockwise;
	int             DepthBias;
	float           DepthBiasClamp;
	float           SlopeScaledDepthBias;
	bool            DepthClipEnable;
	bool            ScissorEnable;
	bool            MultisampleEnable;
	bool            AntialiasedLineEnable;
} RASTERIZER_DESC;



/*
	PRIMITIVE TYPE
*/

enum R_PRIMITIVE_TOPOLOGY
{
	R_PRIMITIVE_TOPOLOGY_UNDEFINED = 0,
	R_PRIMITIVE_TOPOLOGY_POINTLIST = 1,
	R_PRIMITIVE_TOPOLOGY_LINELIST = 2,
	R_PRIMITIVE_TOPOLOGY_LINESTRIP = 3,
	R_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4,
	R_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5,
	R_PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10,
	R_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11,
	R_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12,
	R_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
	R_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST = 33,
	R_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST = 34,
	R_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST = 35,
	R_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST = 36,
};

/*
	RIMITIVE_TOPOLOGY_TYPE
*/

enum R_PRIMITIVE_TOPOLOGY_TYPE
{
	R_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED = 0,
	R_PRIMITIVE_TOPOLOGY_TYPE_POINT = 1,
	R_PRIMITIVE_TOPOLOGY_TYPE_LINE = 2,
	R_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE = 3,
	R_PRIMITIVE_TOPOLOGY_TYPE_PATCH = 4,
};


/*
	HW Querys
*/

enum R_QUERY
{
	QUERY_EVENT = 0,
	QUERY_OCCLUSION = (QUERY_EVENT + 1),
	QUERY_TIMESTAMP = (QUERY_OCCLUSION + 1),
	QUERY_TIMESTAMP_DISJOINT = (QUERY_TIMESTAMP + 1),
	QUERY_PIPELINE_STATISTICS = (QUERY_TIMESTAMP_DISJOINT + 1),
	QUERY_OCCLUSION_PREDICATE = (QUERY_PIPELINE_STATISTICS + 1),
	QUERY_SO_STATISTICS = (QUERY_OCCLUSION_PREDICATE + 1),
	QUERY_SO_OVERFLOW_PREDICATE = (QUERY_SO_STATISTICS + 1),
	QUERY_SO_STATISTICS_STREAM0 = (QUERY_SO_OVERFLOW_PREDICATE + 1),
	QUERY_SO_OVERFLOW_PREDICATE_STREAM0 = (QUERY_SO_STATISTICS_STREAM0 + 1),
	QUERY_SO_STATISTICS_STREAM1 = (QUERY_SO_OVERFLOW_PREDICATE_STREAM0 + 1),
	QUERY_SO_OVERFLOW_PREDICATE_STREAM1 = (QUERY_SO_STATISTICS_STREAM1 + 1),
	QUERY_SO_STATISTICS_STREAM2 = (QUERY_SO_OVERFLOW_PREDICATE_STREAM1 + 1),
	QUERY_SO_OVERFLOW_PREDICATE_STREAM2 = (QUERY_SO_STATISTICS_STREAM2 + 1),
	QUERY_SO_STATISTICS_STREAM3 = (QUERY_SO_OVERFLOW_PREDICATE_STREAM2 + 1),
	QUERY_SO_OVERFLOW_PREDICATE_STREAM3 = (QUERY_SO_STATISTICS_STREAM3 + 1)
};

enum R_STAGE
{
	R_SHADOW = 0,
	R_PRE = 1,
	R_LIGHTING = 2,
	R_SHADING = 3,
	R_TRANS = 4,
	R_HOC = 5,
	R_POST = 6,
	R_MAXSTAGE
};

/* raytracing */
enum R_BIDING
{
	R_SRV_TEXTURE,
	R_SRV_BUFFER,
	R_UAV_BUFFER,
	R_UAV_TEXTURE,
	R_VERTEX_BUFFER,
};

typedef struct R_RESOURCE_BINDING
{
	R_BIDING BindingType;
	int Slot;
	int ResourceId;
	int reserved;
}R_RESOURCE_BINDING;



typedef struct R_RAYTRACING_SHADER_BINDINGS {
	// int shader id
	int ShaderId;
	// num bindings
	int NumBindings;
	// biding
	R_RESOURCE_BINDING Bindings[8];
}R_RAYTRACING_SHADER_BINDINGS;

typedef struct R_RAYTRACING_INSTANCE
{
	// matrix
	Matrix4x4 Transform;
    // raytracing geometry
	int rtGeometry;
	// int Matertial id
	int MaterialId;
	// int flag
	int Flag;
	// num shaders
	int NumShaders;
	// shader bindings. 8 ray types; 
	R_RAYTRACING_SHADER_BINDINGS ShaderBindings[8];
}R_RAYTRACING_INSTANCE;



#endif