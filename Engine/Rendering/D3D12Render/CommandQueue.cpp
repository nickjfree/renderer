#include "CommandQueue.h"
#include "Tasks\ThreadLocal.h"


using namespace D3D12API;


CommandQueue::CommandQueue(ID3D12Device * Device_, D3D12_COMMAND_LIST_TYPE Type) {
	Device = Device_;
	D3D12_COMMAND_QUEUE_DESC Desc = {};
	Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	Desc.NodeMask = 0;
	Desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	Desc.Type = Type;
	HRESULT result = Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&CmdQueue));

	// create fences, event, init fenvevalue
	for (int i = 0; i < MAX_THREAD; i++) {
		Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
		FenceValue = 1;
		// event
		FenceEvent[i] = CreateEvent(0, 0, 0, 0);
	}
}

CommandQueue::~CommandQueue() {
}

void CommandQueue::Wait(UINT64 FenceValue_) {
	// get thread local index
	int Index = (int)ThreadLocal::GetThreadLocal();
	// fencevalue is the fencevalue to wait for
	UINT64 FenceToWait = FenceValue_;
	Fence->SetEventOnCompletion(FenceToWait, FenceEvent[Index]);
	WaitForSingleObject(FenceEvent[Index], -1);
}

bool CommandQueue::FenceComplete(UINT64 FenceValue_) {
	UINT64 Complete = Fence->GetCompletedValue();
	if (FenceValue_ <= Complete) {
		return true;
	} else {
		return false;
	}
}

UINT64 CommandQueue::ExecuteCommandList(int Num, ID3D12CommandList ** CommandLists) {
	CmdQueue->ExecuteCommandLists(Num, CommandLists);
	QueueLock.Acquire();
	UINT64 CurrentFence = InterlockedIncrement(&FenceValue);
	CmdQueue->Signal(Fence, CurrentFence);
	QueueLock.Release();
	return CurrentFence;
}