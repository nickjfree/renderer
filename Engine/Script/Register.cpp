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


