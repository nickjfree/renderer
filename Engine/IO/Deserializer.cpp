#include "Deserializer.h"
#include "stdio.h"
#include "windows.h"
#include <type_traits>

Deserializer::Deserializer() : Data(0), Size(0), NeedClear(false) {
}


Deserializer::~Deserializer() {
	Release();
}

Deserializer::Deserializer(void* Data_) : NeedClear(false), Size(0) {
	Data = Data_;
}


Deserializer::Deserializer(const String& URL) : NeedClear(true) {
	String Paths[3];
	URL.Split('\\', Paths, 3);
	String& Pack = Paths[1];
	String& File = Paths[2];
	char file[256];
	sprintf_s(file, 256, "%s\\%s", Pack.ToStr(), File.ToStr());
	// read the file data in
	HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	DWORD SizeHigh = 0, FileLen, Read, DataLen;
	FileLen = GetFileSize(hFile, &SizeHigh);
	DataLen = 17 - FileLen % 16 + FileLen;
	char* Raw = new char[DataLen];
	//read it,set data to zero
	memset(Raw, 0, DataLen);
	ReadFile(hFile, Raw, FileLen, &Read, NULL);
	CloseHandle(hFile);
	// store the raw data
	Data = Raw;
	Size = FileLen;
}

Deserializer::Deserializer(FileMapping& mapping, void* Data_) : NeedClear(false) {
	mapping_ = mapping;
	Data = Data_;
	Size = 0;
}

Deserializer& Deserializer::operator=(Deserializer&& rh) noexcept {
	// release self befoen accepting new data
	Release();
	Data = rh.Data;
	Size = rh.Size;
	NeedClear = rh.NeedClear;
	mapping_ = std::move(rh.mapping_);
	// rh is not need to clear
	rh.NeedClear = false;
	rh.Data = nullptr;
	rh.Size = 0;
	return *this;
}

Deserializer::Deserializer(Deserializer&& rh) noexcept {
	// release self befoen accepting new data
	Release();
	Data = rh.Data;
	Size = rh.Size;
	NeedClear = rh.NeedClear;
	// rh is not need to clear
	rh.NeedClear = false;
	rh.Data = nullptr;
	rh.Size = 0;
}

// release raw resource
void Deserializer::Release() {
	if (NeedClear) {
		delete Data;
	}
	mapping_.Release();
	NeedClear = false;
	Data = nullptr;
	Size = 0;
}