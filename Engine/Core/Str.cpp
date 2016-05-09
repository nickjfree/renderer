#include "Str.h"
#include "MathFunc.h"


String::String() :Length(0), Str(0)
{
}

String::String(char * buff) 
{
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
}

String::String(String& rh)
{
	char * buff = rh.Str;
	Length = strlen(rh.Str);
	if (Length < SHORSTR_LENGTH) {
		strcpy_s(ShortStr, SHORSTR_LENGTH, buff);
		Str = ShortStr;
	}
	else {
		// be carefull not to free buff from outsite
		Str = buff;
	}
	Hash = rh.Hash;
}

String& String::operator=(String& rh) {
	char * buff = rh.Str;
	if (!rh.Length) {
		return *this;
	}
	Length = rh.Length;
	if (Length < SHORSTR_LENGTH) {
		strcpy_s(ShortStr, SHORSTR_LENGTH, buff);
		Str = ShortStr;
	}
	else {
		// be carefull not to free buff from outsite
		Str = buff;
	}
	Hash = rh.Hash;
	return *this;
}

String& String::operator=(char * buff) {
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


String::operator char *()
{
	return Str;
}

String::operator int() {
	return Hash;
}

bool String::operator == (char * buff) {
	return !strcmp(Str, buff);
}

bool String::operator != (char * buff) {
	return strcmp(Str, buff);
}

int String::Split(char delimiter, String * Result, int Count) {
	int c = 0;
	int size = Length;
	int offset = 0;
	char buffer[1024];
	for (int i = 0; i <= Length; i++) {
		if (Str[i] == delimiter || Str[i] == '\0') {
			memcpy(buffer,Str + offset, i - offset);
			// terminate the string
			buffer[i - offset] = 0;
			Result[c] = String(buffer);
			offset = i + 1;
			c++;
			if (c == Count) {
				break;
			}
		}
	}
	return c;
}


String::~String()
{
}


StringHash::StringHash(char * buff)
{
	value = hash(buff);
}

StringHash::~StringHash()
{
}

StringHash::operator int()
{
	return (int)value;
};

StringHash::operator unsigned int()
{
	return value;
};

unsigned int StringHash::hash(char * buff)
{
	value = djb2_hash((unsigned char *)buff);
	return value;
}
