#ifndef __SHADER_BINDING_TABLE__
#define __SHADER_BINDING_TABLE__

#include "ReuseHeap.h"
#include "CommandContext.h"
#include "Container/Vector.h"


namespace D3D12API {

	/*
		Shder records
	*/
	typedef struct ShaderRecord {
		// 8 bytes indentifier
		void* Identifier;
		// 8 root parameters
		UINT64 Params[8];
	}ShaderRecord;

	/*
		shader binding table
	*/
	class ShaderBindingTable
	{
	private:
		// SBT data
		Vector<ShaderRecord> Data;
		// upload heap for sbt
		ReuseHeap* UploadBuffer = nullptr;
		// gpu buffer for sbt
		ReuseHeap* GPUBuffer = nullptr;
		// buffer size
		UINT Size_;
		// current index
		UINT Current = 0;
	public:
		ShaderBindingTable(ID3D12Device* Device);
		~ShaderBindingTable();

		// ensure size
		void EnsureSize(UINT Size);
		// alloc record
		ShaderRecord* AllocRecord(int MaterialId);
		// reset
		void Reset();
		// stage
		void Stage(CommandContext* cmdContext);

	};


}

#endif