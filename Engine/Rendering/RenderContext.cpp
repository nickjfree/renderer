#include "RenderContext.h"


RenderContext::RenderContext(RenderInterface * Interface_)
{
	Interface = Interface_;
	memset(Constants, 0, sizeof(void*)* MAX_CONSTANT);
	FrameWidth = FRAMEBUFFER_WIDTH;
	FrameHeight = FRAMEBUFFER_HEIGHT;
}


RenderContext::~RenderContext()
{
}

int RenderContext::RegisterConstant(String& Name, int Slot, unsigned int Size) {
	ConstantDesc * Constant = Constants[Slot];
	if (Constant) {
		return 1;
	} else {
		void * CPUData = new char[Size];
		Constant = new ConstantDesc();
		memset(Constant, 0, sizeof(ConstantDesc));
		Constant->CPUData = CPUData;
		Constant->Size = Size;
		Constant->Slot = Slot;
		Constant->Name = Name;
		// create this constant buffer
		Constant->Id = Interface->CreateConstantBuffer(Size);
		Constants[Slot] = Constant;
	}
	return 0;
}


int RenderContext::RegisterParameter(String& Name, int Slot, unsigned int Offset, unsigned int Size) {
	return 0;
}

int RenderContext::SetParameter(int Slot, void * CPUData, unsigned int Offset, unsigned int Size) {
	if (Slot >= 0) {
		ConstantDesc * Constant = Constants[Slot];
		if (Constant) {
			void * Dest = Constant->CPUData;
			memcpy((char*)Dest + Offset, CPUData, Size);
		}
	}
	return 0;
}

int RenderContext::UpdateConstant(int Slot) {
	if (Slot >= 0) {
		ConstantDesc * Constant = Constants[Slot];
		if (Constant) {
			int Id = Constant->Id;
			int Slot = Constant->Slot;
			int Size = Constant->Size;
			void * Data = Constant->CPUData;
			Interface->SetConstant(Slot, Id, Data, Size);
		}
	}
	return 0;
}

int RenderContext::RegisterRenderState(String& Name, int Id) {
	HashMap<String, int>::Iterator Iter;
	Iter = RenderStats.Find(Name);
	if (Iter == RenderStats.End()) {
		RenderStats[Name] = Id;
	}
	else {
		// render stat with same name already registered
		return 1;
	}
	return 0;
}

int RenderContext::GetRenderState(String& Name) {
	HashMap<String, int>::Iterator Iter;
	Iter = RenderStats.Find(Name);
	if (Iter != RenderStats.End()) {
		return (*Iter);
	}
	return -1;
}

int RenderContext::RegisterRenderTarget(String& Target, int Id) {
	HashMap<String, int>::Iterator Iter;
	Iter = RenderTarget.Find(Target);
	if (Iter == RenderTarget.End()) {
		RenderTarget[Target] = Id;
		Variant Var;
		Var.as<int>() = Id;
		SetResource(Target, Var);
	}
	else {
		// render stat with same name already registered
		return 1;
	}
	return 0;
}

int RenderContext::GetRenderTarget(String& Target) {
	HashMap<String, int>::Iterator Iter;
	Iter = RenderTarget.Find(Target);
	if (Iter != RenderTarget.End()) {
		return (*Iter);
	}
	return -1;
}

int RenderContext::SetResource(String& Name, Variant& resource) {
	Parameters[Name] = resource;
	return 0;
}

Variant* RenderContext::GetResource(String& Name) {
	Dict::Iterator Iter = Parameters.Find(Name);
	if (Iter != Parameters.End()) {
		return &(*Iter).Value;
	}
	return 0;
}