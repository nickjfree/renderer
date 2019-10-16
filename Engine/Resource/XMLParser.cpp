#include "XMLParser.h"


XMLParser::XMLParser()
{
}


XMLParser::~XMLParser()
{
}

xml_document<>* XMLParser::Parse(void* Data) {
	xml_document<>* doc = new xml_document<>();
	doc->parse<0>((char*)Data);
	return doc;
}
