#ifndef __HEAP__
#define __HEAP__

#include <d3d12.h>
#include "d3dx12.h"

#include "Container\Vector.h"
#include "Container\List.h"

/*
	d3d12 resource Heap
*/


namespace D3D12API {

#define MAX_CONSTANT_BUFFER_HEAP 256 * 4096
#define CONSTANT_ALIGN 256

	class Heap {

	protected:
		// buffer
		ID3D12Resource* Buffer;
		// CPU heap or GPU heap
		int Type;
		// CPU
		void* CpuData;
		// current offset
		int CurrentOffset;
		// max size
		UINT64 MaxSize;
		// align
		int Align;
		// FenceValue
		UINT64 FenceValue;
		// Device
		ID3D12Device* Device;
		// gpu data
		D3D12_GPU_VIRTUAL_ADDRESS GpuData;
		// heap managers
		static Vector<Heap*> Free;
		static List<Heap> Retired;
	public:
		enum HeapType {
			CPU = 0,
			GPU = 1
		} HeapType;
	public:
		Heap();
		virtual ~Heap();
		// init
		virtual void Init(ID3D12Device* Device, UINT64 MaxSize, int Type, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAGS Flag = D3D12_RESOURCE_FLAG_NONE);
		// alloc
		static Heap* Alloc(ID3D12Device* Device, int Type);
		// Sub Alloc return mapped cpu data
		void* SubAlloc(UINT64 Size);
		// Get Gpu pointer
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress(void* CpuPointer);
		// Get CPU addr
		void* GetCPUAddress(UINT64 Offset);
		// retire
		void Retire(UINT64 FenceValue);
		// free
		void Release();
		// get
		ID3D12Resource* GetResource() { return Buffer; }
	};

}

#endif 
