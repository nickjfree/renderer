#include "ShaderLibrary.h"
#include "D3Dcompiler.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/DxilContainer/DxilContainer.h"


USING_ALLOCATER(ShaderLibrary);

static dxc::DxcDllSupport dxcDllSupport;


ShaderLibrary::ShaderLibrary(Context* context): GPUResource(context)
{
	ResourceType = R_SHADER_LIBRARY;
}

int ShaderLibrary::OnSerialize(Deserializer& deserializer)
{
	// reflect shader. 
	ReflectShader(deserializer.Raw(), deserializer.Length());
	return 0;
}


int ShaderLibrary::AddFunction(char* Name) {
	
	auto Function = Name + 2;

	if (strstr(Function, "ClosestHit") == Function) {
		ClosestHitShaderName = L"ClosestHit";
	}
	if (strstr(Function, "Raygen") == Function) {
		RaygenShaderName = L"Raygen";
	}
	if (strstr(Function, "Miss") == Function) {
		MissShaderName = L"Miss";
	}
	if (strstr(Function, "Intersection") == Function) {
		IntersectionShaderName = L"Intersection";
	}
	if (strstr(Function, "AnyHit") == Function) {
		AnyHitShaderName = L"AnyHit";
	}
	return 0;
}

int ShaderLibrary::ReflectShader(void* Shader, unsigned int Size) 
{
	/*
		shader reflection to get contant buffer info and input signature info
	*/
	dxcDllSupport.Initialize();

	IDxcLibrary* pLibrary = NULL;
	dxcDllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary);
	IDxcBlobEncoding* pBlob = NULL;
	pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)Shader, Size, 0, &pBlob);

	ID3D12LibraryReflection* Reflector;
	IDxcContainerReflection* pReflection;
	UINT32 shaderIdx;
	dxcDllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection);
	pReflection->Load(pBlob);
	pReflection->FindFirstPartKind(hlsl::DFCC_DXIL, &shaderIdx);
	pReflection->GetPartReflection(shaderIdx, __uuidof(ID3D12LibraryReflection), (void**)&Reflector);
	// get shader desc
	D3D12_LIBRARY_DESC desc;
	Reflector->GetDesc(&desc);

	// loop shader functions
	for (auto i = 0; i < desc.FunctionCount; i++) {
		auto functionReflection = Reflector->GetFunctionByIndex(i);
		D3D12_FUNCTION_DESC func{};
		functionReflection->GetDesc(&func);

		// add function name
		AddFunction((char*)func.Name);
		// constans
		for (UINT i = 0; i < func.ConstantBuffers; i++) {
			D3D12_SHADER_BUFFER_DESC Description;
			ID3D12ShaderReflectionConstantBuffer* ConstBuffer = functionReflection->GetConstantBufferByIndex(i);
			ConstBuffer->GetDesc(&Description);
			// get constant buffer bind
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			functionReflection->GetResourceBindingDescByName(Description.Name, &bindDesc);
			if (bindDesc.Type != D3D_SIT_CBUFFER) {
				// ignore rw uav buffers 
				continue;
			}
			// store constant buffer info
			ConstantBuffer cb;
			cb.Name = (char*)Description.Name;
			cb.Size = Description.Size;
			cb.Slot = bindDesc.BindPoint;
			int IsArray = 0;
			if (!memcmp(cb.Name.ToStr(), "Array", 5)) {
				IsArray = 1;
			}
			cb.IsArray = IsArray;
			Constants.PushBack(cb);
			for (UINT j = 0; j < Description.Variables; j++) {   // Get the variable description and store it   
				ID3D12ShaderReflectionVariable* Variable = ConstBuffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC var_desc;
				Variable->GetDesc(&var_desc);
				ID3D12ShaderReflectionType* pType = Variable->GetType();
				D3D12_SHADER_TYPE_DESC type_desc;
				pType->GetDesc(&type_desc);
				// store shader parameter info
				ShaderParameter sp;
				sp.Offset = var_desc.StartOffset;
				sp.Slot = cb.Slot;
				sp.Name = (char*)var_desc.Name;
				sp.Size = var_desc.Size;
				sp.Constant = cb.Name;
				sp.IsArray = IsArray;
				Parameters.PushBack(sp);
			}
		}
		// bindings
		D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
		for (auto n = 0; n < func.BoundResources; n++) {
			functionReflection->GetResourceBindingDesc(n, &bindDesc);
			//	D3D12_SHADER_INPUT_BIND_DESC bind_desc;
				
			// get tlas
			if (bindDesc.Type == 12) {
				RtSceneUnit rt;
				rt.Name = (char*)bindDesc.Name;
				rt.Slot = bindDesc.BindPoint;
				RtSceneUnits.PushBack(rt);
			}

			// get textures
			if (bindDesc.Type == D3D_SIT_TEXTURE) {
				// store constant buffer info
				TextureUnit tu;
				tu.Name = (char*)bindDesc.Name;
				tu.Slot = bindDesc.BindPoint;
				TextureUnits.PushBack(tu);
			}
			// get buffers. tbuffer. typed and structured buffer.
			if (bindDesc.Type == D3D_SIT_TBUFFER || bindDesc.Type == D3D_SIT_STRUCTURED || bindDesc.Type == D3D_SIT_BYTEADDRESS) {
				BufferUnit bu;
				bu.Name = (char*)bindDesc.Name;
				bu.Slot = bindDesc.BindPoint;
				BufferUnits.PushBack(bu);
			}
			// get rwbuffers. unordered textture(buffers).
			if (bindDesc.Type == D3D_SIT_UAV_RWTYPED
				|| bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED
				|| bindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS
				|| bindDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED
				|| bindDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED
				|| bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER) {

				if (bindDesc.Dimension == D3D_SRV_DIMENSION_TEXTURE2D) {
					RWTextureUnit rwtu;
					rwtu.Name = (char*)bindDesc.Name;
					rwtu.Slot = bindDesc.BindPoint;
					// RenderPass->RWTextureUnits.PushBack(rwtu);
					RWTextureUnits.PushBack(rwtu);
				}
				else {
					RWBufferUnit rwbu;
					rwbu.Name = (char*)bindDesc.Name;
					rwbu.Slot = bindDesc.BindPoint;
					RWBufferUnits.PushBack(rwbu);
				}
			}	
		}
	}
	// cleanup
	Reflector->Release();
	pReflection->Release();
	pBlob->Release();
	pLibrary->Release();
	return 0;
}

int ShaderLibrary::OnLoadComplete(Variant& Parameter)
{
	return 0;
}

int ShaderLibrary::OnCreateComplete(Variant& Parameter)
{
	// gen hitgroup name
	wchar_t HitGroupName[32];
	swprintf_s(HitGroupName, L"HitGroup_%S", File.ToStr());
	// create the shader
	renderinterface->CreateRayTracingShader(DeSerial.Raw(), DeSerial.Length(), RaygenShaderName, MissShaderName, 
		HitGroupName, ClosestHitShaderName, AnyHitShaderName, IntersectionShaderName);

	// release 
	DeSerial.Release();
	return 0;
}

int ShaderLibrary::Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext* Context)
{
	return 0;
}

