#ifndef __RAYTRACING_SCENE__
#define __RAYTRACING_SCENE__



#include "Container/Vector.h"
#include "Structs.h"
#include "CommandContext.h"
#include "ReuseHeap.h"

namespace D3D12API {

	typedef struct BottomLevelAsRebuildDesc {
		// bottoem level as
		D3DBottomLevelAS* Blas;
		// buffer 
		D3DBuffer* Buffer;
		// build info
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
		// frame index, resource index to build. one blas per frame. for async build
		int FrameIndex;
	}BottomLevelAsRebuildDesc;


	class RaytracingScene
	{
	private:
		// device
		ID3D12Device* Device_;
		ID3D12Device5* rtDevice_;
		// instance buffer
		ReuseHeap * InstancesBuffer;
		// top level as resource
		ReuseHeap* TopLevelAS;
		// top level scratch buffer
		ReuseHeap* TopLevelScratch;
		// bottom level as to build
		// D3D12_RAYTRACING_INSTANCE_DESC
		Vector<BottomLevelAsRebuildDesc> BottomLevelDesc;
		// top level instances
		Vector<D3D12_RAYTRACING_INSTANCE_DESC> InstanceDesc;
		// translate resource barriars
		Vector<CD3DX12_RESOURCE_BARRIER> ResourceBarriers;
		// UINT64 FenceValue for top level as. also fencevalue for retired graphic context
		UINT64 SceneFenceValue_;
	protected:
		// rtscene which are used be previous frame
		static List<RaytracingScene> RetiredScene;
	public:
		RaytracingScene(ID3D12Device* Device);
		~RaytracingScene();

		// alloc new raytracing scene
		static RaytracingScene* Alloc(ID3D12Device* Device);
		// retire raytracing scene
		void Retire(UINT64 FenceValue);
		// build scene
		UINT64 BuildTopLevelAccelerationStructure(CommandContext * cmdContext);
		// build bottom level as
		UINT64 BuildBottomLevelAccelerationStructure(CommandContext* cmdContext, UINT64 GraphicFenceValue);
		// wait for scene
		UINT64 WaitScene(CommandContext* GraphicContext);
		// add instance
		void AddInstance(ID3D12Resource* BottomLevelAs, UINT InstanceID, UINT Flags, Matrix4x4& Tansform);
		// add bottomlevel as for rebuild
		void RebuildBottomLevelAs(D3DBottomLevelAS* Blas, D3DBuffer * Buffer, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& buildDesc, int FrameIndex);

	};



} // D3D12API
#endif 