#ifndef __COMMAND_BUFFER__
#define __COMMAND_BUFFER__

#include "Container/Dict.h"
#include "Container/Vector.h"
#include "Container/RecyclePool.h"
#include "Core/Shortcuts.h"
#include "Core/Str.h"
#include "Rendering/Material.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"
#include "Rendering/ShaderLibrary.h"
#include "RenderingCamera.h"
#include <functional>

// copy command
typedef struct CopyResourceCommand
{
	// destination
	int dest;
	// source
	int src;
}CopyResourceCommand;

// draw command
typedef struct DrawCommand
{
	// mesh id
	Mesh* mesh;
	// material
	Material* material;
	// pass index to rendering
	int passIndex;
	// instance buffer
	void* instanceBuffer;
	// numinstances
	int numInstances;
	// instance stride
	int instanceStride;
}DrawCommand;

// disptach rays
typedef struct DispatchRaysCommand
{
	// material
	Material* material;
	// ray id
	int rayId;
	// scale
	int width;
	int height;
}DispatchRaysCommand;

// compute job
typedef struct BuildAccelerationStructureCommand
{
	// mesh
	Mesh* mesh;
	// material
	Material* material;
	// material id
	int materialId;
	// int 
	int flag;
	// transform
	Matrix4x4 transform;
	// deformable
	int transientGeometryId;
}BuildAccelerationStructureCommand;


// compute job
typedef struct DispatchComputeCommand
{
	// material
	Material* material;
	// pass index to rendering
	int passIndex;
	// groups
	int x;
	int y;
	int z;
}DispatchComputeCommand;

// set render targets
typedef struct RenderTargetCommand
{
	// render targets
	int targets[8];
	// numtargets
	int numTargets;
	// depth
	int depth;
	// clear target
	bool clearTarget;
	// clear depth
	bool clearDepth;
	// scale
	int width;
	int height;
}RenderTargetCommand;


/*
*	shader bindings
*/

typedef struct ShaderBinding
{
	enum class Type {
		CONSTANT,
		SRV,
		UAV,
		COUNT,
	};

	int slot;
	Type type;
	union {
		int resoutceId;
		union {
			void* data;
			unsigned int size;
		};
	};

	void Apply(RenderCommandContext * cmdContext) {
		switch (type) {
		case ShaderBinding::Type::CONSTANT:
			cmdContext->UpdateConstantBuffer(slot, 0, data, size);
			cmdContext->SetConstantBuffer(slot, size);
			break;
		case ShaderBinding::Type::SRV:
			cmdContext->SetSRV(slot, resoutceId);
			break;
		case ShaderBinding::Type::UAV:
			cmdContext->SetUAV(slot, resoutceId);
			break;
		default:
			break;
		}
	};
}ShaderBinding;




/*
*	gpu rendering command
*/
constexpr  int cmd_max_shader_bindings = 16;

class RenderingCommand
{
	DECLARE_RECYCLE(RenderingCommand);
public:
	// set constant
	RenderingCommand& SetShaderConstant(int slot, void* data, unsigned int size) {
		auto& binding = shaderBindings[numShaderBindings++];
		binding.type = ShaderBinding::Type::CONSTANT;
		binding.data = data;
		binding.size = size;
		binding.slot = slot;
		return *this;
	}

	// set srv
	RenderingCommand& SetShaderResource(int slot, int id) {
		auto& binding = shaderBindings[numShaderBindings++];
		binding.type = ShaderBinding::Type::SRV;
		binding.resoutceId = id;
		return *this;
	}

	// set uav
	RenderingCommand& SetRWShaderResource(int slot, int id) {
		auto& binding = shaderBindings[numShaderBindings++];
		binding.type = ShaderBinding::Type::UAV;
		binding.resoutceId = id;
		return *this;
	}

public:
	// cmdType
	enum class CommandType
	{
		SETUP,
		DRAW,
		DRAW_INSTANCED,
		DISPATCH_COMPUTE,
		DISPATCH_RAYS,
		BUILD_AS,
		RENDER_TARGET,
		COPY_RESOURCE,
	};
	CommandType cmdType = CommandType::DRAW;
	
	// constants and other parameters
	Dict cmdParameters;
	// shader bindings
	ShaderBinding shaderBindings[cmd_max_shader_bindings];
	int numShaderBindings = 0;

	union {
		DrawCommand draw = {};
		DispatchRaysCommand dispatchRays;
		DispatchComputeCommand dispatchCompute;
		BuildAccelerationStructureCommand buildAS;
		RenderTargetCommand renderTargets;
		CopyResourceCommand copyResource;
	};
};

/*
	to record rendering commands
*/
constexpr auto max_command_buffer_size = 4096;
constexpr auto max_instance_size = 256;
constexpr auto max_instance_buffer_size = max_instance_size * 2048;  // 512Kb


class CommandBuffer
{
	DECLARE_RECYCLE(CommandBuffer)
public:
	// Alloc
	static CommandBuffer* Alloc();
	// alloc a new command
	RenderingCommand* AllocCommand();
	// copy
	void CopyResource(RenderingCommand* cmd, int dest, int src);
	// draw
	void Quad(RenderingCommand* cmd, Material* material, int passIndex);
	// draw
	void Draw(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex);
	// draw instanced
	void DrawInstanced(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex);
	// dispatch compute
	void Dispatch(RenderingCommand* cmd, Material* material, int passIndex, int x, int y, int z);
	// dispatch rays
	void DispatchRays(RenderingCommand* cmd, int rayId, Material* material, int w, int h);
	// dispatch rays
	void BuildAccelerationStructure(RenderingCommand* cmd, Mesh* mesh, Material* material, Matrix4x4& transform, int transientGeometryId, int materialId, int flag);
	// render targets
	void RenderTargets(RenderingCommand* cmd, int* targets, int numTargets, int depth, bool clearTargets, bool clearDepth, int w, int h);
	// flush and record commandlist
	void Flush(RenderCommandContext* cmdContext);
	// reset
	void Reset();
	// set frame parameter
	void SetupFrameParameters(RenderingCamera* cam, RenderContext* renderContext);
	// get global parameter
	void SetGlobalParameter(const String& name, Variant& data);
private:
	// alloc instance buffer
	bool appendInstanceBuffer(size_t size);
	// process rendertargets
	void setRenderTargets(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// draw
	void draw(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// draw instanced
	void drawInstanced(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// 
	void dispatch(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// build as
	void buildAccelerationStructure(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// dispatch rays
	void dispatchRays(RenderingCommand* cmd, RenderCommandContext* cmdContext);
	// copy
	void copyResource(RenderingCommand* cmd, RenderCommandContext* cmdContext);
private:
	// commands
	RenderingCommand renderingCommands[max_command_buffer_size];
	// num
	int currentIndex = 0;
	// instance 
	unsigned char instanceBuffer[max_instance_buffer_size] = {};
	// size
	size_t usedInstanceBuffer = 0;
	// global parameters
	Dict globalParameters;
	// renderContext
	RenderContext* renderContext = nullptr;
};

#endif


