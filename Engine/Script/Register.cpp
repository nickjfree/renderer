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
EXPORT(GameObject, CreateGameObject)
EXPORT(GameObject, CreateObject)
EXPORT(GameObject, AddComponent)
EXPORT(GameObject, Walk)
EXPORT(GameObject, Strife)
EXPORT(GameObject, Ascend)
EXPORT(GameObject, Pitch)
EXPORT(GameObject, Yaw)
EXPORT(GameObject, Roll)
EXPORT(GameObject, Destroy)
EXPORT(GameObject, GetRef)
EXPORT(GameObject, GetObjectId)
EXPORT(GameObject, IsDestroyed)
EXPORT(GameObject, GetComponent)
EXPORT(GameObject, CreateComponent)
GC(GameObject)
END_EXPORT()

EXPORT_CLASS(Scene)
EXPORT(Scene, CreateGameObject)
EXPORT(Scene, CreateComponent)
EXPORT(Scene, SendEvent)
EXPORT(Scene, GetGameObject)
GC(Scene)
END_EXPORT()


EXPORT_CLASS(LevelLoader)
EXPORT(LevelLoader, LoadLevel)
EXPORT(LevelLoader, UnloadLevel)
END_EXPORT()


EXPORT_CLASS(Level)
EXPORT(Level, GetModel)
EXPORT(Level, GetMaterial)
EXPORT(Level, GetAnimation)
EXPORT(Level, GetSkeleton)

EXPORT(Level, GetScene)
EXPORT(Level, ListModels)
EXPORT(Level, Save)
END_EXPORT()

EXPORT_CLASS(Model)
GC(Model)
END_EXPORT()

EXPORT_CLASS(MeshRenderer)
EXPORT(MeshRenderer, SetModel)
EXPORT(MeshRenderer, SetMaterial)
GC(MeshRenderer)
END_EXPORT()

EXPORT_CLASS(Light)
EXPORT(Light, SetIntensity)
GC(Light)
END_EXPORT()

EXPORT_CLASS(Material)
EXPORT(Material, GetUrl)
GC(Material)
END_EXPORT()

EXPORT_CLASS(InputSystem)
EXPORT(InputSystem, GetAction)
GC(InputSystem)
END_EXPORT()

EXPORT_CLASS(PhysicsObject)
EXPORT(PhysicsObject, CreateShapeFromModel)
EXPORT(PhysicsObject, SetObjectType)
GC(PhysicsObject)
END_EXPORT()

EXPORT_CLASS(BlendingNode)
EXPORT(BlendingNode, SetParameter)
EXPORT(BlendingNode, SetAnimationClip)
GC(BlendingNode)
END_EXPORT()

EXPORT_CLASS(BlendingNode2)
EXPORT(BlendingNode2, SetParameter)
GC(BlendingNode2)
END_EXPORT()

EXPORT_CLASS(BlendingNode3)
EXPORT(BlendingNode3, SetParameter)
EXPORT(BlendingNode3, SetSyncCycle)
EXPORT(BlendingNode3, SetNode)
GC(BlendingNode3)
END_EXPORT()

EXPORT_CLASS(Animator)
EXPORT(Animator, GetBlendingNode)
EXPORT(Animator, SetSkeleton)
EXPORT(Animator, SetBlendingNode)
EXPORT(Animator, SetBlendShapeWeight)
GC(Animator)
END_EXPORT()

//EXPORT_CLASS(ResourceCache)
//	EXPORT(ResourceCache, AsyncLoadResource)
//	EXPORT(ResourceCache, AsyncUnLoadResource)
//END_EXPORT()