#ifndef __STR__
#define __STR__







#include <string.h>
#include "MathFunc.h"

#define SHORSTR_LENGTH 64


// Hash of String
class StringHash
{
private:
	// value
	unsigned int value;
private:
	unsigned int hash(const char* buff);

public:
	StringHash() : value(0) {};
	template <unsigned int N>  constexpr __forceinline StringHash(const char (&buff)[N]);
	StringHash(char* buff);
	StringHash(const StringHash& rh) { value = rh.value; };
	inline operator int() const;
	bool inline operator ==(const StringHash& rh) const;
	bool inline operator !=(const StringHash& rh) const;
	inline operator unsigned int() const;
};



template <unsigned int N>
__forceinline constexpr StringHash::StringHash(const char(&buff)[N]) : value(djb2_hash(buff))
{
}

// custom ascii string class used in engine, this is not string resource.
class String
{
	friend String;
private:
	// buffer for shor string
	char ShortStr[SHORSTR_LENGTH];
	// pointer to buffer contain string
	const char* Str;
	// string length
	size_t Length;
	StringHash Hash;
public:
	String();
	String(const String& rh) noexcept;
	String(String&& rh) noexcept;
	template <unsigned int N>
	__forceinline  constexpr String(const char(&buff)[N]);
	String(char* buff);
	String& operator=(const String& rh);
	//String& operator=(String&& rh);
	template <unsigned int N>
	String& operator=(const char(&buff)[N]);
	operator int() const;
	operator unsigned int() const;
	operator const StringHash() const;
	//operator char * ();
	//bool operator == (const char * buff);
	bool operator == (const String& rh) const;
	bool operator != (const String& rh) const;
	int Split(char delimiter, String* Result, int Count) const;
	size_t Len() const { return Length; }
	const char* ToStr() const { return Str; }
};

template <unsigned int N>
__forceinline constexpr String::String(const char(&buff)[N]) : Hash(buff), Length(N), Str(0), ShortStr()
{
	static_assert(N <= 64, "const string to too long");
	// auto index = Length;
	//while (index--) {
	//	ShortStr[index] = buff[index];
	//}
	Str = buff;
}

template <unsigned int N>
String& String::operator=(const char(&buff)[N]) {
	if (!buff) {
		return *this;
	}
	Length = strlen(buff);
	if (Length < SHORSTR_LENGTH) {
		strcpy_s(ShortStr, SHORSTR_LENGTH, buff);
		Str = ShortStr;
	}
	else {
		// be carefull not to free buff from outsite
		Str = buff;
	}
	Hash = StringHash(buff);
	return *this;
}


#endif

