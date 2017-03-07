#include "RootSignature.h"


using namespace D3D12API;


RootSignature::RootSignature(ID3D12Device * Device_) {
	Device = Device_;
	InitRootSignature();
}


RootSignature::~RootSignature() {
}

void RootSignature::InitRootSignature() {
	CD3DX12_DESCRIPTOR_RANGE1 DescRange[4];

	// texture materials src t 0-8
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 9, 0);
	// texture g-buffer srv t 9-13
	DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 9);
	// texture misc srv t 14-20
	DescRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 14);
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

}

// set texture
void RootSignature::SetTexture(int slot, int id, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	DescTableSlot TableSlot = Textures[slot];

}

// set constant
void RootSignature::SetConstantBuffer(int slot, D3D12_CONSTANT_BUFFER_VIEW_DESC * ConstBufferView) {

}

// set sampler
void RootSignature::SetSampler(int slot, D3D12_CPU_DESCRIPTOR_HANDLE handle) {

}

// flush descriptors, constant bindings
void RootSignature::Flush(ID3D12CommandList * CommandList, DescriptorHeap * descHeap) {

}
