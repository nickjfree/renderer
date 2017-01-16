#include "d3d11.h"

namespace D3D12API {

	/*
	Texture2D
	*/
	typedef struct D3DTexture {
		ID3D11Resource * Texture;
		ID3D11ShaderResourceView * Resource;
		union {
			ID3D11RenderTargetView * Target;
			ID3D11DepthStencilView * Depth;
		};
	}D3DTexture;

	/*
		Geometry
	*/
	typedef struct D3DGeometry {
		ID3D11Buffer * Vertex;
		ID3D11Buffer * Index;
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
		union {
			ID3D11Buffer * Buffer;
		};
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