#include "CommandContext.h"
#include "CommandQueue.h"
#include <vector>
#include "D3D12Render.h"



using namespace D3D12API;
using namespace std;

Mutex CommandContext::mutex;

List<CommandContext> CommandContext::Retired[4];

Vector<CommandContext*> CommandContext::Free[4];

CommandContext::CommandContext() {
}

CommandContext::CommandContext(ID3D12Device* Device_, D3D12_COMMAND_LIST_TYPE type) {
	Device = Device_;
	Type = type;
	Device->CreateCommandAllocator(type, IID_PPV_ARGS(&CommandAllocator));
	Device->CreateCommandList(0, type, CommandAllocator, NULL, IID_PPV_ARGS(&CommandList));
}

CommandContext::~CommandContext() {
}


CommandContext* CommandContext::Alloc(ID3D12Device* Device, D3D12_COMMAND_LIST_TYPE type) {
	Vector<CommandContext*>& free = Free[type];
	List<CommandContext>& retired = Retired[type];
	CommandContext* context = NULL;
	D3D12Render* render = D3D12Render::GetRender();
	CommandQueue* Queue = render->GetQueue(type);
	mutex.Acquire();
	if (free.Size()) {
		context = free.PopBack();
	}
	else {
		bool Found = 0;
		for (auto Iter = retired.Begin(); Iter != retired.End(); Iter++) {
			context = *Iter;
			UINT64 FenceValue = context->FenceValue;
			if (Queue->FenceComplete(FenceValue)) {
				retired.Remove(Iter);
				Found = 1;
				break;
			}
		}
		if (!Found) {
			context = 0;
		}
	}
	mutex.Release();
	if (context) {
		context->Reset();
	}
	else {
		// new context
		context = new CommandContext(Device, type);
	}
	return context;
}

void CommandContext::Reset() {
	CommandAllocator->Reset();
	CommandList->Reset(CommandAllocator, NULL);
}

UINT64 CommandContext::Finish(bool WaitForFence) {
	D3D12Render* Render = D3D12Render::GetRender();
	CommandQueue* Queue = Render->GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	CommandList->Close();
	ID3D12CommandList* pCommandList[1] = { CommandList };
	FenceValue = Queue->ExecuteCommandList(1, pCommandList);
	if (WaitForFence) {
		Queue->Wait(FenceValue);
	}
	// put context to retired set
	CommandContext* Context = this;
	mutex.Acquire();
	Retired[Type].Insert(this);
	mutex.Release();
	return FenceValue;
}

UINT64 CommandContext::Flush(bool WaitForFence) {
	D3D12Render* Render = D3D12Render::GetRender();
	CommandQueue* Queue = Render->GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12CommandList* pCommandList[1] = { CommandList };
	CommandList->Close();
	FenceValue = Queue->ExecuteCommandList(1, pCommandList);
	if (WaitForFence) {
		Queue->Wait(FenceValue);
	}
	return FenceValue;
}


void CommandContext::InitializeTexture(ID3D12Resource* DestResource, std::vector<D3D12_SUBRESOURCE_DATA>& subresources, ID3D12Resource** Upload) {

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(DestResource, 0, static_cast<UINT>(subresources.size()));

	// Create the GPU upload buffer.
	ID3D12Resource* uploadHeap;
	HRESULT result = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap));

	UpdateSubresources(CommandList, DestResource, uploadHeap,
		0, 0, static_cast<UINT>(subresources.size()), subresources.data());
	// resource barrier
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DestResource,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	// return upload heap
	*Upload = uploadHeap;
}

void CommandContext::InitializeVetexBuffer(ID3D12Resource* DestResource, void* Buffer, unsigned int Size, ID3D12Resource** Upload) {
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(DestResource, 0, 1);

	// Create the CPU upload buffer.
	ID3D12Resource* uploadHeap;
	HRESULT result = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap));

	D3D12_SUBRESOURCE_DATA Data = {};
	Data.pData = Buffer;
	Data.RowPitch = Size;
	Data.SlicePitch = 1;
	UpdateSubresources(CommandList, DestResource, uploadHeap,
		0, 0, 1, &Data);
	// resource barrier
	auto state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		|D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		|D3D12_RESOURCE_STATE_COPY_SOURCE;
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DestResource,
		D3D12_RESOURCE_STATE_COPY_DEST, state));
	// return temp upload head
	*Upload = uploadHeap;
}

void CommandContext::InitializeIndexBuffer(ID3D12Resource* DestResource, void* Buffer, unsigned int Size, ID3D12Resource** Upload) {
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(DestResource, 0, 1);

	// Create the CPU upload buffer.
	ID3D12Resource* uploadHeap;
	HRESULT result = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap));

	D3D12_SUBRESOURCE_DATA Data = {};
	Data.pData = Buffer;
	Data.RowPitch = Size;
	Data.SlicePitch = 1;
	UpdateSubresources(CommandList, DestResource, uploadHeap,
		0, 0, 1, &Data);
	// resource barrier
	auto state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		| D3D12_RESOURCE_STATE_INDEX_BUFFER
		| D3D12_RESOURCE_STATE_COPY_SOURCE;
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DestResource,
		D3D12_RESOURCE_STATE_COPY_DEST, state));
	// return upload heap
	*Upload = uploadHeap;
}

void D3D12API::CommandContext::InitializeUnorderedAccessBuffer(ID3D12Resource* DestResource, void* Buffer, unsigned int Size, ID3D12Resource** Upload)
{
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(DestResource, 0, 1);

	// Create the CPU upload buffer.
	ID3D12Resource* uploadHeap;
	HRESULT result = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadHeap));

	D3D12_SUBRESOURCE_DATA Data = {};
	Data.pData = Buffer;
	Data.RowPitch = Size;
	Data.SlicePitch = 1;
	UpdateSubresources(CommandList, DestResource, uploadHeap,
		0, 0, 1, &Data);
	// resource barrier
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DestResource,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	// return upload heap
	*Upload = uploadHeap;
}



ID3D12CommandList* CommandContext::GetCommandList() {
	return CommandList;
}