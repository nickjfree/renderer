#ifndef __P_QUADTREE__
#define __P_QUADTREE__

#include "PartitionMethod.h"
//#include "RenderEngine.h"

/*
    QuadTree partition, using H3D renderending
*/
class QuadTree : public PartitionMethod
{
private:
    // Quadtree
    //CQuadTree * Tree;
    //// RenderEngine
    //CRenderEngine * RenderEngine;
    //// SceneManager
    //CSceneManager * SceneManager;
    //// QuadTree Region
    //CRect2D Rect;
private:
    void Init();
public:
    QuadTree(Context * context);
    virtual ~QuadTree();
    //	int Construct(CRect2D& Rect, float accuracy);
    virtual int OnAttach(GameObject * GameObj);
    virtual int HandleEvent(Event * Evt);
};

#endif