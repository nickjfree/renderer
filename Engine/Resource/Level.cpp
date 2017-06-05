#include "Level.h"
#include "Rendering\GPUResource.h"
#include "Scene\Scene.h"
#include "Scene\TestComponent.h"
#include "Rendering\MeshRenderer.h"
#include "Rendering\QuadTree.h"
#include "Rendering\BasicPartition.h"
#include "Rendering\Camera.h"
#include "Rendering\Light.h"
#include "Resource\ResourceCache.h"

#include "Script\LuaStack.h"
#include "Script\Proxy.h"
#include "Script\Export.h"

#include "Input\InputSystem.h"
#include "Core\StringTable.h"

#include "Script\Script.h"

#include "Physics\PhysicsObject.h"


USING_ALLOCATER(Level);


Level::Level(Context * context): Resource(context), Loaded(0) {
}


Level::~Level()
{
}

int Level::InitModel() {
	for (int i = 0; i < NumModels; i++) {
		ModelEntry * Entry = &ModelEntries[i];
		Model * model = new Model(context);
		model->SetMesh(GetMesh(Entry->MeshGroup[0]), 0);
		Models.PushBack(model);
	}
	return 0;
}

int Level::CreateGameobject(ObjectEntry * Entry) {
	char * offset = (char*)Entry + sizeof(ObjectEntry);
	GameObject * Object = scene->CreateGameObject(Entry->Name);
	Object->SetTranslation(Entry->Position);
	Object->SetRotation(Entry->Rotation);
	int Components = Entry->NumComponents;
	while (Components--) {
		offset += CreateComponent(Object, (ComponentEntry*)offset);
	}
	// add to objects
	GameObjects.PushBack(Object);
	return offset - (char*)Entry;
}

int Level::CreateComponent(GameObject * Object, ComponentEntry * Entry) {
	Component * component = (Component*)context->CreateObject(String(Entry->TypeName));
	int Size = component->Load(Entry, this);
	Object->AddComponent(component);
	return Size;
}

int Level::InitGameObjects() {
	char * offset = (char*)ObjectEntries;
	for (int i = 0; i < NumObjects; i++) {
		offset += CreateGameobject((ObjectEntry*)offset);
	}
	return 0;
}

int Level::InitLevel() {
	scene = new Scene(context);
	// Add a partition method
	BasicPartition * Tree = new BasicPartition(context);
	//		Tree->Construct(CRect2D(-100.0f, -100.0f, 100.0f, 100.0f), 10);
	scene->AddComponent(Tree);
	Tree->SubscribeTo(scene, 300);
	return 0;
}

int Level::CreateScene() {
	int Objects = GameObjects.Size();
	for (int i = 0; i < Objects; i++) {
		// do nothing now
	}
	MainCamera = scene->CreateGameObject("MainCamera");
	Camera * camera = new Camera(context);
	MainCamera->AddComponent(camera);
	// add camera script

	// set translation
	MainCamera->SetTranslation(Vector3(0, 20, -50));
	GameObjects.PushBack(MainCamera);
	return 0;
}



int Level::OnSerialize(Deserializer& deserializer) {
	// parse level file
	unsigned int Size = deserializer.Length();
	char * Raw = (char*)deserializer.Raw();
	char * offset = Raw;
	// read meshes
	LevelHeader * Header = (LevelHeader*)offset;
	NumMeshes = Header->NumEntries;
	offset += sizeof(LevelHeader);
	MeshEntries = (MeshEntry *)offset;
	offset += sizeof(MeshEntry) * NumMeshes;
	// read materials
	Header = (LevelHeader*)offset;
	NumMaterials = Header->NumEntries;
	offset += sizeof(LevelHeader);
	MaterialEntries = (MatrialEntry *)offset;
	offset += sizeof(MatrialEntry) * NumMaterials;
	// read models
	Header = (LevelHeader*)offset;
	NumModels = Header->NumEntries;
	offset += sizeof(LevelHeader);
	ModelEntries = (ModelEntry *)offset;
	offset += sizeof(ModelEntry)* NumModels;
	// read objects
	Header = (LevelHeader*)offset;
	NumObjects = Header->NumEntries;
	offset += sizeof(LevelHeader);
	ObjectEntries = (ObjectEntry *)offset;
	// dependency count
	DepCount = NumMeshes + NumMaterials;
	return 0;
}

int Level::OnCreateComplete(Variant& Parameter) {
	ResourceCache * Cache = context->GetSubsystem<ResourceCache>();
	Variant Param;
	Mesh * empty_mesh = 0;
	Material * empty_material = 0;
	// submit resource creation task
	for (int i = 0; i < NumMeshes; i++) {
		Param.as<int>() = i;
		Meshs.PushBack(empty_mesh);    // init mesh vertor to zero
		Cache->AsyncLoadResource(String(MeshEntries[i].Url), this, Param);
	}

	for (int i = 0; i < NumMaterials; i++) {
		Param.as<int>() = i;
		Materials.PushBack(empty_material); // init material vector to zero
		Cache->AsyncLoadResource(String(MaterialEntries[i].Url), this, Param);
	}
	return 0;
}

int Level::OnSubResource(int Message, Resource * Sub, Variant& Param) {
	GPUResource * resource = (GPUResource*)Sub;
	int Index = Param.as<int>();
	if (resource->ResourceType == R_MESH) {
		printf("finish  mesh %s\n", (char*)resource->GetUrl());
		Meshs[Index] = (Mesh*)resource;
		DepCount--;
	}
	if (resource->ResourceType == R_MATERIAL) {
		printf("finish  material %s\n", (char*)resource->GetUrl());
		Materials[Index] = (Material*)resource;
		DepCount--;
	}
	if (!DepCount) {
		// init level
		printf("mesh create complete\nnow, create the scene\n");
		InitModel();
		InitLevel();
		InitGameObjects();
		CreateScene();
		InitScript();
		Loaded = 1;
	}
	printf("-------decount %d\n", DepCount);
	return 0;
}

int Level::InitScript() {
	static int flag = 0;
	if (flag) {
		return -1;
	}
	// send event to scripting subsystem
	Event * event = Event::Create();
	event->EventId = 400;
	event->EventParam[hash_string::Level].as<Level *>() = this;
	context->BroadCast(event);
	event->Recycle();
	// add camera script
	// attach a script component for test 
	Script * script = new Script(context);
	script->SetScript(String("F:\\proj\\Game11\\Game\\Engine\\Script\\test\\camera.lua"));
	MainCamera->AddComponent(script);
	// update test
	Vector<GameObject *>::Iterator Iter;
	for (Iter = GameObjects.Begin(); Iter != GameObjects.End(); Iter++) {
		GameObject * Object = *Iter;
		if (Object->GetName() == "ADATA") {
			// attach a script component for test 
			Script * script = new Script(context);
			script->SetScript(String("F:\\proj\\Game11\\Game\\Engine\\Script\\test\\script.lua"));
			Object->AddComponent(script);
		}
		if (Object->GetName() == "qianzhihe") {
			PhysicsObject * Physics = new PhysicsObject(context);
			Object->AddComponent(Physics);
		}
	}
	return 0;
}

void Level::Update(int ms) {

	// tesing 
	if (!Loaded) {
		return;
	}
	Vector<GameObject *>::Iterator Iter;
	float speed = 0.5f/1000.0f;
	for (Iter = GameObjects.Begin(); Iter != GameObjects.End(); Iter++) {
		GameObject * Object = *Iter;
		if (Object->GetName() == "qianzhihe") {
			Quaternion rotation = Quaternion();
			Vector3 Axis = Vector3(0, 1, 0);
			rotation.RotationAxis(Axis, ms * speed);
//			Object->SetRotation(Object->GetRotation() * rotation);
		}
	}
	// update the scene
	scene->Update(ms);
}