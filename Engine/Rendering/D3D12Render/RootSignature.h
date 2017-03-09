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
		int Start;
		int End;
		int TableSize;
		int ResourceId[MAX_DESC_TABLE_SIZE];
		D3D12_CPU_DESCRIPTOR_HANDLE Handles[MAX_DESC_TABLE_SIZE];
	} DescriptorTable;
	
	// texture slot info
	typedef struct DescTableSlot {
		int RootSlot;
		int CacheSlot;
		int Offset;
	}DescTable;

	// constant buffer slot. Const buffers only use root parameters.
	typedef struct RootDescriptorSlot {
		int RootSlot;
		int Dirty;
		union {
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		};
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
		// null handle
		D3D12_CPU_DESCRIPTOR_HANDLE NullHandle;
	private:
		ID3D12Device * Device;
		// init root signature
		void InitRootSignature();
		// init mapping
		void InitMapping();
		// init Cache
		void InitCache(D3D12_CPU_DESCRIPTOR_HANDLE NullHandle);

	public:
		RootSignature(ID3D12Device * Device, D3D12_CPU_DESCRIPTOR_HANDLE NullHandle);
		virtual ~RootSignature();
		// get root signature
		ID3D12RootSignature * Get() { return RootSig; }
		// set texture
		bool SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// set constant
		bool SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC * ConstBufferView);
		// set sampler
		bool SetSamplerTable(ID3D12GraphicsCommandList * CommandList, D3D12_GPU_DESCRIPTOR_HANDLE handle);
		// flush descriptors, constant bindings
		bool Flush(ID3D12GraphicsCommandList * CommandList, DescriptorHeap * descHeap);
	};

}
#endif 