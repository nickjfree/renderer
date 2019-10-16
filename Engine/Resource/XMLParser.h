#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include "Misc\xml\rapidxml.hpp"



using namespace rapidxml;

class XMLParser
{
public:
	XMLParser();
	~XMLParser();
	xml_document<>* Parse(void* Data);
};

#endif

