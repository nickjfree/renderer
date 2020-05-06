#ifndef __SHADER_BINDING_TABLE__
#define __SHADER_BINDING_TABLE__

#include "ReuseHeap.h"
#include "Container/Vector.h"


namespace D3D12API {

	/*
		Shder records
	*/
	typedef struct ShaderRecord {

	}ShaderRecord;

	/*
		shader bindingt table
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
		
	public:
		ShaderBindingTable(ID3D12Device* Device);
		~ShaderBindingTable();

		// ensure size
		void EnsureSize(UINT Size);
		
	};


}

#endif