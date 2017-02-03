#ifndef __ROOT_SIGNATURE__
#define __ROOT_SIGNATURE__

#include <d3d12.h>
#include "d3dx12.h"

/*
	RootSignature
*/

namespace D3D12API {

#define MAX_DESC_TABLE_SIZE 8

	// descriptor table cache
	typedef struct DescriptorTable {
		int RootSlot;
		int Dirty;
		int Used;
		int ResourceId[MAX_DESC_TABLE_SIZE];
		D3D12_CPU_DESCRIPTOR_HANDLE Handles[MAX_DESC_TABLE_SIZE];
	};
	
	// texture slot info
	typedef struct DescTableSlot {
		DescriptorTable * Table;
		int Offset;
	}DescTable;

	// constant buffer slot. Const buffers only use root parameters.
	typedef struct RootParamSlot {
		int RootSlot;
	} RootParam;


	class RootSignature {

	public:
		// texture slot
		DescTableSlot Textures[64];
		// sampler slot
		DescTableSlot Samplers[16];
		// constant slot
		RootParamSlot Constants[32];
	
	public:
		RootSignature(ID3D12Device * Device);
		virtual ~RootSignature();
		
		// set texture
		void SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// set constant
		void SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC * ConstBufferView);
		// set sampler
		void SetSampler(int slot, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	};

}
#endif 