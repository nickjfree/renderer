#ifndef __MAPPINGLOADER__
#define __MAPPINGLOADER__



#define MAX_PACK_MAPPING  32
#define MAX_ENTRY         10240

#include <Windows.h>
#include "Tasks\Mutex.h"

typedef struct FileEntry
{
	char  name[32];
	DWORD offset;
}FileEntry;


typedef struct FileMapping
{
	int            Ref;
	HANDLE         hFile;
	HANDLE         hMapping;
	void *         Data;
	FileEntry *   Entry;
	char *         FileName;
}FileMapping;


class FileLoader
{
private:
	// FileMappings
	FileMapping   m_Mapping[MAX_PACK_MAPPING];
	// Mapping Mutext
	Mutex MapMutex;
public:
	FileLoader(void);
	~FileLoader(void);
	void * GetMappingBase(char * FileName);
private:
	unsigned int hash(char * str);
public:
	void * GetFileHeader(void  * Base, char * Name);
private:
	int FindEntry(char * Name);
public:
	// decrease ref of mapping
	int PutMappingBase(char * FileName);
};



#endif