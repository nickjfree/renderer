#include "FileLoader.h"
#include "stdio.h"

FileLoader::FileLoader(Context * context) : ResourceLoader(context)
{
}


FileLoader::~FileLoader()
{
}

Deserializer FileLoader::GetDeserializer(String& URL) {
	String Paths[3];
	URL.Split('\\', Paths, 3);
	String Pack = Paths[1];
	String File = Paths[2];
	char file[256];
	sprintf_s(file, 256, "%s\\%s", Pack.ToStr(), File.ToStr());
	// read the file data in,realloc if no space to store it
	//printf("create file %x\n", context);
	HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	DWORD SizeHigh = 0, FileLen, Read, DataLen;
	FileLen = GetFileSize(hFile, &SizeHigh);
	DataLen = 17 - FileLen % 16 + FileLen;
	char * Raw = new char[DataLen];
	//read it,set data to zero
	memset(Raw, 0, DataLen);
	ReadFile(hFile, Raw, FileLen, &Read, NULL);
	CloseHandle(hFile);
	return Deserializer(Raw, FileLen);
}