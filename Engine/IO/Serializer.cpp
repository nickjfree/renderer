
#include "Serializer.h"


Serializer::Serializer(): hFile(INVALID_HANDLE_VALUE) {

}

Serializer::Serializer(const String& filePath){
	Create(filePath);
}

Serializer::~Serializer()
{
	// close file handle
	Close();
}


void Serializer::Create(const String& filePath) {
	hFile = CreateFileA(filePath.ToStr(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, NULL, NULL);
}


void Serializer::Write(void* data, unsigned int len) 
{
	WriteFile(hFile, data, len, NULL, NULL);
}

void Serializer::Flush() {
	FlushFileBuffers(hFile);
}

void Serializer::Close() {
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
}