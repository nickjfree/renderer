#ifndef __VOLUME__
#define __VOLUME__


#include "Scene/Component.h"
#include "Rendering/Node.h"


class Volume: public Component
{
public:
	// ctor
	Volume(Context* context);
	virtual ~Volume();

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

