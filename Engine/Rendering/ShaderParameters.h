#ifndef __SHADER_PARAMETERS__
#define __SHADER_PARAMETERS__

#include "shaders/ShaderData.h"


class RenderCommandContext;


template <class T>
class ShaderConstant;


template <class T>
class ShaderConstant<T*>: public ShaderInput {
private:
	T* data = nullptr;
public:

	// assign
	ShaderConstant<T*>& operator = (T* rh) {
		data = rh;
		return *this;
	};

	// pointer
	T* operator -> () {
		return data;
	};

	bool operator != (std::nullptr_t empty) {
		return data != nullptr;
	};
		
	bool operator == (ShaderConstant<T*>& rh) {
		return data == rh.data;
	};

	// apply
	virtual void Apply(RenderCommandContext* cmdContext) {
		cmdContext->UpdateConstantBuffer(T::Slot(), 0, data, sizeof(T));
		cmdContext->SetConstantBuffer(T::Slot(), sizeof(T));
	};
};


template <class T>
class ShaderConstant: public T, public ShaderInput 
{
public:
	// apply
	virtual void Apply(RenderCommandContext* cmdContext) {
		cmdContext->UpdateConstantBuffer(T::Slot(), 0, (T*)this, sizeof(T));
		cmdContext->SetConstantBuffer(T::Slot(), sizeof(T));
	};
};


#endif
