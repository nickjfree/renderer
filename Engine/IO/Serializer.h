#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "../Core/Str.h"
#include <Windows.h>

/*
	save content to file
*/
class Serializer
{
private:
	// file to write
	HANDLE hFile;

public:
	// constrcutor
	Serializer();
	Serializer(const String& filePath);
	// destructor
	~Serializer();
    // create
	void Create(const String& filePath);

	// write data
	void Write(void* data, unsigned int len);

	// flush
	void Flush();

	// close
	void Close();
};


#endif