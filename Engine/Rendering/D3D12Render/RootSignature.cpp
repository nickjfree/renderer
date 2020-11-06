#include "RootSignature.h"
#include <stdio.h>


using namespace D3D12API;


RootSignature::RootSignature(ID3D12Device* Device_, D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters,
	D3D12_CPU_DESCRIPTOR_HANDLE NullHandle_, D3D12_CPU_DESCRIPTOR_HANDLE NullUAVHandle_, bool Local)
	: NumCachedTables(0), NumConstantBuffers(0) {
	Device = Device_;
	NullHandle = NullHandle_;
	NullUAVHandle = NullUAVHandle_;
	Local_ = Local;
	InitRootSignature(rootParameters, numRootParameters);
}



RootSignature::~RootSignature() {
}

void RootSignature::InitRootSignature(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters) {
	
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSig(numRootParameters, rootParameters, 0, 0, Local_?D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE:D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob* pSerializedRootSig;
	ID3DBlob* pError;
	HRESULT result = D3D12SerializeVersionedRootSignature(&RootSig, &pSerializedRootSig, &pError);
	result = Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
		pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&this->RootSig));
	// init slot mapping
	InitMapping(rootParameters, numRootParameters);
	// InitCache(rootParameters, numRootParameters);
}

// init mapping
void RootSignature::InitMapping(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters) {
	int i = 0;
	int offset = 0;
	int CacheSlot = 0;

	for (int i = 0; i < numRootParameters; i++) {
		
		if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			// tables
			if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
				// srv tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					Textures[slot].CacheSlot = CacheSlot;
					Textures[slot].Offset = index;
					Textures[slot].RootSlot = i;
				}
				// init table cache
				InitDescriptorTableCache(CacheSlot, i, &rootParameters[i]);
				// increment cached tables count
				NumCachedTables++;
			}
			else if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
				// uva tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					UAVs[slot].CacheSlot = CacheSlot;
					UAVs[slot].Offset = index;
					UAVs[slot].RootSlot = i;
				}
				// init table cache
				InitDescriptorTableCache(CacheSlot, i, &rootParameters[i]);
				// increment cached tables count
				NumCachedTables++;
			}
			else if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
				// sampler tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					// samplers don't use cache
					Samplers[slot].CacheSlot = 0;
					Samplers[slot].Offset = index;
					Samplers[slot].RootSlot = i;
				}
				// init table cache
				InitDescriptorTableCache(CacheSlot, i, &rootParameters[i]);
			}
			// increase descriptor table index
			CacheSlot++;
		} else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			// constants buffer view
			auto slot = rootParameters[i].Descriptor.ShaderRegister;
			Constants[slot].RootSlot = i;
			Constants[slot].Dirty = 0;
			// increment constant buffer view count
			NumConstantBuffers++;
		}

	}


	//// textures
	//while (i <= 8) {
	//	Textures[i].CacheSlot = 0;
	//	Textures[i].Offset = offset++;
	//	Textures[i].RootSlot = 6;
	//	i++;
	//}

	//offset = 0;
	//while (i <= 13) {
	//	Textures[i].CacheSlot = 1;
	//	Textures[i].Offset = offset++;
	//	Textures[i].RootSlot = 7;
	//	i++;
	//}
	//offset = 0;
	//while (i <= 20) {
	//	Textures[i].CacheSlot = 2;
	//	Textures[i].Offset = offset++;
	//	Textures[i].RootSlot = 8;
	//	i++;
	//}
	//// uavs
	//i = 0;
	//offset = 0;
	//while (i <= 8) {
	//	UAVs[i].CacheSlot = 3;
	//	UAVs[i].Offset = offset++;
	//	UAVs[i].RootSlot = 9;
	//	i++;
	//}
	//// samplers
	//i = 0;
	//offset = 0;
	//while (i <= 2) {
	//	Samplers[i].CacheSlot = 0;
	//	Samplers[i].Offset = offset++;
	//	Samplers[i].RootSlot = 10;
	//	i++;
	//}
	//// constant
	//i = 0;
	//while (i <= 6) {
	//	Constants[i].RootSlot = i;
	//	Constants[i].Dirty = 0;
	//	i++;
	//}
}


void RootSignature::InitDescriptorTableCache(int CacheSlot, int rootParameterIndex, D3D12_ROOT_PARAMETER1* rootParameter) {
	// get cache
	DescriptorTable& Table = DescTables[CacheSlot];
	// init descriptor table cache
	Table.Dirty = 0;
	Table.RootSlot = rootParameterIndex;
	Table.Start = MAX_DESC_TABLE_SIZE;
	Table.End = -1;
	Table.DescriptorType = rootParameter->DescriptorTable.pDescriptorRanges[0].RangeType;
	// init all descriptor handle to null
	for (int n = 0; n < MAX_DESC_TABLE_SIZE; n++) {
		Table.ResourceId[n] = -1;
		Table.Handles[n] = NullHandle;
	}
	Table.TableSize = rootParameter->DescriptorTable.pDescriptorRanges[0].NumDescriptors;
}

//// init Cache
//void RootSignature::InitCache(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters) {
//	for (int i = 0; i < 5; i++) {
//		DescriptorTable& DescTable = DescTables[i];
//		DescTable.Dirty = 1;
//		DescTable.RootSlot = i + 6;
//		DescTable.Start = MAX_DESC_TABLE_SIZE;
//		DescTable.End = -1;
//		for (int n = 0; n < MAX_DESC_TABLE_SIZE; n++) {
//			DescTable.ResourceId[n] = -1;
//			DescTable.Handles[n] = NullHandle;
//		}
//	}
//	DescTables[0].TableSize = 9;
//	DescTables[1].TableSize = 5;
//	DescTables[2].TableSize = 7;
//	DescTables[3].TableSize = 8;
//	DescTables[4].TableSize = 3;
//
//	TotalTableSize = 21;
//}

// set texture
bool RootSignature::SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	DescTableSlot TableSlot = Textures[slot];
	DescriptorTable& DescTable = DescTables[TableSlot.CacheSlot];
	int Offset = TableSlot.Offset;
	DescTable.Fresh[Offset] = 1;
	if (CD3DX12_CPU_DESCRIPTOR_HANDLE(DescTable.Handles[Offset]) != handle) {
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

bool RootSignature::SetUAV(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	DescTableSlot TableSlot = UAVs[slot];
	DescriptorTable& DescTable = DescTables[TableSlot.CacheSlot];
	int Offset = TableSlot.Offset;
	DescTable.Fresh[Offset] = 1;
	if (CD3DX12_CPU_DESCRIPTOR_HANDLE(DescTable.Handles[Offset]) != handle) {
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
bool RootSignature::SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC* ConstBufferView) {
	Constants[slot].constDesc = *ConstBufferView;
	Constants[slot].Dirty = 1;
	return true;
}

// set sampler
bool RootSignature::SetSamplerTable(ID3D12GraphicsCommandList* CommandList, D3D12_GPU_DESCRIPTOR_HANDLE handle) {
	CommandList->SetGraphicsRootDescriptorTable(DescTables[4].RootSlot, handle);
	CommandList->SetComputeRootDescriptorTable(DescTables[4].RootSlot, handle);
	return true;
}

// flush descriptors, constant bindings
bool RootSignature::Flush(ID3D12GraphicsCommandList* CommandList, DescriptorHeap* descHeap, RootSignatureFlushFlag flushFlag) {
	// flush texture bindings
	// get total need size
	int TotalTableSize = 0;
	for (int i = 0; i < NumCachedTables; i++) {
		DescriptorTable& DescTable = DescTables[i];
		if (DescTable.Dirty || StateInvalid ) {
			TotalTableSize += DescTable.TableSize;
		}
	}
	if (!descHeap->HasSpace(TotalTableSize)) {
		return false;
	}

	// flush tables, srvs and uavs
	for (int i = 0; i < NumCachedTables; i++) {
		DescriptorTable& DescTable = DescTables[i];
		int Slot = DescTable.RootSlot;
		if (DescTable.Dirty || StateInvalid) {
			// resource barrier flushed, descriptor heap changed.  handles in the cache is invalid. we must set them to null
			if (DescTable.DescriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
				// unbind srv
				for (int n = 0; n < DescTable.TableSize; n++) {
					if (!DescTable.Fresh[n]) {
						DescTable.ResourceId[n] = -1;
						DescTable.Handles[n] = NullHandle;
					}
				}
			} else if (DescTable.DescriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
				// unbind uav
				for (int n = 0; n < DescTable.TableSize; n++) {
					if (!DescTable.Fresh[n]) {
						DescTable.ResourceId[n] = -1;
						DescTable.Handles[n] = NullUAVHandle;
					}
				}
			}
			D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->StageDescriptors(DescTable.Handles, 0, DescTable.TableSize);
			//D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->StageDescriptors(Start, StartPad, NumToCopy);
			if (!handle.ptr) {
				// need new descripterheap
				return false;
			}
			if (flushFlag & RootSignatureFlushFlag::ROOT_SIGNATURE_FLUSH_COMPUTE) {
				CommandList->SetComputeRootDescriptorTable(Slot, handle);
			}
			if (flushFlag & RootSignatureFlushFlag::ROOT_SIGNATURE_FLUSH_GRAPHIC) {
				CommandList->SetGraphicsRootDescriptorTable(Slot, handle);
			}
			// refresh cache
			DescTable.Dirty = 0;
			for (int n = 0; n < DescTable.TableSize; n++) {
				DescTable.Fresh[n] = 0;
			}
		}
	}
	// flush constants
	for (int i = 0; i < NumConstantBuffers; i++) {
		if (Constants[i].Dirty) {
			if (flushFlag & RootSignatureFlushFlag::ROOT_SIGNATURE_FLUSH_COMPUTE) {
				CommandList->SetComputeRootConstantBufferView(Constants[i].RootSlot, Constants[i].constDesc.BufferLocation);
			}
			if (flushFlag & RootSignatureFlushFlag::ROOT_SIGNATURE_FLUSH_GRAPHIC) {
				CommandList->SetGraphicsRootConstantBufferView(Constants[i].RootSlot, Constants[i].constDesc.BufferLocation);
			}
			Constants[i].Dirty = 0;
		}
	}
	// flushed, mark valid again
	StateInvalid = false;
	return true;
}

bool D3D12API::RootSignature::FlushSBT(DescriptorHeap* descHeap, ShaderRecord* Record)
{
	// flush texture bindings
// get total need size
	int TotalTableSize = 0;
	for (int i = 0; i < NumCachedTables; i++) {
		DescriptorTable& DescTable = DescTables[i];
		if (DescTable.Dirty) {
			TotalTableSize += DescTable.TableSize;
		}
	}
	if (!descHeap->HasSpace(TotalTableSize)) {
		return false;
	}

	// flush tables, srvs and uavs
	for (int i = 0; i < NumCachedTables; i++) {
		DescriptorTable& DescTable = DescTables[i];
		int Start = DescTable.Start;
		int Num = DescTable.TableSize - Start;
		int Slot = DescTable.RootSlot;
		if (DescTable.Dirty || prevRtDescHeap != descHeap) {
			// for local root signature the handles in the cache is invalid. we must set them to null
			if (DescTable.DescriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
				// unbind srv
				for (int n = 0; n < DescTable.TableSize; n++) {
					if (!DescTable.Fresh[n]) {
						DescTable.ResourceId[n] = -1;
						DescTable.Handles[n] = NullHandle;
					}
				}
			}
			else if (DescTable.DescriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
				// unbind uav
				for (int n = 0; n < DescTable.TableSize; n++) {
					if (!DescTable.Fresh[n]) {
						DescTable.ResourceId[n] = -1;
						DescTable.Handles[n] = NullUAVHandle;
					}
				}
			}
			// flush all the handles in table
			DescTable.Start = 0;
			DescTable.End = DescTable.TableSize - 1;
			D3D12_CPU_DESCRIPTOR_HANDLE* Start = DescTable.Handles + DescTable.Start;
			unsigned int NumToCopy = DescTable.End - DescTable.Start + 1;
			unsigned int StartPad = DescTable.Start;
			D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->StageDescriptors(DescTable.Handles, 0, DescTable.TableSize);
			//D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->StageDescriptors(Start, StartPad, NumToCopy);
			if (!handle.ptr) {
				// need new descripterheap
				return false;
			}
			else {
				// set table pointers in the sbt
				Record->Params[Slot] = handle.ptr;
				// set prevraytracinghandle
				DescTable.PrevRaytracingTableHandle = handle;
				// remember this descripter heap
				prevRtDescHeap = descHeap;
			}
			// refresh cache
			DescTable.Dirty = 0;
			DescTable.Start = DescTable.TableSize;
			DescTable.End = -1;
			for (int n = 0; n < DescTable.TableSize; n++) {
				DescTable.Fresh[n] = 0;
			}
		}
		else {
			Record->Params[Slot] = DescTable.PrevRaytracingTableHandle.ptr;
		}
	}

	// flush constants
	for (int i = 0; i < NumConstantBuffers; i++) {
		if (Constants[i].Dirty) {
			// CommandList->SetGraphicsRootConstantBufferView(Constants[i].RootSlot, Constants[i].desc.BufferLocation);
			Record->Params[Constants[i].RootSlot] = Constants[i].constDesc.BufferLocation;
			Constants[i].Dirty = 0;
		}
	}
	return true;
}
