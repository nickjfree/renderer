#ifndef __LEVEL_RES__
#define __LEVEL_RES__

#include "Resource.h"
#include "Rendering\Material.h"
#include "Rendering\Model.h"

#include "Container\HashMap.h"
#include "Container\Vector.h"
#include "LevelDesc.h"


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
	Vector<GameObject*> GameObjects;

	// level infos
	int NumMeshes;
	MeshEntry * MeshEntries;
	int NumMaterials;
	MatrialEntry * MaterialEntries;
	int NumModels;
	ModelEntry * ModelEntries;
	int NumObjects;
	ObjectEntry * ObjectEntries;

	// scene
	Scene * scene;

	Mesh * mesh;
	Mesh * light_mesh;
	Material * material;
	Material * light_material;

private:
	int InitGameObjects();
	int InitLevel();
	int InitModel();
	int CreateGameobject(ObjectEntry * Entry);
	int CreateComponent(GameObject * Object, ComponentEntry * ComponentData);
	int CreateScene();
public:
	Level(Context* context);
	virtual ~Level();
	// parsing files
	virtual int OnSerialize(Deserializer& deserializer);
	// when parse file over
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int OnSubResource(int Message, Resource * Sub, Variant& Parameter);

	// init script
	virtual int InitScript();
	// get mesh
	Mesh * GetMesh(int Index) {	return Meshs[Index];};
	// get model
	Model * GetModel(int Index) { return Models[Index]; };
	// get material
	Material * GetMaterial(int Index) { return Materials[Index]; };
	// get all gameobjects
	Vector<GameObject *> & GetGameObjects() { return GameObjects; };
	// test method
	void Update(int ms);
};

#endif