#ifndef __STR__
#define __STR__







#include <string.h>

#define SHORSTR_LENGTH 64


// Hash of String
class StringHash
{
private:
	// value
	unsigned int value;
private:
	unsigned int hash(const char * buff);

public:
	StringHash() {};
	StringHash(const char * buff);
	StringHash(StringHash& rh) { value = rh.value; };
	inline operator int();
	bool inline operator ==(StringHash& rh);
	bool inline operator !=(StringHash& rh);
	inline operator unsigned int();
	virtual ~StringHash();
};

// custom ascii string class used in engine, this is not string resource.
class String
{
	friend String;
private:
	// buffer for shor string
	char ShortStr[SHORSTR_LENGTH];
	// pointer to buffer contain string
	const char * Str;
	// string length
	unsigned int Length;
	StringHash Hash;
public:
	String();
	String(String& rh);
	String(const char * buff);
	String& operator=(String& rh);
	String& operator=(char * rh);
	operator int();
	operator unsigned int();
	operator char * ();
	bool operator == (char * buff);
	bool operator == (String& rh);
	bool operator != (char * buff);
	bool operator != (String& rh);
	int Split(char delimiter, String * Result, int Count);
	unsigned int Len() { return Length; }
 	virtual ~String();
};

#endif

