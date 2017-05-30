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


