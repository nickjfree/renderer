#ifndef __SHADER_LIBRARY__
#define __SHADER_LIBRARY__


#include "GPUResource.h"
#include "RenderDesc.h"
#include "Container\Dict.h"
#include "Core\Str.h"
#include "Container\Vector.h"
#include "Core\Context.h"
#include "Resource\XMLParser.h"
#include "IO\FileLoader.h"
#include "BatchCompiler.h"
#include "Shader.h"


/*
	shader library
*/
class ShaderLibrary: public GPUResource
{
	OBJECT(ShaderLibrary);
	BASEOBJECT(ShaderLibrary);
	LOADEROBJECT(FileLoader);
	DECLARE_ALLOCATER(ShaderLibrary);

private:
	// textures
	Vector<TextureUnit> TextureUnits;
	// buffers
	Vector<BufferUnit> BufferUnits;
	// rw buffers
	Vector<RWBufferUnit> RWBufferUnits;
	// rw textures
	Vector<RWTextureUnit> RWTextureUnits;
	// rtScene
	Vector<RtSceneUnit> RtSceneUnits;
	// constant buffers
	Vector<ConstantBuffer> Constants;
	// shader patameters
	Vector<ShaderParameter> Parameters;
	// shader names
	wchar_t* RaygenShaderName = nullptr;
	wchar_t* MissShaderName = nullptr;
	wchar_t* ClosestHitShaderName = nullptr;
	wchar_t* AnyHitShaderName = nullptr;
	wchar_t* IntersectionShaderName = nullptr;

private:
	int ReflectShader(void* Shader, unsigned int Size);
	int AddFunction(char* Name);
	template <class ... T> Variant* GetParameter(String& Name, RenderContext* Context, T& ... parameterList);
public:
	ShaderLibrary(Context* context);

	virtual int OnSerialize(Deserializer& deserializer);
	virtual int OnLoadComplete(Variant& Parameter);
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext* Context);
	void GetLocalResourceBindings(Dict& MaterialParam, Dict& ObjectParameter, RenderContext* Context, R_RESOURCE_BINDING * bindings, int* count);
	template <class ...T> void Apply(RenderCommandContext* cmdContext, RenderContext* Context, T& ...parameterList);
};

template <class ... T> Variant* ShaderLibrary::GetParameter(String& Name, RenderContext* Context, T& ... parameterList) {
	// list of parameters to try
	Dict* tries[] = { &parameterList... };

	for (auto i = 0; i < sizeof...(parameterList); ++i) {
		auto iter = tries[i]->Find(Name);
		if (iter != tries[i]->End()) {
			return &(*iter).Value;
		}
	}
	return rendercontext->GetResource(Name);
}


template <class ...T> void ShaderLibrary::Apply(RenderCommandContext* cmdContext, RenderContext* Context, T& ...parameterList)
{
	int Compiled = 0;
	// textures (SRV)
	int texture_units = TextureUnits.Size();
	for (int i = 0; i < texture_units; i++) {
		TextureUnit* unit = &TextureUnits[i];
		if (unit->Space == 0) {
			Variant* Value = GetParameter(unit->Name, Context, parameterList...);
			if (Value) {
				int id = Value->as<int>();
				cmdContext->SetSRV(unit->Slot, id);
			}
		}
	}
	// buffers (SRV)
	int buffer_units = BufferUnits.Size();
	for (int i = 0; i < buffer_units; i++) {
		BufferUnit* unit = &BufferUnits[i];
		if (unit->Space == 0) {
			Variant* Value = GetParameter(unit->Name, Context, parameterList...);
			if (Value) {
				int id = Value->as<int>();
				cmdContext->SetSRV(unit->Slot, id);
			}
		}
	}
	// rwbuffers (UAV)
	int rwbuffer_units = RWBufferUnits.Size();
	for (int i = 0; i < rwbuffer_units; i++) {
		RWBufferUnit* unit = &RWBufferUnits[i];
		if (unit->Space == 0) {
			Variant* Value = GetParameter(unit->Name, Context, parameterList...);
			if (Value) {
				int id = Value->as<int>();
				cmdContext->SetUAV(unit->Slot, id);
			}
		}
	}
	// rwtextures (UAV)
	int rwtexture_units = RWTextureUnits.Size();
	for (int i = 0; i < rwtexture_units; i++) {
		RWTextureUnit* unit = &RWTextureUnits[i];
		if (unit->Space == 0) {
			Variant* Value = GetParameter(unit->Name, Context, parameterList...);
			if (Value) {
				int id = Value->as<int>();
				cmdContext->SetUAV(unit->Slot, id);
			}
		}
	}
	// tlas (SRV)
	int rtscene_units = RtSceneUnits.Size();
	for (int i = 0; i < rtscene_units; i++) {
		auto unit = &RtSceneUnits[i];
		cmdContext->SetRaytracingScene(unit->Slot);
	}
	// constants
	int parameters = Parameters.Size();
	for (int i = 0; i < parameters; i++) {
		ShaderParameter* parameter = &Parameters[i];
		Variant* Value = GetParameter(parameter->Name, Context, parameterList...);
		if (Value) {
			if (!parameter->IsArray) {
				cmdContext->UpdateConstantBuffer(parameter->Slot, parameter->Offset, Value, parameter->Size);
			} else {
				ShaderParameterArray& Array = Value->as<ShaderParameterArray>();
				cmdContext->UpdateConstantBuffer(parameter->Slot, parameter->Offset, Array.Data, parameter->Size);
			}
		}
	}
	// update constants
	int numConstant = Constants.Size();
	for (int i = 0; i < numConstant; i++) {
		ConstantBuffer* cons = &Constants[i];
		cmdContext->SetConstantBuffer(cons->Slot, cons->Size);
	}
}


#endif
