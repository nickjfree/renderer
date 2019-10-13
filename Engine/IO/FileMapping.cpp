#include "FileMapping.h"


List<FileMapping> FileMapping::Mappings_;
Mutex FileMapping::Lock_;

FileMapping::FileMapping(): mapping_(nullptr) {

}

FileMapping::~FileMapping() {
	if (mapping_) {
		Release();
	}
}


void FileMapping::Destory() {
	Lock_.Acquire();
	printf("unmapping file %s\n", mapping_->FileName);
	UnmapViewOfFile(mapping_->Data);
	CloseHandle(mapping_->hMapping);
	CloseHandle(mapping_->hFile);
	// delete mapping info
	delete mapping_;
	// remove control info from list
	for (auto iter = Mappings_.Begin(); iter != Mappings_.End(); iter++) {
		auto mapping = *iter;
		if (mapping->mapping_ == mapping_) {
			Mappings_.Remove(iter);
			break;
		}
	}
	Lock_.Release();
}

void FileMapping::Release() {
	if (mapping_) {
		auto ret = InterlockedDecrement(&mapping_->ref);
		if (ret == 0) {
			// no ref, delete mapping
			Destory();
		}
		// set mapping to nullptr
		mapping_ = nullptr;
	}
}

FileMapping::FileMapping(const FileMapping& rh): mapping_(nullptr) {
	if (rh.mapping_) {
		mapping_ = rh.mapping_;
		auto ret = InterlockedIncrement(&mapping_->ref);
	}
}

FileMapping& FileMapping::operator=(const FileMapping& rh) {
	if (mapping_) {
		Release();
	}
	if (rh.mapping_) {
		mapping_ = rh.mapping_;
		auto ret = InterlockedIncrement(&mapping_->ref);
	}
	return *this;
}

FileMapping& FileMapping::operator=(FileMapping&& rh) noexcept {
	if (mapping_) {
		Release();
	}
	if (rh.mapping_) {
		mapping_ = rh.mapping_;
		rh.mapping_ = nullptr;
	}
	return *this;
}

FileMapping& FileMapping::CreateMapping(const char* path) {
	Lock_.Acquire();
	auto& mapping = _CreateMapping(path);
	printf("mapping data %llx\n", mapping.mapping_->Data);
	Lock_.Release();
	return mapping;
}

FileMapping& FileMapping::_CreateMapping(const char* path) {
	
	

	for (auto iter = Mappings_.Begin(); iter != Mappings_.End(); iter++) {
		auto mapping = *iter;
		if (mapping && !strcmp(mapping->GetFileName(), path)) {
			// found return it
			printf("mapping data old %llx\n", mapping->mapping_->Data);
			return *mapping;
		}
	}
	// not found, ceate new one
	auto mappinginfo = new MappingInfo;
	strcpy_s(mappinginfo->FileName, path);
	mappinginfo->hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	mappinginfo->hMapping = CreateFileMapping(mappinginfo->hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	// new file mapping ref is 0
	mappinginfo->ref = 0;
	if (mappinginfo->hMapping == INVALID_HANDLE_VALUE || mappinginfo->hMapping == NULL)
	{
		printf("mapping failed------------\n");
		return FileMapping();
	}
	mappinginfo->Data = MapViewOfFile(mappinginfo->hMapping, FILE_MAP_READ, 0, 0, 0);
	// create a new one
	auto mapping = new FileMapping();
	mapping->mapping_ = mappinginfo;
	// insert to list
	Mappings_.Insert(mapping);
	printf("mapping data new %llx\n", mapping->mapping_->Data);
	return *mapping;
}

void * FileMapping::GetData() const {
	return mapping_->Data;
}