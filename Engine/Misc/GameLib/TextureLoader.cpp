#include "Include\TextureLoader.h"


using namespace ResourceManager;

CTextureLoader * CTextureLoader::m_ThisLoader = NULL;

CTextureLoader::CTextureLoader(void)
{
	m_ThisLoader = this;
}


CTextureLoader::~CTextureLoader(void)
{
}



int CTextureLoader::LoadTexture(CTexture * Texture)
{
	// only a test loader
	int Scale;
	switch(Texture->m_StreamType)
	{
	case CTexture::STREAM_HNO:
		memset(Texture->m_RAMDatas, 0, 8 * sizeof(char*));
		Texture->m_RAMDatas[0] = new float[512*512];
		memset(Texture->m_RAMDatas[0], 0, 512*512*sizeof(float));
		for(int i = 0;i<512*512;i++)
		{
			((float*)Texture->m_RAMDatas[0])[i] = 0;// rand() % 4;
			if (i > 512 * 128)
			{
				((float*)Texture->m_RAMDatas[0])[i] = 9;
			}
		}
		for (int i = 1; i<511; i++)
		{
			for (int j = 1; j < 511; j++)
			{
				float a, b, c, d;
				a = ((float*)Texture->m_RAMDatas[0])[(i - 1) * 512 + j];
				b = ((float*)Texture->m_RAMDatas[0])[(i + 1) * 512 + j];
				c =((float*)Texture->m_RAMDatas[0])[i * 512 + (j + 1) ] ;
				d = ((float*)Texture->m_RAMDatas[0])[i * 512 + (j -1)];
				((float*)Texture->m_RAMDatas[0])[i * 512 + j] = (a + b + c + d) / 4.0f;
			}
			
		}
		Texture->m_Scale = 512;
		Texture->m_FreeIndex = 0;
		Scale = Texture->m_Scale;
		while(Scale > 64)
		{
			Scale = Scale >> 1;
			Texture->m_FreeIndex++;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int CTextureLoader::UnLoadTexture(CTexture * Texture)
{
	switch(Texture->m_StreamType)
	{
	case CTexture::STREAM_HNO:
		// a test too
		delete Texture->m_RAMDatas[0];
		Texture->m_Scale = 512;
		memset(Texture->m_RAMDatas, 0, 8 * sizeof(char*));
	default:
		return -1;
	}
	return 0;
}


int CTextureLoader::LoadResource(int ID)
{
	CTexture * Texture = m_Pool.GetResourceByID(ID);
	if (Texture) {
		return LoadTexture(Texture);
	}
	return 0;
}

int CTextureLoader::UnLoadResource(int ID)
{
	return 0;
}

CRenderResource * CTextureLoader::MatchFreeResource(CRenderResource * Resource)
{
	return NULL;
}

CRenderResource * CTextureLoader::AllocResource(ResourceDesc * Desc)
{
	CTexture * Texture;
	int ID = m_Pool.AllocResource(&Texture);
	Texture->m_ResourceID = ID;
	Texture->BindStreamFile(Desc->File, Desc->Name, Desc->SubIndex);
	if (Desc->Name && !strcmp(Desc->Name, "heightmap")) {
		Texture->m_StreamType = CTexture::STREAM_HNO;
	}
	if (Desc->TextureFileList) {
		memcpy(Texture->m_Files, Desc->TextureFileList, 8 * sizeof(char*));
	}
	Texture->m_Desc = *Desc;
	return Texture;
}

int CTextureLoader::UseResource(int ID)
{
	CTexture * Texture = m_Pool.GetResourceByID(ID);
	if (Texture && Texture->m_Status == RES_USING) {
		return Texture->m_ResourceIDVRAM;
	}
	return -1;
}

CRenderResource * CTextureLoader::GetResource(int ID)
{
	CTexture * Texture = m_Pool.GetResourceByID(ID);
	if (Texture) {
		return Texture;
	}
	return NULL;
}
