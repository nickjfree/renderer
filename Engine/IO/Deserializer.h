#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "../Core/Str.h"

class Deserializer
{
private:
    // data
	void * Data;
    // data size
    unsigned int Size;
    // flag
    bool NeedClear;
public:
	Deserializer();
    // construct by outer data
	Deserializer(void * Data);
    // construct by resource url
    Deserializer(const String& URL);
	// detor
    virtual ~Deserializer();
    // move operation
    Deserializer(Deserializer&& rh);
    Deserializer& operator=(Deserializer&& rh);
    // raw data info
    void * Raw() { return Data; };
	unsigned int Length() { return Size; }
    // release raw resource
    void Release();
};

#endif