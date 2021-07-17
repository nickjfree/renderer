#ifndef __RENDER_CONTEXT__
#define __RENDER_CONTEXT__

/*
	Render global setup. like shader maps, multirender target HDR pipeline.
*/
#include "RenderInterface.h"
#include "Core\Str.h"
#include "Container\HashMap.h"
#include "Container\Dict.h"
#include  "Core\Variant.h"

/*
  Constant buffer, all create at start, need to use allocators
*/
typedef struct ConstantDesc {
	int Id;
	int Slot;
	unsigned int Size;
	int Dirty;
	void* CPUData;
	String Name;
}ConstantDesc;

/*
	Shader parameters
*/
typedef struct ShaderParameterDesc {
	int Slot;
	unsigned int Offset;
	unsigned int Size;
	void* CPUData;   // a pointer to the data in constane buffer's CPUData
	String Name;
}ShaderParameterDesc;


#define MAX_CONSTANT  128
// #define FRAMEBUFFER_WIDTH  3840
// #define FRAMEBUFFER_HEIGHT 2160

#define FRAMEBUFFER_WIDTH  1920
#define FRAMEBUFFER_HEIGHT 1080



class RenderContext
{
private:
	RenderInterface* Interface;
	// constant desc entry
	ConstantDesc* Constants[MAX_CONSTANT];
	// shader parameter entry
	// RenderStatEntry
	HashMap<String, int> RenderStats;
	// RenderTarget
	HashMap<String, int> RenderTarget;
	// parameter dict, name to resource mapping
	Dict Parameters;
public:

	// frambufferszie
	int FrameWidth;
	int FrameHeight;
public:
	RenderContext(RenderInterface* Interface);
	~RenderContext();

	// interface
	RenderInterface* GetRenderInterface() { return Interface; }
	// Register constant buffer, create one if it's the first time
	int RegisterConstant(const String& Name, int Slot, unsigned int Size);
	// register shader parameter
	int RegisterParameter(const String& Name, int Slot, unsigned int Offset, unsigned int Size);
	// Set parameter
	int SetParameter(int Slot, void* CPUData, unsigned int Offset, unsigned int Size);
	// update constant
	int UpdateConstant(int Slot);
	// Regiseter Render State
	int RegisterRenderState(const String& Name, int Id);
	// Get render state by name
	int GetRenderState(const String& Name);
	// RegiterRenderTarget
	int RegisterRenderTarget(const String& Target, int Id);
	// get rendertarget
	int GetRenderTarget(const String& Target);
	// set resource
	int SetResource(const String& Name, Variant& resource);
	// get by name
	Variant* GetResource(const String& Name);
	// End Frame
	void EndFrame();
};



#endif
