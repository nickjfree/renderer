#include "Volume.h"
#include "GIVolume.h"

Volume::Volume(Context* context): Component(context)
{
	// defualt to gi volume for test
	volumeNode = new GIVolume();
}


Volume::~Volume()
{
	if (volumeNode) {
		delete volumeNode;
		volumeNode = nullptr;
	}
}