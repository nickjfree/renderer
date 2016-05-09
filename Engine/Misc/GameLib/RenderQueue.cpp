#include "StdAfx.h"
#include "Include\RenderQueue.h"
#include "ResourceManager.h"
#include <stdio.h>

using Render::CRenderQueue;
using Render::RenderCommand;
using Render::Renderable;
using Render::RenderJob;
using namespace  ResourceManager;


CRenderQueue::CRenderQueue(void):m_UpToDate(0)
{
	for(int i = 0;i < MAX_CMDTYPE;i++)
	{
		m_RenderCMD[i] = 0;
	}
}

CRenderQueue::~CRenderQueue(void)
{
}

RenderCommand * Render::CRenderQueue::AllocCommand(void)
{
//	printf("Alloc CMD\n");
	RenderCommand * Cmd;
	int ID = m_RenderCommandPool.AllocResource(&Cmd);
	memset(Cmd,0,sizeof(RenderCommand));
	Cmd->CmdID = ID;
	return Cmd;
}

RenderJob * Render::CRenderQueue::AllocRenderJob(void)
{
//	printf("Alloc Job\n");
	RenderJob * Job;
	int ID = m_RenderJobPool.AllocResource(&Job);
	Job->JobID = ID;
	for(int i = 0;i < RENDERABLE_BATCH;i++)
	{
		Job->RenderableList[i].IsRenderable = 0;
	}
	return Job;
}

int Render::CRenderQueue::FreeCommand(RenderCommand * Command)
{
//	printf("Free CMD\n");
	RenderJob * Job;
	while(Command->Job)
	{
		Job = Command->Job;
		Command->Job = Command->Job->NextJob;
		FreeJob(Job);
	}
	//and free subcommand
	while(Command->SubCommand)
	{
		RenderCommand * SubCommand = Command->SubCommand;
		Command->SubCommand = SubCommand->NextCommand;
		FreeCommand(SubCommand);
	}
	m_RenderCommandPool.FreeResource(Command->CmdID);
	return 0;
}

int Render::CRenderQueue::FreeJob(RenderJob * Job)
{
//	printf("Free Job\n");
	// dec entity ref
	for(int i = 0;i < Job->NumRenderable;i++)
	{
		if (Job->RenderableList[i].Entity) {
			Job->RenderableList[i].Entity->DecQueueRef();
			Job->RenderableList[i].Entity = NULL;
		}
	}
	m_RenderJobPool.FreeResource(Job->JobID);
	return 0;
}

int Render::CRenderQueue::Reset(void)
{
	for(int i = 0;i < MAX_CMDTYPE;i++)
	{
		if(m_RenderCMD[i])
		{
			FreeCommand(m_RenderCMD[i]);
			m_RenderCMD[i] = NULL;
		}
	}
	m_RenderState.ShadowMaps = 0;
	return 0;
}

int Render::CRenderQueue::NewCMD(int Type, int Effect, int Tech, int Pass)
{
	RenderCommand * Command = m_RenderCMD[Type];
	if(Command)
	{
		/*FreeCommand(Command);
		m_RenderCMD[Type] = 0;*/
	}
	else
	{
		m_RenderCMD[Type] = AllocCommand();
		Command = m_RenderCMD[Type];
		Command->CurrentJob = 0;
		Command->EffectIndex = Effect;
		Command->TechIndex = Tech;
		Command->PassIndex = Pass;
		Command->nothing = 0;
		Command->Job = 0;
	}
	return 0;
}

int Render::CRenderQueue::AddRenderJobToCommand(RenderCommand * Command,int * ContextKey, CEntity * Entity, CSceneNode * Node)
{
	RenderCommand *SubCommand;
	// need to find the right command to add 
	for(int i = 1;i < CONTEXT_DIM;i++)
	{
		int index = ContextKey[i];
		if(!Command->SubCommand)
		{
			Command->SubCommand = AllocCommand();
			memcpy(Command->SubCommand->ContextKey,ContextKey,sizeof(int) * CONTEXT_DIM);
		}
		SubCommand = Command->SubCommand;
		while(SubCommand)
		{
			if(SubCommand->ContextKey[i] == index)
			{
				break;
			}
			SubCommand = SubCommand->NextCommand;
		}
		//no match
		if(!SubCommand)
		{
			SubCommand = AllocCommand();
			memcpy(SubCommand->ContextKey,ContextKey,sizeof(int) * CONTEXT_DIM);
			SubCommand->NextCommand = Command->SubCommand;
			Command->SubCommand = SubCommand;
		}
		Command = SubCommand;
	}
	// so add jobs to found command
	RenderJob * Job = Command->CurrentJob;
	if(Job && Job->NumRenderable < RENDERABLE_BATCH)
	{
		Job = Command->CurrentJob;
	}
	else
	{
		Job = AllocRenderJob();
		Job->NextJob = 0;
		Job->NumRenderable = 0;
		if(Command->CurrentJob == 0 || Command->Job == 0)
		{
			Command->Job = Job;
			Command->CurrentJob = Job;
		}
		else
		{
			Command->CurrentJob->NextJob = Job;
			Command->CurrentJob = Job;
		}
	}
	Job->RenderableList[Job->NumRenderable].IsRenderable = 1;
	Job->RenderableList[Job->NumRenderable].Entity = Entity;
	Job->RenderableList[Job->NumRenderable].Node = Node;
	Job->RenderableList[Job->NumRenderable].Param = Node->GetRenderParameter();
	Job->NumRenderable++;
	return 0;
}

int Render::CRenderQueue::AddRenderJobByContext(int * ContextKey, CEntity * Entity, CSceneNode * Node)
{
	// get entry command
	if(ContextKey[CONTEXT_TECH] == -1)
		return -1;
	RenderCommand * Command = m_RenderCMD[ContextKey[CONTEXT_TECH]];
	if(!Command)
	{
		Command = AllocCommand();
		m_RenderCMD[ContextKey[CONTEXT_TECH]] = Command;
	}
	AddRenderJobToCommand(Command,ContextKey,Entity,Node);
	return 0;
}

int Render::CRenderQueue::AddRenderJobByContext(int * ContextKey, CEntity * Entity, CSceneNode * Node,int LightIndex)
{
//	printf("add jobs %d ", Node->GetNodeID());
	// get entry command
	if(ContextKey[CONTEXT_TECH] == -1)
		return -1;
	int SlotOffset = SHADOW + LightIndex * MAX_SHADOW_CMD;
	int CommandOffset = ContextKey[CONTEXT_TECH] + SlotOffset - SHADOW_START;
	RenderCommand * Command = m_RenderCMD[CommandOffset];
	if(!Command)
	{
		Command = AllocCommand();
		m_RenderCMD[CommandOffset] = Command;
	}
	// need to find the right command to add 
	AddRenderJobToCommand(Command,ContextKey,Entity,Node);
	return 0;
}

int Render::CRenderQueue::AddRenderJob(CEntity * Entity,CMaterial * Material,CSceneNode * Node)
{
	// sort by material, a diff-tree structure
	int MeshID = -1;
	if(!Material)
	{
		return -1;
	}
	if(Entity && !Entity->IsRamOnly ())
	{
		MeshID = Entity->GetMeshID();
	}
	else
	{
		if(Entity)
			CResourceManager::GetResourceManager()->GetMeshToEntity(Entity);
		MeshID = -1;
	}
	//normal pass
	int * ContextKey = Material->GetContextKey(MeshID);
	AddRenderJobByContext(ContextKey,Entity,Node);
	//pre pass
	ContextKey = Material->GetPreContextKey(MeshID);
	AddRenderJobByContext(ContextKey,Entity,Node);
	//shadow pass
	return 0;
}

int Render::CRenderQueue::AddRenderJob(CEntity * Entity,CMaterial * Material,CSceneNode * Node,int LightIndex)
{
	// sort by material, a diff-tree structure
	int MeshID = -1;
	if(!Material)
	{
		return -1;
	}
	if(Entity && !Entity->IsRamOnly ())
	{
		MeshID = Entity->GetMeshID();
	}
	else
	{
		if(Entity)
			CResourceManager::GetResourceManager()->GetMeshToEntity(Entity);
		MeshID = -1;
	}
	//shadow pass
	int * ContextKey = Material->GetShadowContextKey(MeshID);
	AddRenderJobByContext(ContextKey,Entity,Node,LightIndex);
	return 0;
}
