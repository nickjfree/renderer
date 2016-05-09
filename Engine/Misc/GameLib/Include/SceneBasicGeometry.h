#ifndef __BASIC_GEOMETRY__
#define __BASIC_GEOMETRY__


#include "SceneNode.h"
#include "vertextype.h"

using SceneManager::CSceneNode;

namespace SceneManager {


typedef struct ShapeAttrib
{
	float Reserved;
	Matrix4x4 Translation;
}ShapeAttrib;


class CSceneBasicGeometry : public CSceneNode
{
private:
	ShapeAttrib m_ShapeAttrib;
public:
	CSceneBasicGeometry(void);
	virtual ~CSceneBasicGeometry(void);
	virtual int Render(int LOD);
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);

	virtual void Refresh(void);
};


// Box
class CSceneBox :
	public CSceneBasicGeometry
{
private:
	float m_HalfX,m_HalfY,m_HalfZ;
public:
	CSceneBox(void);
	virtual ~CSceneBox(void);
};



}//end namespace

#endif