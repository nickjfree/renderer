#ifndef __FILE_MAPPING__
#define __FILE_MAPPING__

#include <windows.h>
#include <stdio.h>
#include "../Container/List.h"
#include "Tasks\Mutex.h"


typedef struct  MappingInfo
{
	unsigned int   ref;
	HANDLE         hFile;
	HANDLE         hMapping;
	void *         Data;
	char           FileName[256];
}MappingInfo;

class FileMapping 
{

private:
	MappingInfo* mapping_;
private:
	// unmap
	void Destory();
	// static values for keep track of mappings
	static Mutex Lock_;
	static List<FileMapping> Mappings_;
	// create mapping
	static FileMapping& _CreateMapping(const char* path);
public:
	FileMapping();
	~FileMapping();
	// create mapping
	static FileMapping& CreateMapping(const char* path);
	// copy constructor
	FileMapping(const FileMapping& rh);
	// assign 
	FileMapping& operator =(const FileMapping& rh);
	// move
	FileMapping& operator=(FileMapping&& rh) noexcept;
	// get data
	void * GetData() const;
	// get filename
	const char * GetFileName() const { return mapping_->FileName; }
	// release map
	void Release();
};


#endif