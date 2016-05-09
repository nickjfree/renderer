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
#include "Resource\H3DMesh.h"


#include "RenderEngine.h"

#include <conio.h>
#include <memory>

int _tmain(int argc, _TCHAR* argv[])
{
	
	// the following lines  simulate the engine init phase

	// RenderEngine Init
	CRenderEngine * RenderEngine = new CRenderEngine(GetCurrentThreadId());
	CSceneManager * SceneManager = RenderEngine->GetSceneManager();
	CMaterialManager * MaterialManager = RenderEngine->GetMaterialManager();
	CGameCamera * Camera = RenderEngine->CreateGameCamera();
	Vector3 pos(0, 0, 0);
	Vector3 up(0, 1, 0);
	Vector3 look(0, 0, 1);
	Camera->SetCamera(&pos, &look, &up, 1920 / 1080.0f);
	RenderEngine->SetGameCamera(Camera);

	// Top level init

	CMaterial * MaterialHDR = MaterialManager->CreateMaterial("hdr");
	CMaterial * MaterialPost = MaterialManager->CreateMaterial("post_screen");
	CSceneNode * Sprite = SceneManager->CreateSprite();
	Sprite->SetMaterial(MaterialPost, 0);
	SceneManager->AddToScene(Sprite, false);
	CSceneNode * SpriteHDR = SceneManager->CreateSceneHDR();
	SpriteHDR->SetMaterial(MaterialHDR, 0);
	SceneManager->AddToScene(SpriteHDR, false);
	
	//Set UP SSAO
	CSceneScreenLight * LightNode = SceneManager->CreateSceneScreenLight();
	CMaterial * SSAO = MaterialManager->CreateMaterial("ssao");
	LightNode->SetMaterial(SSAO, 0);
	SceneManager->AddToScene(LightNode, false);

	LightNode = SceneManager->CreateSceneScreenLight();
	CMaterial * Direction = MaterialManager->CreateMaterial("directionlight");
	LightNode->SetMaterial(Direction, 0);
	SceneManager->AddToScene(LightNode, false);

	//CMaterial * Material = MaterialManager->CreateMaterial("zealot");
	//CEntity * Entity = SceneManager->CreateEntity();
	//Entity->Create("unit.pack", "zealot", 1, 1);
	//CSceneObject * Object = SceneManager->CreateSceneObject();
	//Object->SetEntity(Entity, 0);
	//Object->SetMaterial(Material, 0);
	//Object->SetOrigin(0, 0, 1);
	//SceneManager->AddToScene(Object,false);
	//Entity->Load();

	// simulate gameobject with a meshrender component
	Context * context = new Context();
	// Create the Scene
	Scene *scene = new Scene(context);

	// Add a partition method
	QuadTree * Tree = new QuadTree(context);
	Tree->Construct(CRect2D(-100.0f, -100.0f, 100.0f, 100.0f), 10);
	scene->AddComponent(Tree);
	Tree->SubscribeTo(scene, 300);
	// a health component of player1 listen to his parent
	GameObject * Player1 = scene->CreateGameObject("Player1");
	GameObject * Zealot = scene->CreateGameObject("Zealot");
	TestComponent * Health = new TestComponent(context);
	Component * Collision = new Component(context);
	Player1->AddComponent(Health);
	Player1->AddComponent(Collision);
	Health->SubscribeTo(Player1, 100);

	// eg. a zealot attack player1 twice. so send a event to player1, if corrent player1 drop 16 hit points
	Event * Ev = Event::Create();
	Ev->EventId = 100;
	Ev->EventParam["param1"].as<int>() = 16;
	Ev->EventParam["attacker"].as<GameObject*>() = Zealot;

	int param1 = Ev->EventParam["param1"].as<int>();
	printf("%d %x\n", param1, Zealot);
	Zealot->SendEvent(Player1, Ev);

	Collision->BroadCast(Ev);

	Health->UnSubscribe(Player1, 100);

	Zealot->SendEvent(Player1, Ev);

	Ev->Recycle();

	Mesh * mesh = new H3DMesh(context);
	mesh->Load();
	MeshRenderer * Renderer = new MeshRenderer(context);
	Model * model = new Model(context);
	Zealot->AddComponent(model);
	Zealot->AddComponent(Renderer);
	model->SetMesh(mesh, 0);

	// simulate game loop
	float progress = 0;
	DWORD Pro = 0;
	DWORD OldTime, NewTime, DeltaTime;
	OldTime = GetCurrentTime();

	while (1)
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			Vector3 EyePos = CRenderEngine::GetRenderEngine()->GetEyePos();
			if (msg.message == WM_KEYUP)
			{
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//printf("msg %d\n",msg.message);
		}
		//Sleep(10);


		NewTime = GetCurrentTime();
		DeltaTime = NewTime - OldTime;
		OldTime = NewTime;
		Camera->KeyMove(DeltaTime);

		RenderEngine->SetGameCamera(Camera);
		RenderEngine->RenderScenes(DeltaTime);
		//		RenderEngine->RenderFrame();
	}
	while (_getch() != 27);
	return 0;
}

