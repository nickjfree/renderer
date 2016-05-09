#ifndef __CREATION_CMD__
#define __CREATION_CMD__

#include "windows.h"
#include "Mathlib.h"
#include "RenderResource.h"

#define CMDBATCH   32

#define COPY_BUFFER  0x01
#define COPY_TEXTURE 0x02
#define COPY_BONE    0x03

using ResourceManager::CRenderResource;



namespace ResourceManager {


	class CreationCommand
	{
		public:
			CRenderResource * m_Resource;
			int CmdId;
			int CreateId;
			CreationCommand * NextBatch;
		public:
			CreationCommand();
			virtual ~CreationCommand();
			int Create(IMRender * Render);
			int Complete();
			int Submit();
	};


	typedef struct CopyCMD
	{
		int Type;
		int ID;
		int ShaderResourceID;
		CopyCMD * NextBatch;
	}CopyCMD;

} //end namespace

#endif
