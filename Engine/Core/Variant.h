#ifndef __VIARIANT__
#define __VIARIANT__


#define VIARIANT_SIZE 64


// viariant data types. most used in event parameters, message parameters

__declspec(align(16)) class Variant
{
public:
    unsigned char data[VIARIANT_SIZE];      // raw data
public:
    template <class T>	inline T& as() {
        return (T&)data;
    };
	Variant() : data{0} {};
    ~Variant() {};
};

#endif