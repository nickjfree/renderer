#ifndef __COMMAND_CONTEXT__
#define __COMMAND_CONTEXT__

#include <d3d12.h>
#include "d3dx12.h" 
#include "Container\LinearBuffer.h"
#include "Container\RecyclePool.h"
#include "Container\Vector.h"
#include "Container\List.h"
#include "Tasks\Mutex.h"
#include <vector>


namespace D3D12API {



    class CommandContext {

    private:
        // commandcontext submitted. waiting on fence
        static List<CommandContext> Retired[4];
        // commancontext free to reuse
        static Vector<CommandContext *> Free[4];
        // command allocator
        ID3D12CommandAllocator * CommandAllocator;
        // command list
        ID3D12GraphicsCommandList  * CommandList;
        // Mutex
        static Mutex mutex;
    protected:
        // command type
        D3D12_COMMAND_LIST_TYPE  Type;
        // fence value when retire
        UINT64 FenceValue;
        // D3D12Device
        ID3D12Device * Device;
    public:
        CommandContext();
        CommandContext(ID3D12Device * Device_, D3D12_COMMAND_LIST_TYPE type);
        virtual ~CommandContext();
        // alloc a new command context
        static CommandContext * Alloc(ID3D12Device * Device, D3D12_COMMAND_LIST_TYPE type);
        // retired command list
        void Retire(UINT64 FenceValue);
        // reset command list
        void Reset();
        // flush command list
        UINT64 Flush(bool WaitForFence);
        // finish command list and retire
        UINT64 Finish(bool WaitForFence);
        // set descriptorheaps. typicaly. 2 frame buffers, 2-3 descriptor heaps
        void SetDescriptorHeaps(ID3D12DescriptorHeap * Heaps);
        // get commandlist
        ID3D12GraphicsCommandList  * GetGraphicsCommandList() { return CommandList; }
        // init texture
        void InitializeTexture(ID3D12Resource * DestResource, std::vector<D3D12_SUBRESOURCE_DATA>& subresources, ID3D12Resource ** Upload);
        // init vertext buffers
        void InitializeVetexBuffer(ID3D12Resource * DestResource, void * Buffer, unsigned int Size, ID3D12Resource ** Upload);
        // init index buffers
        void InitializeIndexBuffer(ID3D12Resource * DestResource, void * Buffer, unsigned int Size, ID3D12Resource ** Upload);
        // init uav buffers
        void InitializeUnorderedAccessBuffer(ID3D12Resource * DestResource, void * Buffer, unsigned int Size, ID3D12Resource ** Upload);
        // get commandlist
        ID3D12CommandList * GetCommandList();
    };

}

#endif
