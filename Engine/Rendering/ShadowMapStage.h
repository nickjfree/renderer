#ifndef __SHADOWMAP_STAGE__
#define __SHADOWMAP_STAGE__

#include "RenderStage.h"


#define SHADOWSTAGE_SHADOWMAPS (8)

class ShadowMapStage : public RenderStage {

private:
    // shadowmaps 
    int Targets[SHADOWSTAGE_SHADOWMAPS];
    // RenderViews for shadow map rendering
    Vector<RenderView*> RenderViews;
    // affected lights may create shadows
    Vector<Node*> Lights;

private:
    // initialization
    void Initialize();
    // create shadowmaps
    void CreateShadowMaps();
    // shadowpass
    int ShadowPass(RenderingCamera * Camera, Spatial * spatial, RenderQueue* renderQueue, WorkQueue * Queue, Vector<OsEvent*>& Events);


public:
    ShadowMapStage(RenderContext * Context);
    virtual ~ShadowMapStage();

    // execute stage
    virtual int Execute(RenderingCamera * Camera, Spatial * spatial, RenderQueue* renderQueue, WorkQueue * Queue, Vector<OsEvent*>& Events);
    // end
    virtual int End();
};

#endif 

