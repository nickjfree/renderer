#include "CommandBuffer.h"

USING_RECYCLE(RenderingCommand)

bool CommandBuffer::appendInstanceBuffer(size_t size)
{
	if (usedInstanceBuffer + size + max_instance_size > max_instance_buffer_size) {
		return false;
	} 
	// append buffer
	usedInstanceBuffer += size;
}

void CommandBuffer::Quad(RenderingCommand* cmd, Material* material, int passIndex)
{
	Draw(cmd, nullptr, material, passIndex);
}

void CommandBuffer::Draw(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex)
{
	cmd->cmdType = RenderingCommand::CommandType::DRAW;
	cmd->draw.mesh = mesh;
	cmd->draw.material = material;
	cmd->draw.passIndex = passIndex;
}

void CommandBuffer::DrawInstanced(RenderingCommand* cmd, Mesh* mesh, Material* material, int passIndex)
{
	// get prev cmd
	RenderingCommand* prev = nullptr;
	if (currentIndex > 0) {
		prev = &renderingCommands[currentIndex - 1];
	}
	// get current instance data
	auto shader = material->GetShader();
	if (shader->IsInstance(passIndex)) {
		auto currentInstanceBuffer = instanceBuffer + usedInstanceBuffer;
		auto instanceStride = shader->MakeInstance(passIndex, cmd->cmdParameters, currentInstanceBuffer);
		if (!appendInstanceBuffer(instanceStride)) {
			// instancebuffer full
			printf("instance buffer full, size %zu\n", usedInstanceBuffer);
			return;
		}
		if (prev && (int)prev->cmdType == (int)RenderingCommand::CommandType::DRAW_INSTANCED && prev->draw.mesh == mesh && prev->draw.material == material) {
			// merge cmd to prev one
			prev->draw.numInstances += 1;
			// discard current cmd
			--currentIndex;
		} else {
			cmd->cmdType = RenderingCommand::CommandType::DRAW_INSTANCED;
			cmd->draw.mesh = mesh;
			cmd->draw.material = material;
			cmd->draw.passIndex = passIndex;
			cmd->draw.instanceBuffer = currentInstanceBuffer;
			cmd->draw.numInstances = 1;
			cmd->draw.instanceStride = instanceStride;
		}
	}
	// error, 
	printf("shader dosn;t support instancing %s\n", shader->GetUrl().ToStr());
}

void CommandBuffer::DispatchRays(RenderingCommand* cmd, const String& shaderName, Material* material, int w, int h)
{
	cmd->cmdType = RenderingCommand::CommandType::DISPATCH_RAYS;
	cmd->dispatchRays.material = material;
	cmd->dispatchRays.shaderName = shaderName;
	cmd->dispatchRays.width = w;
	cmd->dispatchRays.height = h;
}

void CommandBuffer::BuildAccelerationStructure(RenderingCommand* cmd, Mesh* mesh, Material* material, Matrix4x4& transform, bool deformable, int materialId, int flag)
{
	cmd->cmdType = RenderingCommand::CommandType::BUILD_AS;
	cmd->buildAS.deformable = deformable;
	cmd->buildAS.materialId = materialId;
	cmd->buildAS.flag = flag;
	cmd->buildAS.material = material;
	cmd->buildAS.mesh = mesh;
	cmd->buildAS.transform = transform;
}

void CommandBuffer::RenderTargets(RenderingCommand* cmd, int* targets, int numTargets, int depth, bool clearTargets, bool clearDepth, int w, int h)
{
	cmd->cmdType = RenderingCommand::CommandType::RENDER_TARGET;
	memcpy(cmd->renderTargets.targets, targets, sizeof(int) * numTargets);
	cmd->renderTargets.numTargets = numTargets;
	cmd->renderTargets.depth = depth;
	cmd->renderTargets.clearTarget = clearTargets;
	cmd->renderTargets.clearDepth = clearDepth;
	cmd->renderTargets.width = w;
	cmd->renderTargets.height = h;
}

void CommandBuffer::Flush(RenderCommandContext* cmdContext)
{
	//
}

// alloc a new command
RenderingCommand* CommandBuffer::AllocCommand()
{
	auto cmd = &renderingCommands[currentIndex++];
	cmd->cmdParameters.Clear();
	return cmd;
}
