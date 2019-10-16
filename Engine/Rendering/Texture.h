#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "GPUResource.h"


class Texture : public GPUResource
{
	OBJECT(Texture);
	BASEOBJECT(Texture);
	LOADEROBJECT(FileLoader);
	DECLAR_ALLOCATER(Texture);
private:
	// raw data in binary. dds file. texture shold take care of deallocation of this raw memoery
	void* Raw;
	int Size;
public:
	Texture(Context* context);
	~Texture();
	virtual int OnSerialize(Deserializer& deserializer);
	virtual int OnLoadComplete(Variant& Parameter);
	virtual int OnCreateComplete(Variant& Parameter);
	virtual int OnDestroy(Variant& Param);
};

#endif