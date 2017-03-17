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
	shader parameter
*/

typedef struct ShaderParameter{
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
}ShaderParameter;

/*
	constant buffers
*/
typedef struct ConstantBuffer {
	String Name;
	unsigned int Size;
	unsigned int Slot;
}ConstantBuffer;

typedef struct TextureUnit {
	String Name;
	unsigned int Slot;
}TextureUnit;

/*
	pass
*/
typedef struct Pass{
	int VS;
	int GS;
	int HS;
	int DS;
	int PS;
	int DepthStencil;
	int Rasterizer;
	int Blend;
	int InputLayout;
	String Name;
	// light radius, wind direction, bone matrices eg.
	Vector<ShaderParameter> Parameters;
	// constant buffer
	Vector<ConstantBuffer> Constants;
	// textures
	Vector<TextureUnit> TextureUnits;
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
typedef struct Technique{
	// pass count
	int PassCount;
	// instance
	bool Instance;
	// technique name
	String Name;
	// max pass 2
	Pass * RenderPass[2];
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
	DECLAR_ALLOCATER(Shader);
private:
	// techs 
	Vector<Technique> Techs;
	// raw xml
	void * RawXML;
	// parsed xml
	xml_document<> * xml_doc;
private:
	// read shader constant use reflection
	int ReflectShader(Pass * RenderPass, void * Shader, unsigned int Size, Vector<InstanceElement>& InstanceElements);

	// get variant from dict
	Variant* GetParameter(String& Name, Dict& Material, Dict& Object, RenderContext * Context);
public:
	Shader(Context * context);
	~Shader();
	int Apply(int Stage, char * buffer, Dict& Parameter);
	Deserializer virtual AsyncLoad();
	bool IsInstance(int Stage);
	virtual int OnSerialize(Deserializer& deserializer);
	virtual int OnLoadComplete(Variant& Parameter);
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& MaterialParam, Dict& ObjectParameter, RenderContext * Context);
	virtual int MakeInstance(BatchCompiler * Compiler, int Stage, Dict& ObjectParameter, void * Buffer);
};



#endif