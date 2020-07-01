#ifndef __LEVEL_RES__
#define __LEVEL_RES__

#include "Resource.h"
#include "Rendering\Material.h"
#include "Rendering\Model.h"
#include "Animation\Animation.h"
#include "Animation\Skeleton.h"
#include "Animation\BlendShape.h"

#include "Container\HashMap.h"
#include "Container\Vector.h"
#include "LevelDesc.h"
#include "..\\IO\Serializer.h"

typedef struct BoneInfo {
	int parent;
	Matrix4x4 offsetMatrix;
}BoneInfo;

typedef struct TestFrame {
	int BoneId;
	float Translation[3];
	float Rotation[4];
}TestFrame;

typedef struct TestBone {
	int BoneId;
	int Parent;
	int Updated;
	int pad;
	Matrix4x4 Local;
	Matrix4x4 Global;
	Matrix4x4 InvertBind;
}TestBone;

class GameObject;
class Scene;

class Level : public Resource
{
	DECLAR_ALLOCATER(Level);
	LOADEROBJECT(FileLoader);
	OBJECT(Level);
	BASEOBJECT(Level);
private:
	// url to resource map
	HashMap<String, Resource*> Resources;
	// resource indics
	Vector<Mesh*> Meshs;
	Vector<Material*> Materials;
	Vector<Model*> Models;
	Vector<Animation*> Animations;
	Vector<Skeleton*> Skeletons;
	// blendshapes
	Vector<BlendShape*> BlendShapes;
	// game objects
	Vector<GameObject*> GameObjects;
	// test
	GameObject* MainCamera;
	// level infos
	// mesh
	int NumMeshes;
	MeshEntry* MeshEntries;
	// material
	int NumMaterials;
	MatrialEntry* MaterialEntries;
	// model
	int NumModels;
	ModelEntry* ModelEntries;
	// skeleton
	int NumSkeletons;
	SkeletonEntry* SkeletonEntries;
	// animation
	int NumAnimations;
	AnimationEntry* AnimationEntries;
	// blendshep
	int NumBlendShapes;
	BlendShapeEntry* BlendShapeEntries;

	int NumObjects;
	ObjectEntry* ObjectEntries;
	// loaded
	int Loaded;
	// destorying
	int Destorying;
	// scene
	Scene* scene;
	// serializer
	Serializer Serializer_;
public:
	// destroyed frames
	int DestroyedFrames;
private:
	int InitGameObjects();
	int InitModel();
	size_t CreateGameobject(ObjectEntry* Entry);
	int CreateComponent(GameObject* Object, ComponentEntry* ComponentData);
	int CreateScene();
public:
	Level(Context* context);
	virtual ~Level();
	// parsing files
	virtual int OnSerialize(Deserializer& deserializer);
	// when parse file over
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int OnSubResource(int Message, Resource* Sub, Variant& Parameter);

	// init script
	virtual int InitScript();
	// get mesh
	Mesh* GetMesh(int Index) { return Meshs[Index]; };
	// get model
	Model* GetModel(int Index) { return Models[Index]; };
	// get material
	Material* GetMaterial(int Index) { return Materials[Index]; };
	// get skeleton
	Skeleton* GetSkeleton(int Index) { return Skeletons[Index]; };
	// get animaton
	Animation* GetAnimation(int Index) { return Animations[Index]; };
	// get blendshape
	BlendShape* GetBlendShape(int Index) { return BlendShapes[Index]; };
	// get all gameobjects
	Vector<GameObject*>& GetGameObjects() { return GameObjects; };
	// get scene
	Scene* GetScene() { return scene; };
	// test method
	void Update(int ms);
	// list models on console
	void ListModels();
	// save level
	void Save(const String& file);
	// unload resource
	void Clear();
	// destroy level
	void Destroy();
	// ondestroyed
	virtual int OnDestroy(Variant& Data);
};

#endif