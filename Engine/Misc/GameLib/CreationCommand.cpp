#include "Include\CreationCommand.h"
#include "ResourceManager.h"
#include "Render.h"

using namespace ResourceManager;
using namespace Render;

CreationCommand::CreationCommand()
{
}


CreationCommand::~CreationCommand()
{
}

int CreationCommand::Create(IMRender * Render)
{
	CreateId = m_Resource->CreateOnRender(Render, CreateId);
	return 0;
}

int CreationCommand::Complete()
{
	m_Resource->Complete();
	CResourceManager::GetResourceManager()->CompleteResource(this);
	return 0;
}

int CreationCommand::Submit()
{
	CRender * Render = CRender::GetRender();
	Render->CreateResource(this);
	return 0;
}
