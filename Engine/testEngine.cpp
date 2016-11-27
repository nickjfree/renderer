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

int _tmain(int argc, _TCHAR* argv[])
{
	//Vector3 center(0,0,0);
	//Vector3 extents(1, 1, 1);
	//AABB box(center, extents);

	//AABB box2(Vector3(0, 0, 0), Vector3(0.5, 0.5, 0.5));
	//
	//int ct = box.Contains(box2);
	//
	//Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(3.1416 * 0.15, 1920 / 1080.0, 1, 1000);

	//Frustum Fr = Frustum::CreateFromProjection(Projection);

	//Matrix4x4 Trans; 
	//Trans.Translate(0, 0, -1);

	//Fr.Transform(Trans);

	//ct = Fr.Contains(box);
	//ct = Fr.Contains(box2);


	//Engine initialize
	Engine * GameEngine = new Engine();
	GameEngine->Initialize();
	GameEngine->InitSubsystems();
	
//	// simulate gameobject with a meshrender component. this is simulate of the level loader
//	Context * context = new Context();
//	// Create the Scene
//	Scene *scene = new Scene(context);
//
//	// Add a partition method
//	QuadTree * Tree = new QuadTree(context);
////	Tree->Construct(CRect2D(-100.0f, -100.0f, 100.0f, 100.0f), 10);
//	scene->AddComponent(Tree);
//	Tree->SubscribeTo(scene, 300);
//	// a health component of player1 listen to his parent
//	GameObject * Player1 = scene->CreateGameObject("Player1");
//	GameObject * Zealot = scene->CreateGameObject("Zealot");
//	TestComponent * Health = new TestComponent(context);
//	Component * Collision = new Component(context);
//	Player1->AddComponent(Health);
//	Player1->AddComponent(Collision);
//	Health->SubscribeTo(Player1, 100);
//
//	// eg. a zealot attack player1 twice. so send a event to player1, if corrent player1 drop 16 hit points
//	Event * Ev = Event::Create();
//	Ev->EventId = 100;
//	Ev->EventParam["param1"].as<int>() = 16;
//	Ev->EventParam["attacker"].as<GameObject*>() = Zealot;
//
//	int param1 = Ev->EventParam["param1"].as<int>();
//	printf("%d %x\n", param1, (unsigned int)Zealot);
//	Zealot->SendEvent(Player1, Ev);
//
//	Collision->BroadCast(Ev);
//
//	Health->UnSubscribe(Player1, 100);
//
//	Zealot->SendEvent(Player1, Ev);
//
//	Ev->Recycle();

	// simulate level loading
	



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

