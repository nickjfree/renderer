#ifndef __RAYTRACING_SCENE__
#define __RAYTRACING_SCENE__



#include "Container/Vector.h"
#include "Structs.h"
#include "CommandContext.h"


namespace D3D12API {

	class RaytracingScene
	{
	private:
		// ratracing instances. to generate top level AS build info
		Vector<R_RAYTRACING_INSTANCE> rtInstances;
		// UINT64 FenceValue for top level as
		UINT64 SceneFenceValue_;
	public:
		RaytracingScene();
		~RaytracingScene();

		// build scene
		UINT64 BuildTopLevelAccelerationStructure(CommandContext * cmdContext);
		// build bottom level as
		UINT64 BuildBottomLevelAccelerationStructure(CommandContext* cmdContext);
		// wait for scene
		UINT64 WaitScene(CommandContext* GraphicContext);

	};



} // D3D12API
#endif 