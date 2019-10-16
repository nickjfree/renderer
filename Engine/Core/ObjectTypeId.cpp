#include "ObjectTypeId.h"



unsigned int ObjectTypeId::TypeId = 0;


ObjectTypeId::ObjectTypeId() : InstanceId(0) {
	ClassId = ++TypeId;
}


ObjectTypeId::~ObjectTypeId() {
}

unsigned int ObjectTypeId::GetId() {
	unsigned int Id = InstanceId++;
	Id = (ClassId << 24) + Id;
	return Id;
}