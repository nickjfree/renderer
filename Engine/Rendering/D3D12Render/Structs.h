#ifndef __D3D12_STRUCT__
#define __D3D12_STRUCT__


//#include "d3d11.h"
#include "d3d12.h"
#include "Rendering\RenderInterface.h"
#include "Container/Vector.h"

namespace D3D12API {

#define NUM_FRAMES 2
#define MAX_THREAD 8

	/* resource state */
	typedef struct D3DResourceState {
		D3D12_RESOURCE_STATES CurrentState;
		int BindSlot;
	} D3DResourceState;

	/*
	Texture2D
	*/
	typedef struct D3DTexture {
		// multi frame flag
		int MultiFrame;
		int MultiResource;
		ID3D12Resource* Texture[NUM_FRAMES];
		union {
			DXGI_FORMAT DSVFormat;
			DXGI_FORMAT RTVFormat;
		};
		D3D12_CPU_DESCRIPTOR_HANDLE Resource[NUM_FRAMES];
		D3D12_CPU_DESCRIPTOR_HANDLE UAV[NUM_FRAMES];
		union {
			D3D12_CPU_DESCRIPTOR_HANDLE Target[NUM_FRAMES];
			D3D12_CPU_DESCRIPTOR_HANDLE Depth[NUM_FRAMES];
		};
		D3DResourceState State[NUM_FRAMES];
	}D3DTexture;

	/*
		Geometry
	*/
	typedef struct D3DGeometry {
		ID3D12Resource* VertexResource;
		ID3D12Resource* IndexResource;
		D3D12_VERTEX_BUFFER_VIEW VBV;
		D3D12_INDEX_BUFFER_VIEW IBV;
		unsigned int VSize;
		unsigned int INum;
		unsigned int VBSize;
		R_PRIMITIVE_TOPOLOGY Top;
		// blas list. 1 for static obejects many for deformable objects
		Vector<int> UsedBlas;
		Vector<int> FreeBlas;
	}D3DGeometry;
	/*
		Buffer
	*/
	typedef struct D3DBuffer {
		int MultiFrame;
		int MultiResource;
		ID3D12Resource* BufferResource[NUM_FRAMES];
		D3D12_CPU_DESCRIPTOR_HANDLE UAV[NUM_FRAMES];
		D3D12_CPU_DESCRIPTOR_HANDLE SRV[NUM_FRAMES];
		D3DResourceState State[NUM_FRAMES];
		unsigned int Size;
		unsigned int ByteStride;
	}D3DBuffer;

	/*
		layout
	*/
	typedef struct D3DInoutLayout {
		D3D12_INPUT_LAYOUT_DESC Layout;
		D3D12_INPUT_ELEMENT_DESC Element[32];
		char Names[32][32];
	}D3DInputLayout;

	/* shaders */
	typedef struct D3DRenderShader {
		D3D12_SHADER_BYTECODE ByteCode;
		void* RawCode;
	}D3DRenderShader;


	/* constant buffers */
	typedef struct D3DConstant {
		int Size;
		int Slot;
		void* Buffer;
	}D3DConstant;

	/* render state */
	typedef struct D3DRenderState {
		union {
			D3D12_DEPTH_STENCIL_DESC Depth;
			D3D12_RASTERIZER_DESC Raster;
			D3D12_BLEND_DESC Blend;
		};
		unsigned char StencilRef;
	}D3DRenderState;

	// quad vertex
	typedef struct BasicVertex {
		float x, y, z;
		float u, v;
	}BasicVertex;

	// bottom level acceleration structure
	typedef struct D3DBottomLevelAS {
		// referenced geometry
		int GeometryId;
		// referenced deformable buffer
		int BufferId;

		int MultiFrame;
		int MultiResource;
		// static or dynamic as
		bool Deformable;
		// dirty
		bool Dirty[NUM_FRAMES];
		// resources
		ID3D12Resource* BLAS[NUM_FRAMES];
		ID3D12Resource* Scrach[NUM_FRAMES];
		ID3D12Resource* Deformed[NUM_FRAMES];
		// resource state
		D3DResourceState BLASState[NUM_FRAMES];
		// build desc
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc[NUM_FRAMES];
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs[NUM_FRAMES];
	}D3DBottomLevelAS;

	// hit group and raygen miss
	typedef struct D3DRaytracingShader {
		// collection
		ID3D12StateObject* Collection;
		// shader indentifier 
		void* RaygenShaderIndentifier;
		void* HitGroupShaderIndentifier;
		void* MissShaderIndentifier;
	}D3DRaytracingShader;
}

#endif