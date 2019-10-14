#ifndef __MAPPINGLOADER__
#define __MAPPINGLOADER__



#define MAX_PACK_MAPPING  32
#define MAX_ENTRY         10240

#include "Tasks\Mutex.h"
#include "IO\ResourceLoader.h"
#include "IO\Deserializer.h"
#include "IO/FileMapping.h"
#include "Core\Str.h"
#include "../Container/LinkList.h"
#include "../Container/List.h"

typedef struct FileEntry
{
	char  name[32];
	DWORD offset;
} FileEntry;

class FileMappingLoader : public ResourceLoader
{
    OBJECT(FileMappingLoader);
    BASEOBJECT(ResourceLoader);
private:

public:
    FileMappingLoader(Context * context);
    ~FileMappingLoader(void);
	FileMapping GetMapping(const char* FileName);
private:
    unsigned int hash(const char * str);
public:
    void * GetFileHeader(void  * Base, const char * Name);
    virtual Deserializer GetDeserializer(const String&  URL);
private:
    int FindEntry(const char * Name);

};



#endif