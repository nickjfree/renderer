#ifndef __TERRAIN_PAYLOAD__
#define __TERRAIN_PAYLOAD__


#include "../Resource/Resource.h"

class TerrainPayload : public Resource
{
	OBJECT(TerrainPayload);
	BASEOBJECT(TerrainPayload);
	LOADEROBJECT(FileLoader);
	DECLARE_ALLOCATER(TerrainPayload);

public:

	TerrainPayload(Context* context);

};


#endif
