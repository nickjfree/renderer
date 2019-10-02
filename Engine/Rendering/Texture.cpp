#include "Texture.h"


USING_ALLOCATER(Texture);

Texture::Texture(Context * context) : GPUResource(context), Raw(0)
{
    ResourceType = R_TEXTURE;
}


Texture::~Texture()
{
}


int Texture::OnCreateComplete(Variant& Parameter) {
    return 0;
}

int Texture::OnLoadComplete(Variant& Paramener) {
    id = renderinterface->CreateTexture2D(NULL, Raw, Size, 0);
    // raw data can be released
    DeSerial.Release();
    return 0;
}

int Texture::OnSerialize(Deserializer& deserializer) {
    Raw = deserializer.Raw();
    Size = deserializer.Length();
    /*
        The dds format parsing job is not done here but in the renderiterface instead, which is not standard
    */
    return 0;
}

// destroy texture
int Texture::OnDestroy(Variant& Param) {
	renderinterface->DestroyTexture2D(id);
	return 0;
}