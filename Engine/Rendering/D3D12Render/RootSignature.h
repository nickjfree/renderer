#ifndef __ROOT_SIGNATURE__
#define __ROOT_SIGNATURE__

#include <d3d12.h>
#include "d3dx12.h"
#include "DescriptorHeap.h"

/*
	RootSignature
*/

namespace D3D12API {

#define MAX_DESC_TABLE_SIZE 16
	// descriptor table cache
	typedef struct DescriptorTable {
		int RootSlot;
		int Dirty;
		int Used;
		int ResourceId[MAX_DESC_TABLE_SIZE];
		D3D12_CPU_DESCRIPTOR_HANDLE Handles[MAX_DESC_TABLE_SIZE];
	} DescriptorTable;
	
	// texture slot info
	typedef struct DescTableSlot {
		int RootSlot;
		int Offset;
	}DescTable;

	// constant buffer slot. Const buffers only use root parameters.
	typedef struct RootDescriptorSlot {
		int RootSlot;
	} RootDescriptorSlot;


	class RootSignature {

	private:
		// rootsig
		ID3D12RootSignature * RootSig;
		// texture slot
		DescTableSlot Textures[64];
		// sampler slot
		DescTableSlot Samplers[16];
		// constant slot
		RootDescriptorSlot Constants[32];
		// cached tables 
		DescriptorTable DescTables[4];
	private:
		ID3D12Device * Device;
		// init root signature
		void InitRootSignature();

	public:
		RootSignature(ID3D12Device * Device);
		virtual ~RootSignature();
		// get root signature
		ID3D12RootSignature * Get() { return RootSig; }
		// set texture
		void SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// set constant
		void SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC * ConstBufferView);
		// set sampler
		void SetSampler(int slot, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// flush descriptors, constant bindings
		void Flush(ID3D12CommandList * CommandList, DescriptorHeap * descHeap);
	};

}
#endif 