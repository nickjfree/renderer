#ifndef __VIARIANT__
#define __VIARIANT__


#include "Str.h"


#define VIARIANT_SIZE 64


// viariant data types. most used in event parameters, message parameters

__declspec(align(16)) class Variant
{
public:
	unsigned char data[VIARIANT_SIZE] = {};      // raw data
public:
	template <class T>	inline T& as() {
		return (T&)data;
	};

	Variant& operator = (const Variant& rh) {
		memcpy(data, rh.data, VIARIANT_SIZE);
		return *this;
	}

	Variant& operator = (int rh) {
		*(int*)data = rh;
		return *this;
	}

	Variant& operator = (unsigned int rh) {
		*(unsigned int*)data = rh;
		return *this;
	}

	Variant& operator = (float rh) {
		*(float*)data = rh;
		return *this;
	}

	// assign a string to variant is not allowed
	Variant& operator = (String& rh) = delete;

	// assign struct to variant
	template <class T> inline Variant& operator = (const T& rh) {
		*(T*)data = rh;
		return *this;
	}

	// assign struct pointer to variant
	template <class T> inline Variant& operator = (const T* rh) {
		*(T**)data = rh;
		return *this;
	}
};

#endif