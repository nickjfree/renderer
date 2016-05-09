#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__


class Deserializer
{
private:
	void * Data;
public:
	unsigned int Size;
public:
	Deserializer();
	Deserializer(void * Data, unsigned int Size=0);
	virtual ~Deserializer();
	void * Raw() { return Data; };
	unsigned int Length() { return Size; }
	int Read(char * buffer, unsigned int size, unsigned int * read);
};

#endif