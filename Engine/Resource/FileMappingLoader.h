#ifndef __MAPPINGLOADER__
#define __MAPPINGLOADER__



#define MAX_PACK_MAPPING  32
#define MAX_ENTRY         10240

#include <Windows.h>
#include "Tasks\Mutex.h"
#include "IO\ResourceLoader.h"
#include "IO\Deserializer.h"
#include "Core\Str.h"

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
	char          FileName[256];
}FileMapping;


class FileMappingLoader : public ResourceLoader
{
	OBJECT(FileMappingLoader);
	BASEOBJECT(ResourceLoader);
private:
	// FileMappings
	FileMapping   m_Mapping[MAX_PACK_MAPPING];
	// Mapping Mutext
	Mutex MapMutex;
public:
	FileMappingLoader(Context * context);
	~FileMappingLoader(void);
	void * GetMappingBase(char * FileName);
private:
	unsigned int hash(char * str);
public:
	void * GetFileHeader(void  * Base, char * Name);
	// decrease ref of mapping
	int PutMappingBase(char * FileName);
	virtual Deserializer GetDeserializer(String&  URL);
	virtual void Unload(String& URL);
private:
	int FindEntry(char * Name);

};



#endif