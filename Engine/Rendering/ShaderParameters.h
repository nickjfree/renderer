#ifndef __SHADER_PARAMETERS__
#define __SHADER_PARAMETERS__

#include "shaders/ShaderData.h"


class RenderCommandContext;
class RenderingCommand;
class CommandBuffer;

class ShaderParameters;

template <class T>
class ShaderConstant {

	friend CommandBuffer;

public:
	ShaderParameters& Set(T* data) {

		auto handler = [=] (RenderCommandContext* cmdContext) {
			cmdContext->UpdateConstantBuffer(T::Slot(), 0, data, sizeof(T));
			cmdContext->SetConstantBuffer(T::Slot(), sizeof(T));
		};
		cmd->AddShaderInput(handler);
		return cmd->shaderParameters;
	};
private:
	// rendering command
	RenderingCommand* cmd;
};


class ShaderParameters {

public:
	// shader paremeters
	union {
		// perframe constants
		ShaderConstant<PerFrameData> PerFrameConstant;
		// perobject constants
		ShaderConstant<PerObject> PerObjectConstant;
		// skinning matrics
		ShaderConstant<SkinningMatrices> SkinningMatrices;
	};
};



void inline UpdatePerframeConstant(RenderingCamera* cam, RenderContext* renderContext, PerFrameData& perFrameConstant)
{
	// per-frame constant 
	Matrix4x4::Tranpose(cam->GetInvertView(), &perFrameConstant.gInvertViewMaxtrix);
	Matrix4x4::Tranpose(cam->GetViewMatrix(), &perFrameConstant.gViewMatrix);
	Matrix4x4::Tranpose(cam->GetViewProjection(), &perFrameConstant.gViewProjectionMatrix);
	perFrameConstant.gViewPoint = cam->GetViewPoint();
	perFrameConstant.gScreenSize.x = static_cast<float>(renderContext->FrameWidth);
	perFrameConstant.gScreenSize.y = static_cast<float>(renderContext->FrameHeight);
	// update framenum and time
	if (perFrameConstant.gAbsoluteTime) {
		perFrameConstant.gTimeElapse = GetTickCount() - perFrameConstant.gAbsoluteTime;
	} else {
		perFrameConstant.gTimeElapse = 0;
	}
	perFrameConstant.gAbsoluteTime = GetTickCount();
	++perFrameConstant.gFrameNumber;
}


#endif
