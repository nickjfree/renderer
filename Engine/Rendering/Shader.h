#ifndef __SHADER_H__
#define __SHADER_H__


#include "GPUResource.h"
#include "RenderDesc.h"
#include "Container\Dict.h"
#include "Core\Str.h"
#include "Container\Vector.h"
#include "Core\Context.h"
#include "Resource\XMLParser.h"
#include "IO\FileLoader.h"
#include "BatchCompiler.h"

/*
	shader reflection helpers
*/
R_FORMAT GetFormat(int Mask, int Type);
int GetOffset(int Mask);

/*
	shader parameter
*/

typedef struct ShaderParameter {
	// variable name
	String Name;
	// constant buffer name
	String Constant;
	// ofsset in cbuffer
	unsigned int Offset;
	// slot index
	unsigned int Slot;
	// Size
	unsigned int Size;
	// int is array
	int IsArray;
}ShaderParameter;

/*
	shader parameter array
*/
typedef struct ShaderParameterArray {
	void* Data;
	unsigned int Size;
}ShaderParameterArray;


/*
	constant buffers
*/
typedef struct ConstantBuffer {
	String Name;
	unsigned int Size;
	unsigned int Slot;
	unsigned int Space;
	int IsArray;
}ConstantBuffer;

/*
	raytracing scene
*/

typedef struct RtSceneUnit {
	String Name;
	unsigned int Slot;
}RtSceneUnit;

/*
	texture unit
*/
typedef struct TextureUnit {
	String Name;
	unsigned int Slot;
	unsigned int Space;
}TextureUnit;

/*
	buffers
*/
typedef struct BufferUnit {
	String Name;
	unsigned int Slot;
	unsigned int Space;
}BufferUnit;

/*
	RWBuffers, Unordered Access Buffer
*/
typedef struct RWBufferUnit {
	String Name;
	unsigned int Slot;
	unsigned int Space;
}RWBufferUnit;

/*
	RWTextures, Unordered Access Texture
*/
typedef struct RWTextureUnit {
	String Name;
	unsigned int Slot;
	unsigned int Space;
}RWTextureUnit;


/*
	pass
*/
typedef struct Pass {
	int VS;
	int PS;
	int CS;
	int DepthStencil;
	int Rasterizer;
	int Blend;
	int InputLayout;
	String Name;
	// light radius, wind direction, bone matrices eg.
	Vector<ShaderParameter> Parameters;
	// constant buffers
	Vector<ConstantBuffer> Constants;
	// textures
	Vector<TextureUnit> TextureUnits;
	// buffers
	Vector<BufferUnit> BufferUnits;
	// rw buffers
	Vector<RWBufferUnit> RWBufferUnits;
	// rw textures
	Vector<RWTextureUnit> RWTextureUnits;

}Pass;

/*
instancing data
*/
typedef struct InstanceElement {
	String Name;
	int Offset;
	int Size;

}InstanceElement;

/*
	technique
*/
typedef struct Technique {
	// pass count
	int PassCount;
	// instance
	bool Instance;
	// technique name
	String Name;
	// max pass 2
	Pass* RenderPass[2];
	// instance elemnt
	Vector<InstanceElement> InstanceElements;
}Technique;


/*
	Shader class. shader contains several technich. tech_pre tech_shading tech_shadow
*/


class Shader : public GPUResource
{
	OBJECT(Shader);
	BASEOBJECT(Shader);
	LOADEROBJECT(FileLoader);
	DECLARE_ALLOCATER(Shader);
private:
	// techs 
	Vector<Technique> Techs;
	// raw xml
	void* RawXML;
	// parsed xml
	xml_document<>* xml_doc;
private:
	// read shader constant use reflection
	int ReflectShader(Pass* RenderPass, void* Shader, unsigned int Size, Vector<InstanceElement>& InstanceElements);

	// get variant from dict
	// Variant* GetParameter(String& Name, Dict& Material, Dict& Object, RenderContext* Context);

	template <class ...T> Variant* GetParameter(String& Name, RenderContext* Context, T& ...parameterList);

public:
	Shader(Context* context);
	~Shader();
	template <class ...T> void Apply(RenderCommandContext* cmdContext, int stage, RenderContext* Context, T& ...parameterList);
	Deserializer virtual AsyncLoad();
	bool IsInstance(int Stage);
	virtual int OnSerialize(Deserializer& deserializer);
	virtual int OnLoadComplete(Variant& Parameter);
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext* Context);
	virtual int MakeInstance(int Stage, Dict& ObjectParameter, void* Buffer);

};

template <class ...T> Variant* Shader::GetParameter(String& Name, RenderContext* Context, T& ...parameterList) {

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

template <class ...T> void Shader::Apply(RenderCommandContext* cmdContext, int stage, RenderContext* Context, T& ...parameterList )
{
	int techs = Techs.Size();
	int Compiled = 0;
	if (techs) {
		Technique* tech = &Techs[stage];
		for (int c = 0; c < tech->PassCount; c++) {
			Pass* pass = tech->RenderPass[c];
			// inputayput
			cmdContext->SetInputLayout(pass->InputLayout);
			// textures
			int texture_units = pass->TextureUnits.Size();
			for (int i = 0; i < texture_units; i++) {
				TextureUnit* unit = &pass->TextureUnits[i];
				Variant* Value = GetParameter(unit->Name, Context, parameterList...);
				if (Value) {
					int id = Value->as<int>();
					cmdContext->SetSRV(unit->Slot, id);
				}
			}
			// buffers (SRV)
			int buffer_units = pass->BufferUnits.Size();
			for (int i = 0; i < buffer_units; i++) {
				BufferUnit* unit = &pass->BufferUnits[i];
				Variant* Value = GetParameter(unit->Name, Context, parameterList...);
				if (Value) {
					int id = Value->as<int>();
					cmdContext->SetSRV(unit->Slot, id);
				}
			}
			// rwbuffers (UAV)
			int rwbuffer_units = pass->RWBufferUnits.Size();
			for (int i = 0; i < rwbuffer_units; i++) {
				RWBufferUnit* unit = &pass->RWBufferUnits[i];
				Variant* Value = GetParameter(unit->Name, Context, parameterList...);
				if (Value) {
					int id = Value->as<int>();
					cmdContext->SetUAV(unit->Slot, id);
				}
			}
			// rwtextures (UAV)
			int rwtexture_units = pass->RWTextureUnits.Size();
			for (int i = 0; i < rwtexture_units; i++) {
				RWTextureUnit* unit = &pass->RWTextureUnits[i];
				Variant* Value = GetParameter(unit->Name, Context, parameterList...);
				if (Value) {
					int id = Value->as<int>();
					cmdContext->SetUAV(unit->Slot, id);
				}
			}
			// constants
			int parameters = pass->Parameters.Size();
			for (int i = 0; i < parameters; i++) {
				ShaderParameter* parameter = &pass->Parameters[i];
				Variant* Value = GetParameter(parameter->Name, Context, parameterList...);
				if (Value) {
					if (!parameter->IsArray) {
						cmdContext->UpdateConstantBuffer(parameter->Slot, parameter->Offset, Value, parameter->Size);
					}
					else {
						ShaderParameterArray& Array = Value->as<ShaderParameterArray>();
						cmdContext->UpdateConstantBuffer(parameter->Slot, parameter->Offset, Array.Data, parameter->Size);
					}
				}
			}
			// update constants
			int Constants = pass->Constants.Size();
			for (int i = 0; i < Constants; i++) {
				ConstantBuffer* cons = &pass->Constants[i];
				cmdContext->SetConstantBuffer(cons->Slot, cons->Size);
			} 
			// shaders
			if (pass->VS != -1)
				cmdContext->SetVertexShader(pass->VS);
			if (pass->PS != -1)
				cmdContext->SetPixelShader(pass->PS);
			if (pass->CS != -1)
				cmdContext->SetComputeShader(pass->CS);
			// render stats
			cmdContext->SetDepthStencilState(pass->DepthStencil);
			cmdContext->SetRasterizer(pass->Rasterizer);
			cmdContext->SetBlendState(pass->Blend);
		}
	}
}

#endif