#ifndef __FILE_LOADER_H__
#define __FILE_LOADER_H__

#include "ResourceLoader.h"
/*
    FileLoader. load whole file into memory
*/
class FileLoader : public ResourceLoader
{
    OBJECT(FileLoader);
    BASEOBJECT(ResourceLoader);
public:
    FileLoader(Context * context);
    ~FileLoader();
    Deserializer GetDeserializer(const String& URL);
};

#endif