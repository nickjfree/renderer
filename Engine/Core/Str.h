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
	unsigned int hash(char * buff);

public:
	StringHash() {};
	StringHash(char * buff);
	StringHash(StringHash& rh) { value = rh.value; };
	inline operator int();
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
	char * Str;
	// string length
	int Length;
	StringHash Hash;
public:
	String();
	String(String& rh);
	String(char * buff);
	String& operator=(String& rh);
	String& operator=(char * rh);
	operator int();
	operator char * ();
	bool operator == (char * buff);
	bool operator != (char * buff);
	int Split(char delimiter, String * Result, int Count);
 	virtual ~String();
};

#endif

