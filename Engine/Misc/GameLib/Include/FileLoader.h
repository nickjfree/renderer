#ifndef __LOADER__
#define __LOADER__



#define MAX_PACK_MAPPING  32
#define MAX_ENTRY         10240


namespace Loader{


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


class CFileLoader
{
private:
	FileMapping   m_Mapping[MAX_PACK_MAPPING];
public:
	CFileLoader(void);
	~CFileLoader(void);
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



} //end namespace


#endif