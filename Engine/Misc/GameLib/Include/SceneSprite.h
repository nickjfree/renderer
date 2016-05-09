#ifndef __SCENE_SPRITE__
#define __SCENE_SPRITE__

#include "SceneNode.h"
#include "vertextype.h"

namespace SceneManager{



class CSceneSprite : public CSceneNode 
{
protected:
	int                       m_Texture;
	vertex_dynamic_instancing m_Vertex[4];
	static WORD                      m_Index[6];
public:
	CSceneSprite(void);
	virtual ~CSceneSprite(void);
	virtual int Render(int LOD);
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


} // end namespace


#endif
