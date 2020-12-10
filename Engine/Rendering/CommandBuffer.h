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
	// shader name
	String shaderName;
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
	bool deformable;
}BuildAccelerationStructureCommand;


// compute job
typedef struct DispatchComputeCommand
{
	// material
	Material* material;
	// shader
	Shader* shader;
	// pass index to rendering
	int passIndex;
	// scale
	int width;
	int height;
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
*	gpu rendering command
*/
class RenderingCommand
{
	DECLARE_RECYCLE(RenderingCommand);

public:
	// cmdType
	enum class CommandType
	{
		DRAW,
		DRAW_INSTANCED,
		DISPATCH_COMPUTE,
		DISPATCH_RAYS,
		BUILD_AS,
		RENDER_TARGET,
	};
	CommandType cmdType = CommandType::DRAW;
	
	// constants and other parameters
	Dict cmdParameters;
	// rendercontext
	RenderContext* renderContext;
	union {
		DrawCommand draw = {};
		DispatchRaysCommand dispatchRays;
		DispatchComputeCommand dispatchCompute;
		BuildAccelerationStructureCommand buildAS;
		RenderTargetCommand renderTargets;
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
public:
	// alloc a new command
	RenderingCommand* AllocCommand();
	// draw
	void Quad(RenderingCommand* cmd, Material* material, int passIndex);
	// draw
	void Draw(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex);
	// draw instanced
	void DrawInstanced(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex);
	// dispatch rays
	void DispatchRays(RenderingCommand* cmd, const String& shaderName, Material* material, int w, int h);
	// dispatch rays
	void BuildAccelerationStructure(RenderingCommand* cmd, Mesh* mesh, Material* material, Matrix4x4& transform, bool deformable, int materialId, int flag);
	// render targets
	void RenderTargets(RenderingCommand* cmd, int* targets, int numTargets, int depth, bool clearTargets, bool clearDepth, int w, int h);
	// flush and record commandlist
	void Flush(RenderCommandContext* cmdContext);
	// reset
	void Reset() { currentIndex = 0; usedInstanceBuffer = 0; }
private:
	// alloc instance buffer
	bool appendInstanceBuffer(size_t size);
private:
	// commands
	RenderingCommand renderingCommands[max_command_buffer_size];
	// num
	int currentIndex = 0;
	// instance 
	unsigned char instanceBuffer[max_instance_buffer_size];
	// size
	size_t usedInstanceBuffer;
};

#endif


