#ifndef __PSOCACHE__
#define __PSOCACHE__

#include <d3d12.h>

#include "Rendering\RenderDesc.h"

namespace D3D12API {

	class PSOCache {

	private:
		// hash value
		mutable unsigned int hash;
	public:
		// dirty flag
		int Dirty;
		// ids of shaders
		int VS;
		int PS;
		int GS;
		int DS;
		int HS;
		// id of render state
		int Depth;
		int Rasterizer;
		int Blend;
		// int input element
		int InputLayout;
		// render target and format;
		int NumRTV;
		DXGI_FORMAT RTVFormat[8];
		DXGI_FORMAT DSVFormat;
        // geometry topology
        R_PRIMITIVE_TOPOLOGY_TYPE Top;
	public:
		// operator int
		operator int() const;
		bool operator == (PSOCache& rh);
		bool operator != (PSOCache& rh);
		PSOCache();
		virtual ~PSOCache();
	};

}
#endif 

