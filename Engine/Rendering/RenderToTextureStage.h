#ifndef __RTT_STAGE__
#define __RTT_STAGE__

#include "RenderStage.h"


class RenderToTextureStage : public RenderStage {

public:
    RenderToTextureStage(RenderContext * context);
    virtual ~RenderToTextureStage();
};

#endif
