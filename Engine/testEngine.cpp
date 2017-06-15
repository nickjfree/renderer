// Engine.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Scene/TestComponent.h"
#include "Core/ObjectFactory.h"
#include "Core/Variant.h"
#include "Core/EventNode.h"
#include "Scene/GameObject.h"
#include "Container/Vector.h"
#include "Container/Dict.h"
#include "Container/List.h"
#include "Container/HashMap.h"
#include "Scene\Scene.h"
#include "Rendering\QuadTree.h"

#include "Rendering\MeshRenderer.h"
#include "Rendering\Model.h"
#include "Rendering\H3DMesh.h"

#include "Core\Engine.h"
#include "Resource\ResourceCache.h"
#include "Resource\Resource.h"

#include "Math\LinearMath.h"
#include "Math\Collision.h"


//#include "RenderEngine.h"

#include <conio.h>
#include <memory>

int _tmain(int argc, _TCHAR* argv[]) {

	//Engine initialize
	Engine * GameEngine = new Engine();
	GameEngine->Initialize();
	GameEngine->InitSubsystems();
	

	// simulate game loop
	float progress = 0;
	DWORD Pro = 0;
	DWORD OldTime, NewTime, DeltaTime;
	OldTime = GetCurrentTime();

	while (1)
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//printf("msg %d\n",msg.message);
			GameEngine->OnMessage(msg.message, msg.lParam, msg.wParam);
		} else {
			//Sleep(10);
			NewTime = GetCurrentTime();
			DeltaTime = NewTime - OldTime;
			OldTime = NewTime;
			GameEngine->Update(DeltaTime);
		}
	}
	while (_getch() != 27);
	return 0;
}

