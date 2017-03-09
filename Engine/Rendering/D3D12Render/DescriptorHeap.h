#ifndef __DESCRIPTOR_HEAP__
#define __DESCRIPTOR_HEAP__


/*
	descriptor heap
*/
#include <d3d12.h>
#include "d3dx12.h"
#include "Container\Vector.h"
#include "Container\List.h"


namespace D3D12API {

#define MAX_DESCRIPTOR_SIZE 1024

	class DescriptorHeap {

	private:
		int Size;
		int Current;
		int Increment;
		// pointers
		D3D12_CPU_DESCRIPTOR_HANDLE CpuStart;
		D3D12_GPU_DESCRIPTOR_HANDLE GpuStart;
		// heaps
		ID3D12DescriptorHeap * Heap;
		// device
		ID3D12Device * Device;
		// copy healper datas
		UINT SrcRangeSize[MAX_DESCRIPTOR_SIZE];
		D3D12_CPU_DESCRIPTOR_HANDLE SrcStarts[MAX_DESCRIPTOR_SIZE];
	protected:
		// cpu reitred heaps
		static List<DescriptorHeap> CpuRetired[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		// gpu retired heaps
		static List<DescriptorHeap> GpuRetired[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		// type
		D3D12_DESCRIPTOR_HEAP_TYPE Type;
		// flag
		D3D12_DESCRIPTOR_HEAP_FLAGS Flag;
		// FenceValue
		UINT64 FenceValue;

	public:
		DescriptorHeap(ID3D12Device * Device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag);
		virtual ~DescriptorHeap();
		static DescriptorHeap * Alloc(ID3D12Device * Device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag);
		// retire at FenceValue
		void Retire(UINT64 FenceValue);
		// get heap
		ID3D12DescriptorHeap * Get() { return Heap; }
		// get cpu handle
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(int slot);
		// get GPU handle
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(int slot);
		// Stage descriptors, return start dest positions
		D3D12_GPU_DESCRIPTOR_HANDLE StageDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE * Handles, int PadStart, int Num);
	};

}

#endif 
