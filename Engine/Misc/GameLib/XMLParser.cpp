#include "Include\XMLParser.h"
#include <Windows.h>

using namespace XML;

CXMLParser::CXMLParser(void):m_FileData(0),m_DataLen(0)
{
}


CXMLParser::~CXMLParser(void)
{
	if(m_FileData)
	{
		delete m_FileData;
	}
}

// parse a file into a doc
int XML::CXMLParser::Parse(char * Path)
{
	m_doc.clear();
	Load(Path);
	m_doc.parse<0>(m_FileData);
	return 0;
}


int XML::CXMLParser::Load(char * File)
{
	// read the file data in,realloc if no space to store it
	HANDLE hFile =  CreateFileA(File,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	DWORD SizeHigh = 0,FileLen,Read,DataLen;
	FileLen = GetFileSize(hFile,&SizeHigh);
	DataLen = 17 - FileLen%16 + FileLen;
	if(DataLen > m_DataLen)
	{
		if(m_FileData)
		{
			delete m_FileData;
		}
		m_FileData = new char[DataLen];
		m_DataLen = DataLen;
	}
	//read it,set data to zero
	memset(m_FileData,0,m_DataLen);
	ReadFile(hFile,m_FileData,FileLen,&Read,NULL);
	CloseHandle(hFile);
	return 0;
}


char * XML::CXMLParser::GetString(char * XMLPath)
{
	char name[256];
	int len = strlen(XMLPath) + 1;
	int endpos = len - 1;
	int start = 0,end=len+1;
	xml_node<> *node = m_doc.first_node();
	for(int i = 0;i < len;i++)
	{
		if(XMLPath[i] == '/' || i == endpos)
		{
			end = i;
			memset(name,0,256);
			memcpy(name,&XMLPath[start],end-start);
			node = node->first_node(name);
			start = end + 1;
		}
	}
	return node->value();
}


int XML::CXMLParser::GetInt(char * XMLPath)
{
	char * value = GetString(XMLPath);
	return atoi(value);
}


float XML::CXMLParser::GetFloat(char * XMLPath)
{
	char * value = GetString(XMLPath);
	return atof(value);
}

//get attribut value, if Attrib==NULL,return the value of node,or NULL if no such attribute
char * XML::CXMLParser::GetAttribString(xml_context context,char * XMLPath,char * Attrib)
{
	char name[256];
	int len = strlen(XMLPath) + 1;
	int endpos = len - 1;
	int start = 0,end=len+1;
	xml_node<> *node = context;
	if(XMLPath[0])
	{

		for(int i = 0;i < len;i++)
		{
			if(XMLPath[i] == '/' || i == endpos)
			{
				end = i;
				memset(name,0,256);
				memcpy(name,&XMLPath[start],end-start);
				node = node->first_node(name);
				start = end + 1;
			}
		}
	}
	if(Attrib)
		return node->first_attribute(Attrib)->value();
	return node->value();
}

int XML::CXMLParser::GetAttribInt(xml_context context,char * XMLPath,char * Attrib)
{
	char * value = GetAttribString(context,XMLPath,Attrib);
	return atoi(value);
}

float XML::CXMLParser::GetAttribFloat(xml_context context,char * XMLPath,char * Attrib)
{
	char * value = GetAttribString(context,XMLPath,Attrib);
	return atof(value);
}


xml_context  XML::CXMLParser::GetFirstContext(xml_context context,char * XMLPath)
{
	char name[256];
	int len = strlen(XMLPath) + 1;
	int endpos = len - 1;
	int start = 0,end=len+1;
	xml_node<> *node = context?context:m_doc.first_node();
	for(int i = 0;i < len;i++)
	{
		if(XMLPath[i] == '/' || i == endpos)
		{
			end = i;
			memset(name,0,256);
			memcpy(name,&XMLPath[start],end-start);
			node = node->first_node(name);
			start = end + 1;
		}
	}
	return node;
}


xml_context  XML::CXMLParser::GetNextContext(xml_context  context)
{
	xml_context next_context;
	next_context = context->next_sibling();
	if(next_context && strcmp(next_context->name(),context->name()))
	{
		return NULL;
	}
	return next_context;
}
