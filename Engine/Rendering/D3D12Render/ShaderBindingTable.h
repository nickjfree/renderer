#ifndef __SHADER_BINDING_TABLE__
#define __SHADER_BINDING_TABLE__

#include "ReuseHeap.h"
#include "CommandContext.h"
#include "Container/Vector.h"
#include "Structs.h"


namespace D3D12API {


	/*
		Shder records
	*/
	typedef struct ShaderRecord {
		// 8 bytes indentifier
		D3DShaderIdetifier ShaderIdentifier;
		// 8 root parameters
		UINT64 Params[8];
	}ShaderRecord;

	constexpr int MAX_RAY_TYPES = 3;
	constexpr unsigned int RAYGEN_TABLE_OFFSET = 0;
	constexpr unsigned int MISS_TABLE_OFFSET = MAX_RAY_TYPES * sizeof(ShaderRecord);
	constexpr unsigned int HITGROUP_TABLE_OFFSET = 2 * MAX_RAY_TYPES * sizeof(ShaderRecord);
	constexpr unsigned int RAYGEN_TABLE_SIZE = MAX_RAY_TYPES * sizeof(ShaderRecord);
	constexpr unsigned int MISS_TABLE_SIZE = MAX_RAY_TYPES * sizeof(ShaderRecord);

	/*
		shader binding table
	*/
	class ShaderBindingTable
	{
	private:
		// SBT data
		Vector<ShaderRecord> HitGroup;
		// raygen table
		ShaderRecord RayGen[MAX_RAY_TYPES];
		// miss table
		ShaderRecord Miss[MAX_RAY_TYPES];
		// upload heap for sbt
		ReuseHeap* UploadBuffer = nullptr;
		// gpu buffer for sbt
		ReuseHeap* GPUBuffer = nullptr;
		// buffer size
		UINT Size_;
		// current index
		UINT Current = 0;
		// GPU Address
		D3D12_DISPATCH_RAYS_DESC RayDesc;
	public:
		ShaderBindingTable(ID3D12Device* Device);
		~ShaderBindingTable();

		// ensure size
		void EnsureSize(UINT Size);
		// alloc record
		ShaderRecord* AllocRecord(int MaterialId);
		// update ray
		void UpdateRay(int rayIndex, D3DShaderIdetifier& raygenIdentifier, D3DShaderIdetifier& missIdentifier);
		// reset
		void Reset();
		// stage
		void Stage(CommandContext* cmdContext);
		// get ray desc
		const D3D12_DISPATCH_RAYS_DESC& GetDesc() { return RayDesc; }
	};


}

#endif