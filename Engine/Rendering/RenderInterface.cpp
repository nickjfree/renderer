#include "RenderInterface.h"
#include <stdio.h>

RenderInterface::RenderInterface()
{
}


RenderInterface::~RenderInterface()
{
}


int RenderInterface::Debug(char* info) {
	printf(info);
	return 0;
}
