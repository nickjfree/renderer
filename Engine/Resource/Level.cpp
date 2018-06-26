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


Level::~Level() {
}

int Level::InitModel() {
	for (int i = 0; i < NumModels; i++) {
		ModelEntry * Entry = &ModelEntries[i];
		Model * model = new Model(context);
		model->SetMesh(GetMesh(Entry->MeshGroup[0]), 0);
		model->SetName(Entry->Name);
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
	Tree->SubscribeTo(scene, EV_NODE_ADD);
	Tree->SubscribeTo(scene, EV_NODE_REMOVE);
	return 0;
}

int Level::CreateScene() {
	// for test
	LoadAnime();

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
		if (Object->GetName() == "Lumia") {
			// attach a script component for test 
			Script * script = new Script(context);
			script->SetScript(String("F:\\proj\\Game11\\Game\\Engine\\Script\\test\\script.lua"));
			Object->AddComponent(script);
		}
		if (Object->GetName() == "qianzhihe" || Object->GetName() == "ADATA") {
			PhysicsObject * Physics = new PhysicsObject(context);
			// creat convext hulls for collision shape
			MeshRenderer * render = (MeshRenderer * )Object->GetComponent(String("Renderer"));
			Model * model = render->GetModel();
			Physics->CreateShapeFromModel(model);
			Object->AddComponent(Physics);
		}
		if (Object->GetName() == "Lumia") {
			PhysicsObject * Physics = new PhysicsObject(context);
			// creat convext hulls for collision shape
			MeshRenderer * render = (MeshRenderer *)Object->GetComponent(String("Renderer"));
			Model * model = render->GetModel();
			Physics->CreateShapeFromModel(model);
			Physics->SetObjectType(PhysicsObject::KINEMATIC);
			Object->AddComponent(Physics);
			render->SetMatrixPalette(Palette, 52);
		}
		if (Object->GetName() == "Light2" || Object->GetName() == "LightProb") {
			// global light should follow camera
			MainCamera->Attach(Object);
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
	// debug physics
	PhysicsSystem * Physics = context->GetSubsystem<PhysicsSystem>();
	//// for debug hack. this shold be done with camera scripts to set debug window view
	Physics->SetDebugView(MainCamera->GetLook(), MainCamera->GetUp(), MainCamera->GetRight(), MainCamera->GetWorldTranslation());
}

void Level::ListModels() {
	int Size = Models.Size();
	for (int i = 0; i < Size; i++) {
		Model * model = Models[i];
		printf("%d\t%s\n", i, model->GetName());
	}
}

void Level::LoadAnime() {
	DWORD len;
	HANDLE hFile = CreateFileA("bone", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	ReadFile(hFile, bones, sizeof(BoneInfo) * 52, &len, 0);
	CloseHandle(hFile);
	// load 0 frame
	hFile = CreateFileA("anime", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	ReadFile(hFile, Frame, sizeof(TestFrame) * 51, &len, 0);
	CloseHandle(hFile);
	// calculate palette
	TestBone testbones[52];
	for (int i = 0; i < 52; i++) {
		Matrix4x4::Tranpose(bones[i].offsetMatrix, &testbones[i].InvertBind);
		// get trans matrix from anime data
		Matrix4x4 Mat;
		Mat.Identity();
		for (int j = 0; j < 51; j++) {
			TestFrame &frame = Frame[j];
			if (frame.BoneId == i) {
				Vector3 Trans(frame.Translation[0], frame.Translation[1], frame.Translation[2]);
				Quaternion Rot;
				Rot.x = frame.Rotation[0];
				Rot.y = frame.Rotation[1];
				Rot.z = frame.Rotation[2];
				Rot.w = frame.Rotation[3];
				Rot.x = frame.Rotation[0];
				Mat = Matrix4x4::FormPositionRotation(Trans, Rot);
			}
		}
		testbones[i].Local = Mat;
		testbones[i].BoneId = i;
		testbones[i].Parent = bones[i].parent;
		testbones[i].Updated = 0;
	}

	for (int i = 0; i < 52; i++) {
		if (!testbones[i].Updated) {
			// make sure parent is updated
			int parent = testbones[i].Parent;
			testbones[i].Global = testbones[i].Local;
			while (parent!=-1) {
				testbones[i].Global = testbones[i].Global * testbones[parent].Local;
				parent = testbones[parent].Parent;
			}
			testbones[i].Updated = 1;
		}
	}
	// print bone global position
	for (int i = 0; i < 52; i++) {
		Vector3 pos = Vector3(0, 0, 0) * testbones[i].Global;
		printf("%d  %f %f %f\n", i, pos.x, pos.y, pos.z);
	}

	// save palette
	for (int i = 0; i < 52; i++) {
		Vector3 pos = Vector3(0, 0, 0) * testbones[i].InvertBind * testbones[i].Global;
		printf("inv: %d  %f %f %f\n", i, pos.x, pos.y, pos.z);
		testbones[i].Global = testbones[i].InvertBind * testbones[i].Global;
		Matrix4x4::Tranpose(testbones[i].Global, &Palette[i]);
	}
}