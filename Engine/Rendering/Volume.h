#ifndef __VOLUME__
#define __VOLUME__


#include "Scene/Component.h"
#include "Rendering/Node.h"


class Volume: public Component
{
	OBJECT(Volume);
	BASEOBJECT(Volume);
	DECLARE_ALLOCATER(Volume);
public:
	// ctor
	Volume(Context* context);
	virtual ~Volume();
	// on attach
	int OnAttach(GameObject* GameObj);
	// on transform
	int OnTransform(GameObject* GameObj);
public:
	enum class VolumeType
	{
		GIVOLUME,
		COUNT,
	};

private:
	// volume Type
	VolumeType volumeType = VolumeType::GIVOLUME;
	// volume node
	Node* volumeNode = nullptr;
};

#endif

