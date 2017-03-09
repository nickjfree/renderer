#include "RootSignature.h"
#include <stdio.h>


using namespace D3D12API;


RootSignature::RootSignature(ID3D12Device * Device_, D3D12_CPU_DESCRIPTOR_HANDLE NullHandle_) {
	Device = Device_;
	NullHandle = NullHandle_;
	InitRootSignature();
}


RootSignature::~RootSignature() {
}

void RootSignature::InitRootSignature() {
	CD3DX12_DESCRIPTOR_RANGE1 DescRange[4];

	// texture materials src t 0-8
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 9, 0, 0);
	// texture g-buffer srv t 9-13
	DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 9, 0);
	// texture misc srv t 14-20
	DescRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 14, 0);
	// samplers  s 0-2
	DescRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 3, 0);

	/*
		b0  PerObject
		b1  PerFrame
		b2  Animation
		b3  Misc
		table  t0-t8
		table  t9-t13
		table  t14-t20
		table  s0-s2
	*/
	CD3DX12_ROOT_PARAMETER1 RP[8];
	// constant buffer
	RP[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b0
	RP[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b1
	RP[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b2
	RP[3].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b3
	// tables
	RP[4].InitAsDescriptorTable(1, &DescRange[0]);
	RP[5].InitAsDescriptorTable(1, &DescRange[1]);
	RP[6].InitAsDescriptorTable(1, &DescRange[2]);
	RP[7].InitAsDescriptorTable(1, &DescRange[3]);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSig(8, RP, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob * pSerializedRootSig;
	ID3DBlob * pError;
	HRESULT result = D3D12SerializeVersionedRootSignature(&RootSig, &pSerializedRootSig, &pError);
	result = Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
		pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&this->RootSig));
   // init slot mapping
	InitMapping();
	InitCache(NullHandle);
}

// init mapping
void RootSignature::InitMapping() {
	int i = 0;
	int offset = 0;
	// textures
	while (i <= 8) {
		Textures[i].CacheSlot = 0;
		Textures[i].Offset = offset++;
		Textures[i].RootSlot = 4;
		i++;
	}

	offset = 0;
	while (i <= 13) {
		Textures[i].CacheSlot = 1;
		Textures[i].Offset = offset++;
		Textures[i].RootSlot = 5;
		i++;
	}
	offset = 0;
	while (i <= 20) {
		Textures[i].CacheSlot = 2;
		Textures[i].Offset = offset++;
		Textures[i].RootSlot = 6;
		i++;
	}
	// samplers
	i = 0;
	offset = 0;
	while (i <= 2) {
		Samplers[i].CacheSlot = 0;
		Samplers[i].Offset = offset++;
		Samplers[i].RootSlot = 7;
		i++;
	}
	// constant
	i = 0;
	while (i <= 3) {
		Constants[i].RootSlot = i;
		Constants[i].Dirty = 0;
		i++;
	}
}
// init Cache
void RootSignature::InitCache(D3D12_CPU_DESCRIPTOR_HANDLE NullHandle) {
	for (int i = 0; i < 4; i++) {
		DescriptorTable& DescTable = DescTables[i];
		DescTable.Dirty = 0;
		DescTable.RootSlot = i + 4;
		DescTable.Start = MAX_DESC_TABLE_SIZE;
		DescTable.End = -1;
		for (int n = 0; n < MAX_DESC_TABLE_SIZE; n++) {
			DescTable.ResourceId[n] = -1;
			DescTable.Handles[n] = NullHandle;
		}
	}
	DescTables[0].TableSize = 9;
	DescTables[1].TableSize = 5;
	DescTables[2].TableSize = 7;
	DescTables[3].TableSize = 3;
}

// set texture
bool RootSignature::SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	DescTableSlot TableSlot = Textures[slot];
	DescriptorTable& DescTable = DescTables[TableSlot.CacheSlot];
	int Offset = TableSlot.Offset;
	if (DescTable.ResourceId[Offset] != id) {
		DescTable.Dirty = 1;
		DescTable.ResourceId[Offset] = id;
//		DescTable.RootSlot = TableSlot.RootSlot;
		DescTable.Handles[Offset] = handle;
		DescTable.End = max(Offset, DescTable.End);
		DescTable.Start = min(Offset, DescTable.Start);
		return true;
	}
	return false;
}

// set constant
bool RootSignature::SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC * ConstBufferView) {
	Constants[slot].desc = *ConstBufferView;
	Constants[slot].Dirty = 1;
	return true;
}

// set sampler
bool RootSignature::SetSamplerTable(ID3D12GraphicsCommandList * CommandList, D3D12_GPU_DESCRIPTOR_HANDLE handle) {
	CommandList->SetGraphicsRootDescriptorTable(DescTables[3].RootSlot, handle);
	return true;
}

// flush descriptors, constant bindings
bool RootSignature::Flush(ID3D12GraphicsCommandList * CommandList, DescriptorHeap * descHeap) {
	// flush texture bindings
	for (int i = 0; i < 3; i++) {
		DescriptorTable &DescTable = DescTables[i];
		int Start = DescTable.Start;
//		int Num = DescTable.End +1 - Start;
		int Num = DescTable.TableSize - Start;
		int Slot = DescTable.RootSlot;
		if (DescTable.Dirty) {
			D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->StageDescriptors(DescTable.Handles + Start, Start, Num);
			if (!handle.ptr) {
				// need new descripterheap
				return false;
			} else {
				CommandList->SetGraphicsRootDescriptorTable(Slot, handle);
			}
			// debug
			printf("set texture table %d:", DescTable.RootSlot);
			for (int i = 0; i < DescTable.TableSize; i++) {
				printf("%d,", DescTable.ResourceId[i]);
			}
			printf("\n");
			// refresh cache
			DescTable.Dirty = 0;
			DescTable.RootSlot = i + 4;
			DescTable.Start = MAX_DESC_TABLE_SIZE;
			DescTable.End = -1;
			for (int n = 0; n < DescTable.TableSize; n++) {
				DescTable.ResourceId[n] = -1;
				DescTable.Handles[n] = NullHandle;
			}
		}
	} 
	//for (int i = 0; i < 4; i++) {
	//	DescriptorTable &DescTable = DescTables[i];
	//	DescTable.Dirty = 0;
	//	DescTable.Start = 
	//}
	// flush constants
	for (int i = 0; i < 32; i++) {
		if (Constants[i].Dirty) {
			CommandList->SetGraphicsRootConstantBufferView(Constants[i].RootSlot, Constants[i].desc.BufferLocation);
			Constants[i].Dirty = 0;
		}
	}
	return true;
}
