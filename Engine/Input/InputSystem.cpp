#include "InputSystem.h"



InputSystem::InputSystem(Context* context) : System(context) {
	Handler = new InputHandler(this);
}


InputSystem::~InputSystem() {
}

int InputSystem::Initialize() {
	return 0;
}

int InputSystem::Update(int ms) {
	// profile
	PIXScopedEvent(0xFF00FF00, __FUNCTION__);
	return 0;
}

int InputSystem::Shutdown() {
	return 0;
}

int InputSystem::OnWindowsMessage(DWORD Msg, size_t lParam, size_t wParam) {
	Handler->OnWindowsMessage(Msg, lParam, wParam);
	return 0;
}

int InputSystem::SetAction(Action& action) {
	Actions[action.Code] = action;
	return 0;
}

int InputSystem::GetAction(int Code) {
	return Actions[Code].Actived;
}

void InputSystem::ClearKeyTrigers() {
	for (int i = 0; i < ACT_MAX; i++) {
		Actions[i].Started = 0;
		Actions[i].Stoped = 0;
	}
}