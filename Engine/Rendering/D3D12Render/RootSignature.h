#ifndef __ROOT_SIGNATURE__
#define __ROOT_SIGNATURE__

#include <d3d12.h>
#include "d3dx12.h"
#include "DescriptorHeap.h"
#include "ShaderBindingTable.h"



/*
	RootSignature
*/

namespace D3D12API {

	// max descriptor number in a descriptor table
	constexpr int MAX_DESC_TABLE_SIZE = 16;
	// max descriptor number in root signature
	constexpr int MAX_DESC_TABLE_NUM = 8;
	// max srv number in rootsig
	constexpr int MAX_SRV_NUM = 64;
	// max uav number in rootsig
	constexpr int MAX_UAV_NUM = 64;
	// max constant number in rootsig
	constexpr int MAX_CONSTANT_NUM = 16;
	// max root srv num
	constexpr int MAX_ROOT_SRV_NUM = 16;

	// descriptor table cache
	typedef struct DescriptorTable {
		int RootSlot;
		int Dirty;
		int Start;
		int End;
		int TableSize;
		D3D12_DESCRIPTOR_RANGE_TYPE DescriptorType;
		int ResourceId[MAX_DESC_TABLE_SIZE];
		D3D12_CPU_DESCRIPTOR_HANDLE Handles[MAX_DESC_TABLE_SIZE];
		//  PrevRaytracingTableHandle
		D3D12_GPU_DESCRIPTOR_HANDLE PrevRaytracingTableHandle;
		int Fresh[MAX_DESC_TABLE_SIZE];
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
			D3D12_CONSTANT_BUFFER_VIEW_DESC constDesc;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		};
	} RootDescriptorSlot;

	enum RootSignatureFlushFlag {
		ROOT_SIGNATURE_FLUSH_GRAPHIC = 1,
		ROOT_SIGNATURE_FLUSH_COMPUTE = 2,
		ROOT_SIGNATURE_FLUSH_BOTH    = 3,
	};

	class RootSignature {

	private:
		// rootsig
		ID3D12RootSignature* RootSig;
		// local or global
		bool Local_;
		// texture slot
		DescTableSlot Textures[MAX_SRV_NUM];
		// uav slot
		DescTableSlot UAVs[MAX_UAV_NUM];
		// sampler slot
		DescTableSlot Samplers[16];
		// constant slot
		RootDescriptorSlot Constants[MAX_CONSTANT_NUM];
		// cached tables 
		DescriptorTable DescTables[MAX_DESC_TABLE_NUM];
		// null handle
		D3D12_CPU_DESCRIPTOR_HANDLE NullHandle;
		// null uav handle
		D3D12_CPU_DESCRIPTOR_HANDLE NullUAVHandle;
		// prev raytracing Descriper heap
		DescriptorHeap* prevRtDescHeap;
		// staging table count
		int NumCachedTables;
		// constant buffer view count
		int NumConstantBuffers;
		// table size
		int TotalTableSize;
	private:
		ID3D12Device* Device;
		// init root signature
		void InitRootSignature(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters);
		// init mapping
		void InitMapping(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters);
		//// init Cache
		//void InitCache(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters);
		// init descriptor table cache
		void InitDescriptorTableCache(int CacheSlot, int rootParameterIndex, D3D12_ROOT_PARAMETER1* rootParameter);
	public:
		RootSignature(ID3D12Device* Device, D3D12_ROOT_PARAMETER1 * rootParameters, int numRootParameters,
			D3D12_CPU_DESCRIPTOR_HANDLE NullHandle, D3D12_CPU_DESCRIPTOR_HANDLE NullUAVHandle, bool Local=false);

		virtual ~RootSignature();
		// get root signature
		ID3D12RootSignature* Get() { return RootSig; }
		// set texture
		bool SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// set texture
		bool SetUAV(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle);
		// set constant
		bool SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC* ConstBufferView);
		// set sampler
		bool SetSamplerTable(ID3D12GraphicsCommandList* CommandList, D3D12_GPU_DESCRIPTOR_HANDLE handle);
		// flush descriptors, constant bindings
		bool Flush(ID3D12GraphicsCommandList* CommandList, DescriptorHeap* descHeap, bool BarrierFlushed, bool HeapChanged, RootSignatureFlushFlag flashFlag=RootSignatureFlushFlag::ROOT_SIGNATURE_FLUSH_GRAPHIC);
		// flush shader binding tables
		bool FlushSBT(DescriptorHeap* descHeap, ShaderRecord * Record);
	};

}
#endif 