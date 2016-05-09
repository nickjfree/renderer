#ifndef __XML_PARSER__
#define __XML_PARSER__


#include "rapidxml.hpp"

using namespace rapidxml;

namespace XML {

typedef xml_node<> * xml_context;

class CXMLParser
{
private:
	xml_document<> m_doc;
	xml_context m_context;
	char * m_FileData;
	unsigned long m_DataLen;
public:
	CXMLParser(void);
	virtual ~CXMLParser(void);
	int Parse(char * Path);
private:
	int Load(char * File);
public:
	char * GetString(char * XMLPath);
	int GetInt(char * XMLPath);
	float GetFloat(char * XMLPath);
	char * GetAttribString(xml_context context,char * XMLPath, char * Attrib);
	int GetAttribInt(xml_context context,char * XMLPath, char * Attrib);
	float GetAttribFloat(xml_context context,char * XMLPath, char * Attrib);
	xml_context GetFirstContext(xml_context context,char * XMLPath);
	xml_context GetNextContext(xml_context context);
};



} //end namespace 

#endif
