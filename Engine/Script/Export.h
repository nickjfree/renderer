#ifndef __EXPORT_H__
#define __EXPORT_H__

#include "Proxy.h"

#include "Scene\GameObject.h"

BEGIN_PROXY(GameObject)
	METHOD(GetName, &GameObject::GetName)
	METHOD(SetName, (void (GameObject::*)(char *))&GameObject::SetName)
	METHOD(SetTranslation, &GameObject::SetTranslation)
	METHOD(GetTranslation, &GameObject::GetTranslation)
END_PROXY()


#endif 
