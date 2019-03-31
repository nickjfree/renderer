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
	inline operator int() const;
	bool inline operator ==(const StringHash& rh) const;
	bool inline operator !=(const StringHash& rh) const;
	inline operator unsigned int() const;
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
	String(const String& rh);
    String(String&& rh);
	String(const char * buff);
	String& operator=(const String& rh);
    String& operator=(String&& rh);
	String& operator=(const char * rh);
	operator int() const;
	operator unsigned int() const;
	//operator char * ();
	//bool operator == (const char * buff);
	bool operator == (const String& rh) const;
	bool operator != (const String& rh) const ;
	int Split(char delimiter, String * Result, int Count);
	unsigned int Len() const { return Length; }
    const char * ToStr() const { return Str; } 
 	virtual ~String();
};

#endif

