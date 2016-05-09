#ifndef __MATERIAL__
#define __MATERIAL__

#include "IMRender.h"
#include "XMLParser.h"
/*
light params
	diffuse
	specular
	ambient
	glow
textures  TextureID
	diffuse0
	diffuse1
	normal
	env
shaders
	shader catergory                  Illum,Glass,UI,sprite,Light,
lighting
	cast shadow
	cast light
trans
	transparent
*/

// materials is in ascript file ,say  zealot.m,then the iterpator ananlysis the file and make it in to a CMaterial class
// this will be complicated. I'll give it a try with Lua or Python.

using namespace XML;

namespace MaterialSystem {

class CMaterial
{
private:
	int      m_PreTechniqueId;            // light-pre pass
	int      m_PrePassId;                 // light-pre pass
	int      m_ShadowTechniqueId;        // shadow pass
	int      m_ShadowPassId;             // shadow pass
	int      m_TechniqueId;              //  normal pass
	int      m_PassId;                   // normal pass 
	int      m_MaterialID;               //  the id in resourcemanager
	int      m_TextureID;                // this is the id in resourcemanager
	int      m_TerrainID;                // terrain heightmap id
	int      m_ContextKey[CONTEXT_DIM];
	char     m_Name[64];
private:
	//texture files
	wchar_t     m_diffuse[256];
	wchar_t     m_specular[256];
	wchar_t     m_normal[256];
	wchar_t     m_cude[256];
	// flags
	bool        m_HasNormal;
public:
	int      ID;
private:
	int MakeFileList(wchar_t * String, wchar_t ** Files);  

public:
	CMaterial(void);
	~CMaterial(void);
	int Create(char * Name,CXMLParser * Parser);
	int * GetContextKey(int MeshID);
	int * GetPreContextKey(int MeshID);
	int * GetShadowContextKey(int MeshID);
	int SetTerrainID(int TerrainID) {m_TerrainID = TerrainID; return 0;};
};

} //end namespace 

#endif
