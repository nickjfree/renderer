#ifndef __FRAME_GRAPH__
#define __FRAME_GRAPH__

#include <functional>
#include "Container/Dict.h"
#include "Core/Str.h"
#include "RenderContext.h"
#include "CommandBuffer.h"
#include "RenderingCamera.h"
#include "Spatial.h"

/*
*  frame graph implementation
*/


/*
	resource
*/
class FrameGraph;
class GraphBuilder;
class BaseRenderPass;

class RenderResource
{
	friend GraphBuilder;
	friend FrameGraph;
public:
	// get real resource id
	int GetActualResource();
	// flip resource, swap texture
	void Flip(RenderResource* resource);
	// get owner
	BaseRenderPass* GetOwner() { return owner; }
private:
	// owner pass
	BaseRenderPass* owner = nullptr;
	// actual resource creation
	std::function<int()> realize = nullptr;
	// depend
	RenderResource* input = nullptr;
	// actual resource_id 
	int resourceId = -1;
	// diection
	enum class Direction {
		READ,
		WRITE,
		CREATE,
		TRANSIENT,
	};
	// Direction
	Direction direction = Direction::READ;
	// name 
	String name = {};
};


/*
	graph builder
*/
class GraphBuilder
{
public:
	GraphBuilder(BaseRenderPass* renderPass);
	// input resource
	RenderResource Read(RenderResource* input);
	// write resource
	RenderResource Write(RenderResource* input);
	// create
	RenderResource Create(const String& name, std::function<int()> createFunc);
	// create transient
	RenderResource CreateTransient(const String& name, std::function<int()> createTransientFunc);
	// use async
	void SetAsyncCompute(bool value);
	// add renderpass
	
private:
	// pass
	BaseRenderPass* renderPass;
};


/*
	render pass
*/
class BaseRenderPass
{
public:
	// setup
	virtual void Setup(GraphBuilder& builder) = 0;
	// execute
	virtual void Execute(CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial) = 0;
	// add inputs
	void AddDependency(BaseRenderPass* dependency);
	// set async
	void SetAsyncCompute(bool value) { asyncCompute = value; }
	// get async compute
	bool IsAsyncCompute() { return asyncCompute; }
	// setname
	void SetName(const String& name) { this->name = name; }
	// getname
	const String& GetName() { return name; }
protected:
	// dependencies
	Vector<BaseRenderPass*> dependencies;
	// asynccompute
	bool asyncCompute = false;
	// name
	String name;
};


template <class T>
class RenderPass : public BaseRenderPass
{
public:
	// setup callback
	std::function<void(GraphBuilder& builder, T& passData)> setup;
	// execute callback
	std::function<void(T& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial)> execute;
	// setup
	void Setup(GraphBuilder& builder);
	// execute
	void Execute(CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial);
	// get data
	T& Data() { return passData; }
private:
	// passData
	T passData = {};
};

template <class T> void RenderPass<T>::Setup(GraphBuilder& builder)
{
	setup(builder, passData);
}

template <class T> void RenderPass<T>::Execute(CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial)
{
	execute(passData, cmdBuffer, cam, spatial);
}

/*
	renderingqueue
*/
class RenderingQueue
{

};


/*
	frame graph
*/
class FrameGraph
{
public:
	// add render pass
	template <class T> RenderPass<T>* AddRenderPass(
		const String& name, 
		std::function<void(GraphBuilder& builder, T& passData)> setup,
		std::function<void(T& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial)> execute);
private:
	// render passes
	Vector<BaseRenderPass*> renderPasses;
};


template<class T> RenderPass<T>* FrameGraph::AddRenderPass(
	const String& name, std::function<void(GraphBuilder& builder, T& passData)> setup,
	std::function<void(T& passData, CommandBuffer* cmdBuffer, RenderingCamera* cam, Spatial* spatial)> execute)
{
	auto renderPass = new RenderPass<T>();
	auto builder = GraphBuilder(renderPass);
	renderPass->setup = setup;
	renderPass->execute = execute;
	renderPass->SetName(name);
	// setup renderpass
	renderPass->Setup(builder);
	BaseRenderPass* pass = renderPass;
	renderPasses.PushBack(pass);
	return renderPass;
}

#endif

