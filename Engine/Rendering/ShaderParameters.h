#ifndef __SHADER_PARAMETERS__
#define __SHADER_PARAMETERS__

#include "Rendering/CommandBuffer.h"
#include "shaders/ShaderData.h"


template <class T>
class ShaderConstant : public T {
public:
	// get get slot
	const int Slot();
	// get sizee
	const unsigned int Size();
	// set constant
	void Update(RenderingCommand* cmd);
};

template <class T>
const int ShaderConstant<T>::Slot()
{
	return T::Slot();
}

template <class T>
const unsigned int ShaderConstant<T>::Size()
{
	return sizeof(T);
}

template <class T>
void ShaderConstant<T>::Update(RenderingCommand* cmd)
{
	ShaderParameterBinding binding{};
	binding.BindingType = ShaderParameterBinding::BindingType::CONSTANT;
	binding.Data = this;
	binding.Slot = T::Slot();
	binding.Size = sizeof(T);
	cmd->AddShaderParametes(binding);
	return;
}

#endif
