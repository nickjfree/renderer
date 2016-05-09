#include "Deserializer.h"


Deserializer::Deserializer()
{
}


Deserializer::~Deserializer()
{
}

Deserializer::Deserializer(void * Data, unsigned int Size) {
	this->Data = Data;
	this->Size = Size;
}
