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
	ret.name = input->name;
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
	ret.name = input->name;
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
	// TODO
}

void FrameGraph::Execute(RenderingCamera* cam, Spatial* spatial, RenderContext* renderContext)
{
	// this is a test run, just run the pass in order
	auto renderInterface = renderContext->GetRenderInterface();
	if (renderPasses.Size()) {
		auto gbuffer = renderPasses[0];
		auto shadow = renderPasses[1];
		auto lighting = renderPasses[2];
		auto emissive = renderPasses[3];
		auto ao = renderPasses[4];
		auto reflection = renderPasses[5];
		auto rtLighting = renderPasses[6];
		auto resolve = renderPasses[7];
		auto hdr = renderPasses[8];
		auto as = renderPasses[9];
		

		// get commandbuffer
		auto cmdBuffer = CommandBuffer::Alloc();
		// execute the gbuffer pass
		gbuffer->Execute(cmdBuffer, cam, spatial);

		// sync point: flush the gbuffer command
		auto renderCommandContext = renderInterface->BeginContext(false);
		cmdBuffer->Flush(renderCommandContext);
		cmdBuffer->Recycle();
		auto graphicsFence = renderInterface->EndContext(renderCommandContext, false);
		UINT64 computeFence = 0;
		{
			// build the raytracing structure
			auto cmdBuffer = CommandBuffer::Alloc();
			as->Execute(cmdBuffer, cam, spatial);
			auto computeCommandContext = renderInterface->BeginContext(true);
			computeCommandContext->Wait(graphicsFence, false);
			cmdBuffer->Flush(computeCommandContext);
			cmdBuffer->Recycle();
			computeFence = renderInterface->EndContext(computeCommandContext, false);
		}
		{
			// new command buffer
			auto cmdBuffer = CommandBuffer::Alloc();
			// execute the shadow pass
			shadow->Execute(cmdBuffer, cam, spatial);
			auto renderCommandContext = renderInterface->BeginContext(false);
			cmdBuffer->Flush(renderCommandContext);
			cmdBuffer->Recycle();
			renderInterface->EndContext(renderCommandContext, false);
		}
		{
			// new command buffer
			auto cmdBuffer = CommandBuffer::Alloc();
			// execute the lighting pass
			lighting->Execute(cmdBuffer, cam, spatial);
			// emissive
			emissive->Execute(cmdBuffer, cam, spatial);
			// do ssao
			ao->Execute(cmdBuffer, cam, spatial);
			// do rt-relection
			reflection->Execute(cmdBuffer, cam, spatial);
			// do rt-relection
			rtLighting->Execute(cmdBuffer, cam, spatial);
			// do resolve
			resolve->Execute(cmdBuffer, cam, spatial);
			// do hdr
			hdr->Execute(cmdBuffer, cam, spatial);
			// flush the command buffer
			auto renderCommandContext = renderInterface->BeginContext(false);
			renderCommandContext->Wait(computeFence, true);
			cmdBuffer->Flush(renderCommandContext);
			cmdBuffer->Recycle();
			// present
			renderInterface->EndContext(renderCommandContext, true);
		}
	}
}