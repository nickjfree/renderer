#include "Str.h"
#include "MathFunc.h"


String::String() :Length(0), Str(0)
{
}

String::String(const char * buff)
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

String::String(const String& rh)
{
    const char * buff = rh.Str;
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

String::String(String&& rh) {
    const char * buff = rh.Str;
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

String& String::operator=(const String& rh) {
    const char * buff = rh.Str;
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

//String& String::operator=(String&& rh) {
//    const char * buff = rh.Str;
//    if (!rh.Length) {
//        return *this;
//    }
//    Length = rh.Length;
//    if (Length < SHORSTR_LENGTH) {
//        strcpy_s(ShortStr, SHORSTR_LENGTH, buff);
//        Str = ShortStr;
//    }
//    else {
//        // be carefull not to free buff from outsite
//        Str = buff;
//    }
//    Hash = rh.Hash;
//    return *this;
//}

String& String::operator=(const char * buff) {
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

String::operator int() const {
    return Hash;
}

String::operator unsigned int() const {
    return Hash;
}

//bool String::operator == (const char * buff) {
//	if (!Length) {
//		return false;
//	}
//	return !strcmp(Str, buff);
//}

bool String::operator == (const String& rh) const {
    if (!Length) {
        return false;
    }
    return Hash == rh.Hash;
}

bool String::operator != (const String& rh) const {
    return Hash != rh.Hash;
}

int String::Split(char delimiter, String * Result, int Count) const {
    int c = 0;
    int offset = 0;
    char buffer[1024];
    for (auto i = 0; i <= Length; i++) {
        if (Str[i] == delimiter || Str[i] == '\0') {
            memcpy(buffer, Str + offset, i - offset);
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


StringHash::StringHash(const char * buff)
{
    value = hash(buff);
}

StringHash::~StringHash()
{
}

StringHash::operator int() const
{
    return (int)value;
};

bool StringHash::operator==(const StringHash& rh) const {
    return value == rh.value;
}

bool StringHash::operator !=(const StringHash& rh) const {
    return value != rh.value;
}
StringHash::operator unsigned int() const
{
    return value;
};

unsigned int StringHash::hash(const char * buff)
{
    value = djb2_hash((unsigned char *)buff);
    return value;
}
