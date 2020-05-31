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
	DECLAR_ALLOCATER(ShaderLibrary);

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
	Variant* GetParameter(String& Name, Dict& Material, Dict& Object, RenderContext* Context);
public:
	ShaderLibrary(Context* context);

	virtual int OnSerialize(Deserializer& deserializer);
	virtual int OnLoadComplete(Variant& Parameter);
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext* Context);
	void GetLocalResourceBindings(Dict& MaterialParam, RenderContext* Context, R_RESOURCE_BINDING * bindings, int* count);
};


#endif
