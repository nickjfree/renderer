#include "FrameGraph.h"


int RenderResource::GetActualResource()
{
	if (direction == Direction::READ || direction == Direction::WRITE) {
		//  read from input
		return input->GetActualResource();
	} else if (direction == Direction::CREATE && resourceId == -1) {
		// only create once
		resourceId = realize();
	} else if (direction == Direction::TRANSIENT) {
		// always create transient resource
		resourceId = realize();
	}
	return resourceId;
}

void RenderResource::Flip(RenderResource* resource)
{
	// only persisent resource can flip
	assert(resource->direction == Direction::CREATE && direction == Direction::CREATE);
	swap(resource->resourceId, resourceId);
}


GraphBuilder::GraphBuilder(BaseRenderPass* renderPass) : renderPass(renderPass)
{
}

RenderResource GraphBuilder::Read(RenderResource* input)
{
	auto ret =  RenderResource{};
	ret.direction = RenderResource::Direction::READ;
	ret.input = input;
	ret.owner = renderPass;
	// dependency
	renderPass->AddDependency(input->GetOwner());
	return ret;
}

RenderResource GraphBuilder::Write(RenderResource* input)
{
	auto ret = RenderResource{};
	ret.direction = RenderResource::Direction::WRITE;
	ret.input = input;
	ret.owner = renderPass;
	// dependency
	renderPass->AddDependency(input->GetOwner());
	return ret;
}

RenderResource GraphBuilder::Create(const String& name, std::function<int()> createFunc)
{
	auto ret = RenderResource{};
	ret.direction = RenderResource::Direction::CREATE;
	ret.realize = createFunc;
	ret.name = name;
	ret.owner = renderPass;
	return ret;
}

RenderResource GraphBuilder::CreateTransient(const String& name, std::function<int()> createTransientFunc)
{
	auto ret = RenderResource{};
	ret.direction = RenderResource::Direction::TRANSIENT;
	ret.realize = createTransientFunc;
	ret.name = name;
	ret.owner = renderPass;
	return ret;
}

void GraphBuilder::SetAsyncCompute(bool value)
{
	renderPass->SetAsyncCompute(value);
}



void BaseRenderPass::AddDependency(BaseRenderPass* dependency)
{
	for (auto iter = dependencies.Begin(); iter != dependencies.End(); iter++) {
		if (*iter == dependency) {
			return;
		}
	}
	// add dependencies
	dependencies.PushBack(dependency);
}

void FrameGraph::Resolve()
{
}

void FrameGraph::Execute(RenderingCamera* cam, Spatial* spatial, RenderContext* renderContext)
{
	// this is a test run, just run the first pass
	auto renderInterface = renderContext->GetRenderInterface();
	if (renderPasses.Size()) {
		auto pass = renderPasses[0];
		// get commandbuffer
		auto cmdBuffer = CommandBuffer::Create();
		cmdBuffer->Reset();
		// execute the pass
		pass->Execute(cmdBuffer, cam, spatial);
		// flush the command buffer
		auto renderCommandContext = renderInterface->BeginContext(pass->IsAsyncCompute());
		cmdBuffer->Flush(renderCommandContext);
		cmdBuffer->Recycle();
		// present
		renderInterface->EndContext(renderCommandContext, true);
	}
}