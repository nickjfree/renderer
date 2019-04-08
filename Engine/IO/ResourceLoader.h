#ifndef __LOADER_H__
#define __LOADER_H__

/*
    basic loader.
*/
#include "Deserializer.h"
#include "Core\Str.h"
#include "Core\Object.h"


class ResourceLoader : public Object
{
public:
    ResourceLoader(Context * context);
    virtual ~ResourceLoader();
    // virtual functions
    virtual Deserializer  GetDeserializer(const String& URL) {
        return Deserializer();
    };
    virtual void Unload(String& URL) {};
};

#endif