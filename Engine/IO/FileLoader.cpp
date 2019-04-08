#include "FileLoader.h"
#include "stdio.h"

FileLoader::FileLoader(Context * context) : ResourceLoader(context)
{
}


FileLoader::~FileLoader()
{
}

Deserializer FileLoader::GetDeserializer(const String& URL) {
    return Deserializer(URL);
}