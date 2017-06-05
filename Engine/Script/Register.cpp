#include "Proxy.h"
#include "Export.h"


EXPORT_CLASS(GameObject)
	EXPORT(GameObject, SetName)
	EXPORT(GameObject, GetName)
	EXPORT(GameObject, SetTranslation)
	EXPORT(GameObject, GetTranslation)
	EXPORT(GameObject, SetRotation)
	EXPORT(GameObject, GetRotation)
	EXPORT(GameObject, Subscribe)
	EXPORT(GameObject, SendEvent)
	EXPORT(GameObject, AddComponent)
	EXPORT(GameObject, Walk)
	EXPORT(GameObject, Strife)
	EXPORT(GameObject, Ascend)
	EXPORT(GameObject, Pitch)
	EXPORT(GameObject, Yaw)
	EXPORT(GameObject, Roll)
END_EXPORT()

EXPORT_CLASS(Scene)
	EXPORT(Scene, CreateGameObject)
	EXPORT(Scene, CreateComponent)
END_EXPORT()

EXPORT_CLASS(Level)
	EXPORT(Level, GetModel)
	EXPORT(Level, GetMaterial)
	EXPORT(Level, GetScene)
END_EXPORT()

EXPORT_CLASS(Model)
END_EXPORT()

EXPORT_CLASS(MeshRenderer)
	EXPORT(MeshRenderer, SetModel)
	EXPORT(MeshRenderer, SetMaterial)
END_EXPORT()

EXPORT_CLASS(Material)
	EXPORT(Material, GetUrl)
END_EXPORT()

EXPORT_CLASS(InputSystem)
	EXPORT(InputSystem, GetAction)
END_EXPORT()
