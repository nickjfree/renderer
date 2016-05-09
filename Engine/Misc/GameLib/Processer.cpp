#include "RenderProcesser.h"

using namespace Render;

CProcesser::CProcesser(IMRender * Render):m_Render(Render),m_LightIndex(-1)
{
}

CProcesser::~CProcesser(void)
{
}

// call processRenderable in this function
int Render::CProcesser::ProcessCommand(RenderState * State,RenderCommand * Command)
{
	//frist prepare
	Prepare(State,Command);
	//process each renderable next
	RenderJob * Job;
	Renderable * pRenderable;
	Job = Command->Job;
	while(Job)
	{
		for(int index = 0;index < Job->NumRenderable;index++)
		{
			pRenderable = &Job->RenderableList[index];
			ProcessRenderable(pRenderable,Command->ContextKey);
		}
		Job = Job->NextJob;
	}
	m_Render->Flush();
	//sub commands
	RenderCommand * SubCommand = Command->SubCommand;
	while(SubCommand)
	{
		CProcesser::ProcessCommand(State,SubCommand);
		SubCommand = SubCommand->NextCommand;
	}
	return 0;
}

// set rendersate , constants,shaders,effect
int Render::CProcesser::Prepare(RenderState * State,RenderCommand * Command)
{
	//m_Render->UseEffect(Command->EffectIndex);
	m_Render->SetViewMatrix(&State->ViewMatrix,&State->ViewPoint);
	m_Render->SetProjectionMatrix(&State->Projection);
	m_Render->SetRenderContext(Command->ContextKey);
	//m_Render->UsePass(Command->TechIndex,Command->PassIndex);
	return 0;
}

// process a renderable
int Render::CProcesser::ProcessRenderable(Renderable * pRenderable,int * ContextKey)
{
	//do thing in base class
	return 0;
}
