#include "FileMappingLoader.h"
#include "stdio.h"


FileMappingLoader::FileMappingLoader(Context* context) : ResourceLoader(context) {
}

FileMappingLoader::~FileMappingLoader(void)
{
}

FileMapping FileMappingLoader::GetMapping(const char* FileName) {
	return FileMapping::CreateMapping(FileName);
}

unsigned int FileMappingLoader::hash(const char* str)
{
	register unsigned int h = 0;
	register unsigned char* p;
	for (h = 0, p = (unsigned char*)str; *p; p++)
		h = 31 * h + *p;
	return h;
}

void* FileMappingLoader::GetFileHeader(void* Base, const char* Name)
{
	int Index = FindEntry(Name);
	FileEntry* Entry = (FileEntry*)((char*)Base + 4);
	while (strcmp(Entry[Index].name, Name))
	{
		Index++;
		Index = Index % MAX_ENTRY;
		printf("index %d\n", Index);
	}
	void* Header = Entry[Index].offset + (char*)Entry - 4;
	return Header;
}

int FileMappingLoader::FindEntry(const char* Name)
{
	return hash(Name) % MAX_ENTRY;
}

Deserializer FileMappingLoader::GetDeserializer(const String& URL) {
	String Paths[3];
	URL.Split('\\', Paths, 3);
	auto mapping = GetMapping(Paths[1].ToStr());
	void* Header = GetFileHeader(mapping.GetData(), Paths[2].ToStr());
	return Deserializer(mapping, Header);
}
