#ifndef __D3D12_STRUCT__
#define __D3D12_STRUCT__


#include "d3d11.h"
#include "d3d12.h"

namespace D3D12API {

#define NUM_FRAMES 2
#define MAX_THREAD 8
	
	/* resource state */
	typedef struct D3DResourceState {
		D3D12_RESOURCE_STATES CurrentState;
	} D3DResourceState;
	
	/*
	Texture2D
	*/
	typedef struct D3DTexture {
		// multi frame flag
		int MultiFrame;
		ID3D12Resource * Texture[NUM_FRAMES];
		D3D12_CPU_DESCRIPTOR_HANDLE Resource[NUM_FRAMES];
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
		ID3D12Resource * VertexResource;
		ID3D12Resource * IndexResource;
		unsigned int VSize;
		unsigned int INum;
		unsigned int VBSize;
	}D3DGeometry;

	/*
		layout
	*/
	typedef struct D3DInoutLayout {
		ID3D11InputLayout * Layout;
	}D3DInputLayout;

	/* shaders */
	typedef struct D3DRenderShader {
		union {
			ID3D11VertexShader * VS;
			ID3D11GeometryShader * GS;
			ID3D11HullShader * HS;
			ID3D11DomainShader * DS;
			ID3D11PixelShader * PS;
		};
	}D3DRenderShader;


	/* constant buffers */
	typedef struct D3DConstant {
		int Size;
		int Slot;
		void * Buffer;
	}D3DConstant;

	/* render state */
	typedef struct D3DRenderState {
		union {
			ID3D11DepthStencilState * Depth;
			ID3D11RasterizerState * Raster;
			ID3D11BlendState * Blend;
		};
		unsigned char StencilRef;
	}D3DRenderState;

	// quad vertext
	typedef struct BasicVertex {
		float x, y, z;
		float u, v;
	}BasicVertex;
}

#endif