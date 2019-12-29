#ifndef __RAYTRACING_SCENE__
#define __RAYTRACING_SCENE__



#include "Container/Vector.h"
#include "Structs.h"
#include "CommandContext.h"
#include "ReuseHeap.h"

namespace D3D12API {


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
		Vector<D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC> BottomLevelDesc;
		// top level instances
		Vector<D3D12_RAYTRACING_INSTANCE_DESC> InstanceDesc;

		// UINT64 FenceValue for top level as
		UINT64 SceneFenceValue_;
	public:
		RaytracingScene(ID3D12Device* Device);
		~RaytracingScene();

		// build scene
		UINT64 BuildTopLevelAccelerationStructure(CommandContext * cmdContext);
		// build bottom level as
		UINT64 BuildBottomLevelAccelerationStructure(CommandContext* cmdContext);
		// wait for scene
		UINT64 WaitScene(CommandContext* GraphicContext);

		// add instance
		void AddInstance(ID3D12Resource* BottomLevelAs, UINT InstanceID, UINT Flags, Matrix4x4& Tansform);

	};



} // D3D12API
#endif 