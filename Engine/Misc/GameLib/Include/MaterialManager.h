#ifndef __MATERIAL_MANAGER__
#define __MATERIAL_MANAGER__

#include "Material.h"
#include "XMLParser.h"
#include "ResourceContainer.h"


//we also need a XML parser

using namespace ResourceManager;
using namespace XML;

namespace MaterialSystem {


// manage all the materials

// NOTE:  to get a shader index,we need to initialize a shader system first

class CMaterialManager
{
private:
	CResourceContainer<CMaterial> m_MaterialPool;
	CXMLParser                    m_XMLParser;
	char *                        m_Prefix;
	char *                        m_Postfix;
	// sharded marterial
	CMaterial *                   m_OccludeeMaterial;
public:
	CMaterialManager(void);
	~CMaterialManager(void);
	CMaterial * CreateDefaultMaterial(void);
	CMaterial * CreateMaterial(char * Name);
	int Init();
	CMaterial * GetOccludeeMaterial();
};

}// end namespace

#endif
