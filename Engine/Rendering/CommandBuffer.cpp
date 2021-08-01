#include "CommandBuffer.h"

USING_RECYCLE(RenderingCommand)
USING_RECYCLE(CommandBuffer)


CommandBuffer* CommandBuffer::Alloc()
{
	auto commandBuffer = CommandBuffer::Create();
	commandBuffer->Reset();
	return commandBuffer;
}


ShaderConstant<PerFrameData> CommandBuffer::GetFrameParameters(RenderingCamera* cam, RenderContext* renderContext)
{
	ShaderConstant<PerFrameData> perFrameConstant{};
	// per-frame constant 
	Matrix4x4::Tranpose(cam->GetInvertView(), &perFrameConstant.gInvertViewMaxtrix);
	Matrix4x4::Tranpose(cam->GetViewMatrix(), &perFrameConstant.gViewMatrix);
	Matrix4x4::Tranpose(cam->GetViewProjection(), &perFrameConstant.gViewProjectionMatrix);
	perFrameConstant.gViewPoint = cam->GetViewPoint();
	perFrameConstant.gScreenSize.x = static_cast<float>(renderContext->FrameWidth);
	perFrameConstant.gScreenSize.y = static_cast<float>(renderContext->FrameHeight);
	return perFrameConstant;
}

void CommandBuffer::SetGlobalParameter(const String& name, Variant& data)
{
	globalParameters[name] = data;
}

void CommandBuffer::SetFrameShaderInput(ShaderInput* input)
{
	shaderInputs.Add(input);
}

void CommandBuffer::Reset() {
	currentIndex = 0;
	usedInstanceBuffer = 0; 
	globalParameters.Clear();
	shaderInputs.Reset();
}

bool CommandBuffer::appendInstanceBuffer(size_t size)
{
	if (usedInstanceBuffer + size + max_instance_size > max_instance_buffer_size) {
		return false;
	} 
	// append buffer
	usedInstanceBuffer += size;
	return true;
}

RenderingCommand* CommandBuffer::PassSetup(bool isCumpute)
{
	auto cmd = AllocCommand();
	cmd->cmdType = RenderingCommand::CommandType::PASS_SETUP;
	cmd->isComputePassSetup = isCumpute;
	return cmd;
}


void CommandBuffer::CopyResource(RenderingCommand* cmd, int dest, int src)
{
	cmd->cmdType = RenderingCommand::CommandType::COPY_RESOURCE;
	cmd->copyResource.dest = dest;
	cmd->copyResource.src = src;
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
		prev = &renderingCommands[currentIndex - 2];
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
	} else {
		// error, 
		printf("shader doesn't support instancing %s\n", shader->GetUrl().ToStr());
	}

}

void CommandBuffer::Dispatch(RenderingCommand* cmd, Material* material, int passIndex, int x, int y, int z)
{
	cmd->cmdType = RenderingCommand::CommandType::DISPATCH_COMPUTE;
	cmd->dispatchCompute.material = material;
	cmd->dispatchCompute.passIndex = passIndex;
	cmd->dispatchCompute.x = x;
	cmd->dispatchCompute.y = y;
	cmd->dispatchCompute.z = z;
}


void CommandBuffer::DispatchRays(RenderingCommand* cmd, int rayId, Material* material, int w, int h)
{
	cmd->cmdType = RenderingCommand::CommandType::DISPATCH_RAYS;
	cmd->dispatchRays.material = material;
	cmd->dispatchRays.rayId = rayId;
	cmd->dispatchRays.width = w;
	cmd->dispatchRays.height = h;
}

void CommandBuffer::BuildAccelerationStructure(RenderingCommand* cmd, Mesh* mesh, Material* material, Matrix4x4& transform, int transientGeometryId, int materialId, int flag)
{
	cmd->cmdType = RenderingCommand::CommandType::BUILD_AS;
	cmd->buildAS.transientGeometryId = transientGeometryId;
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

void CommandBuffer::setRenderTargets(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->SetRenderTargets(cmd->renderTargets.targets, cmd->renderTargets.numTargets, cmd->renderTargets.depth);
	cmdContext->ClearRenderTargets(cmd->renderTargets.clearTarget, cmd->renderTargets.clearDepth);
	if (cmd->renderTargets.width || cmd->renderTargets.height) {
		cmdContext->SetViewPort(0, 0, cmd->renderTargets.width, cmd->renderTargets.height);
	}
}

void CommandBuffer::draw(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->SetGraphicsMode();
	// set material and constants
	if (cmd->draw.material) {
		auto material = cmd->draw.material;
		auto shader = cmd->draw.material->GetShader();
		shader->Apply(cmdContext, cmd->draw.passIndex, renderContext, cmd->cmdParameters, material->GetParameter(), globalParameters);
	}
	// TEST: constant buffer bindings
	cmd->shaderInputs.Apply(cmdContext);
	// draw
	if (cmd->draw.mesh == 0) {
		// no mesh set. draw full screen quad
		cmdContext->Quad();
	} else {
		cmdContext->Draw(cmd->draw.mesh->GetId());
	}
}

void CommandBuffer::drawInstanced(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->SetGraphicsMode();
	// set material and constants
	if (cmd->draw.material) {
		auto material = cmd->draw.material;
		auto shader = cmd->draw.material->GetShader();
		shader->Apply(cmdContext, cmd->draw.passIndex, renderContext, cmd->cmdParameters, material->GetParameter(), globalParameters);
	}
	cmd->shaderInputs.Apply(cmdContext);
	// draw
	if (cmd->draw.mesh == 0) {
		// no mesh set. draw full screen quad with instancing. id 0 is the full screen quad
		cmdContext->DrawInstanced(0, cmd->draw.instanceBuffer, cmd->draw.instanceStride, cmd->draw.numInstances);
	} else {
		cmdContext->DrawInstanced(cmd->draw.mesh->GetId(), cmd->draw.instanceBuffer, cmd->draw.instanceStride, cmd->draw.numInstances);
	}
}

void CommandBuffer::buildAccelerationStructure(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	auto material = cmd->buildAS.material;
	auto rtShader = material->GetShaderLibrary(0);
	if (rtShader) {
		R_RAYTRACING_INSTANCE instance = {};
		instance.Flag = cmd->buildAS.flag;
		instance.MaterialId = cmd->buildAS.materialId;
		instance.rtGeometry = cmd->buildAS.transientGeometryId != -1? cmd->buildAS.transientGeometryId: cmd->buildAS.mesh->GetId();
		instance.Transform = cmd->buildAS.transform;
		material->GetRtShaderBindings(renderContext, &instance);
		// add instance
		cmdContext->AddRaytracingInstance(&instance);
	}
}

void CommandBuffer::dispatch(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->SetComputeMode();
	auto material = cmd->dispatchCompute.material;
	auto shader = material->GetShader();
	if (shader) {
		// apply shader
		shader->Apply(cmdContext, cmd->dispatchCompute.passIndex, renderContext, cmd->cmdParameters, material->GetParameter(), globalParameters);
		// apply shader inputs
		cmd->shaderInputs.Apply(cmdContext);
		// dispatch rays
		cmdContext->DispatchCompute(cmd->dispatchCompute.x, cmd->dispatchCompute.y, cmd->dispatchCompute.z);
	}
}

void CommandBuffer::dispatchRays(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->SetRaytracingMode();
	auto material = cmd->dispatchRays.material;
	auto rtShader = material->GetShaderLibrary(cmd->dispatchRays.rayId);
	if (rtShader) {
		// apply shader
		rtShader->Apply(cmdContext, renderContext, cmd->cmdParameters, material->GetParameter(), globalParameters);
		// apply shader inputs
		cmd->shaderInputs.Apply(cmdContext);
		// dispatch rays
		cmdContext->DispatchRays(rtShader->GetId(), cmd->dispatchRays.width, cmd->dispatchRays.height);
	}
}

void CommandBuffer::copyResource(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	cmdContext->CopyResource(cmd->copyResource.dest, cmd->copyResource.src);
}

void CommandBuffer::passSetup(RenderingCommand* cmd, RenderCommandContext* cmdContext)
{
	if (cmd->isComputePassSetup) {
		cmdContext->SetComputeMode();
	} else {
		cmdContext->SetGraphicsMode();
	}
	cmd->shaderInputs.Apply(cmdContext);
}

void CommandBuffer::Flush(RenderCommandContext* cmdContext)
{
	// TODO: submit to rendercontext
	auto i = 0;
	bool hasAs = false;
	while(i < currentIndex) {
		auto& cmd = renderingCommands[i++];
		switch (cmd.cmdType) {
		case RenderingCommand::CommandType::RENDER_TARGET:
			setRenderTargets(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::DRAW:
			draw(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::DRAW_INSTANCED:
			drawInstanced(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::BUILD_AS:
			buildAccelerationStructure(&cmd, cmdContext);
			hasAs = true;
			break;
		case RenderingCommand::CommandType::DISPATCH_COMPUTE:
			dispatch(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::DISPATCH_RAYS:
			dispatchRays(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::COPY_RESOURCE:
			copyResource(&cmd, cmdContext);
			break;
		case RenderingCommand::CommandType::PASS_SETUP:
			passSetup(&cmd, cmdContext);
			break;
		default:
			break;
		}
	}
	if (hasAs) {
		cmdContext->BuildAccelerationStructure();
	}
	// recycle cmdbuffer
	Recycle();
}

// alloc a new command
RenderingCommand* CommandBuffer::AllocCommand()
{
	auto cmd = &renderingCommands[currentIndex++];
	cmd->cmdParameters.Clear();
	cmd->shaderInputs.Reset();
	return cmd;
}

void RenderingCommand::AddShaderInput(ShaderInput* input)
{
	shaderInputs.Add(input);
}

void ShaderInputList::Apply(RenderCommandContext* cmdContext)
{
	for (auto i = 0; i < numShaderInputs; ++i) {
		auto input = shaderInputs[i];
		input->Apply(cmdContext);
	}
}

void ShaderInputList::Add(ShaderInput* input)
{
	if (numShaderInputs >= cmd_max_shader_inputs) {
		printf("too many shader bindings\n");
		return;
	}
	shaderInputs[numShaderInputs++] = input;
}