#ifndef __VIARIANT__
#define __VIARIANT__


#define VIARIANT_SIZE 64


// viariant data types. most used in event parameters, message parameters

class Viariant
{
public:
	unsigned char data[VIARIANT_SIZE];      // raw data
public:
	template <class T>	inline T& as() {
		return (T&)data;
	};
	Viariant() {};
	virtual ~Viariant() {};
};

#endif