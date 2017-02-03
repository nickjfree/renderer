#include "FileLoader.h"



FileLoader::FileLoader(void)
{
	memset(m_Mapping,0,sizeof(FileMapping) * MAX_PACK_MAPPING);
}

FileLoader::~FileLoader(void)
{
}

void * FileLoader::GetMappingBase(char * FileName)
{
	MapMutex.Acquire();
	FileMapping * Mapping = NULL;
	for(int i = 0;i < MAX_PACK_MAPPING;i++)
	{
		if(m_Mapping[i].FileName && !strcmp(m_Mapping[i].FileName,FileName) && m_Mapping[i].Ref)
		{
			m_Mapping[i].Ref++;
			return m_Mapping[i].Data;
		}
		if(!m_Mapping[i].Ref)
		{
			// find a empty mapping field
			Mapping = &m_Mapping[i];
		}
	}	
	// the file is't mapped
	Mapping->FileName = FileName;
	Mapping->hFile =  CreateFileA(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	Mapping->hMapping = CreateFileMapping(Mapping->hFile,NULL,PAGE_READONLY,0,0,NULL);
	if(Mapping->hMapping == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	Mapping->Data = MapViewOfFile(Mapping->hMapping,FILE_MAP_READ,0,0,0);
	Mapping->Entry = (FileEntry*)((char*)Mapping->Data + 4);
	Mapping->Ref = 1;
	MapMutex.Release();
	return Mapping->Data;
}

unsigned int FileLoader::hash(char * str)
{
	register unsigned int h = 0;
	register unsigned char *p; 
	for(h=0, p = (unsigned char *)str; *p ; p++)
		h = 31 * h + *p; 
	return h;
}

void * FileLoader::GetFileHeader(void * Base, char * Name)
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

int FileLoader::FindEntry(char * Name)
{
	return hash(Name) % MAX_ENTRY;
}

// decrease ref of mapping
int FileLoader::PutMappingBase(char * FileName)
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
