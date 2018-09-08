#include "FileMappingLoader.h"
#include "stdio.h"


FileMappingLoader::FileMappingLoader(Context * context) : ResourceLoader(context) {
	memset(m_Mapping,0,sizeof(FileMapping) * MAX_PACK_MAPPING);
}

FileMappingLoader::~FileMappingLoader(void)
{
}

void * FileMappingLoader::GetMappingBase(char * FileName)
{
	MapMutex.Acquire();
	FileMapping * Mapping = NULL;
	for(int i = 0;i < MAX_PACK_MAPPING;i++)
	{
		if(m_Mapping[i].FileName && !strcmp(m_Mapping[i].FileName, FileName) && m_Mapping[i].Ref)
		{
			m_Mapping[i].Ref++;
			MapMutex.Release();
			return m_Mapping[i].Data;
		}
		if(!m_Mapping[i].Ref)
		{
			// find a empty mapping field
			Mapping = &m_Mapping[i];
		}
	}	
	// the file is't mapped
	strcpy_s(Mapping->FileName, FileName);
	Mapping->hFile =  CreateFileA(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	Mapping->hMapping = CreateFileMapping(Mapping->hFile,NULL,PAGE_READONLY,0,0,NULL);
	if(Mapping->hMapping == INVALID_HANDLE_VALUE)
	{
		printf("mapping failed------------\n");
		return NULL;
	}
	Mapping->Data = MapViewOfFile(Mapping->hMapping,FILE_MAP_READ,0,0,0);
	Mapping->Entry = (FileEntry*)((char*)Mapping->Data + 4);
	Mapping->Ref = 1;
	MapMutex.Release();
	return Mapping->Data;
}

unsigned int FileMappingLoader::hash(char * str)
{
	register unsigned int h = 0;
	register unsigned char *p; 
	for(h=0, p = (unsigned char *)str; *p ; p++)
		h = 31 * h + *p; 
	return h;
}

void * FileMappingLoader::GetFileHeader(void * Base, char * Name)
{
	int Index = FindEntry(Name);
	FileEntry * Entry = (FileEntry*)((char *)Base + 4);
	while(strcmp(Entry[Index].name,Name))
	{
		Index++;
		Index = Index % MAX_ENTRY;
	}
	void * Header = Entry[Index].offset + (char*)Entry - 4;
	return Header;
}

int FileMappingLoader::FindEntry(char * Name)
{
	return hash(Name) % MAX_ENTRY;
}

// decrease ref of mapping
int FileMappingLoader::PutMappingBase(char * FileName)
{
	FileMapping * Mapping = NULL;
	MapMutex.Acquire();
	for(int i = 0;i < MAX_PACK_MAPPING;i++)
	{
		if(m_Mapping[i].FileName && !strcmp(m_Mapping[i].FileName,FileName) && m_Mapping[i].Ref)
		{
			m_Mapping[i].Ref--;
			Mapping = &m_Mapping[i];
			break;
		}
	}
	// ref reach 0 we release it
	if(!Mapping->Ref)
	{
		UnmapViewOfFile(Mapping->Data);
		CloseHandle(Mapping->hMapping);
		CloseHandle(Mapping->hFile);
	}
	MapMutex.Release();
	return 0;
}

Deserializer FileMappingLoader::GetDeserializer(String& URL) {
	String Paths[3];
	URL.Split('\\', Paths, 3);
	void * Base = GetMappingBase(Paths[1]);
	void * Header = GetFileHeader(Base, Paths[2]);
	return Deserializer(Header);
	//Pack = Paths[1];
	//File = Paths[2];
}


void FileMappingLoader::Unload(String& URL) {
	String Paths[3];
	URL.Split('\\', Paths, 3);
	PutMappingBase(Paths[1]);
}
