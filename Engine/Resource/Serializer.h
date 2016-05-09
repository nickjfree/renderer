#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__


class Serializer
{
private:
	void * Data;
public:
	Serializer();
	virtual ~Serializer();
	void * Raw() { return Data; };
};

#endif