#ifndef __DESERIALIZER_H__
#define __DESERIALIZER_H__

#include "../Core/Str.h"
#include "FileMapping.h"

class Deserializer
{
private:
	// data
	void* Data;
	// data size
	unsigned int Size;
	// flag
	bool NeedClear;
	// file mapping
	FileMapping mapping_;
public:
	Deserializer();
	// construct by outer data
	Deserializer(void* Data);
	// construct by resource url
	Deserializer(const String& URL);
	// construct by filemapping, and data
	Deserializer(FileMapping& mapping, void* Data);
	// detor
	virtual ~Deserializer();
	// move operation
	Deserializer(Deserializer&& rh) noexcept;
	// disable copy assignment
	Deserializer& operator=(Deserializer& rh) = delete;
	// move
	Deserializer& operator=(Deserializer&& rh) noexcept;
	// raw data info
	void* Raw() { return Data; };
	unsigned int Length() { return Size; }
	// release raw resource
	void Release();
};

#endif