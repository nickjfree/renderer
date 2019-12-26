#ifndef __COMMAND_QUEUE__
#define __COMMAND_QUEUE__

#include <d3d12.h>
#include "d3dx12.h"
#include "Structs.h"
#include "Tasks\Mutex.h"


namespace D3D12API {

	class CommandQueue {
	private:
		// queue
		ID3D12CommandQueue* CmdQueue;
		// type
		D3D12_COMMAND_LIST_TYPE Type_;
		// device
		ID3D12Device* Device;
		// fence
		ID3D12Fence* Fence;
		// fence value
		UINT64 FenceValue;
		// fence event handle
		HANDLE FenceEvent[MAX_THREAD];
		// mutex
		Mutex QueueLock;
	public:
		CommandQueue(ID3D12Device* Device, D3D12_COMMAND_LIST_TYPE Type);
		virtual ~CommandQueue();
		ID3D12CommandQueue* Get() { return CmdQueue; }
		// wait for fence
		void Wait(UINT64 FenceValue);
		// idle gpu for a thread
		void IdleGpu();
		// excute command list
		UINT64 ExecuteCommandList(int Num, ID3D12CommandList** CommandLists);
		// is fence complete
		bool FenceComplete(UINT64 FenceValue);
		// gpu wait
		void WaitQueue(CommandQueue * Queue, UINT64 FenceValue);
	};

}

#endif 

