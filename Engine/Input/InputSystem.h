#ifndef __INPUT_SYSTEM__
#define __INPUT_SYSTEM__

#include "Core\System.h"
#include "ActionCode.h"
#include "InputHandler.h"


class InputHandler;

class InputSystem : public System {

	BASEOBJECT(System)
		OBJECT(InputSystem)

private:
	// actions status
	Action Actions[ACT_MAX];
	// input handler
	InputHandler* Handler;
private:
	// clear key flags
	void ClearKeyTrigers();
public:
	InputSystem(Context* context);
	virtual ~InputSystem();

	int OnWindowsMessage(DWORD Msg, size_t lParam, size_t wParam);

	virtual int Initialize();

	virtual int Update(int ms);

	virtual int Shutdown();

	virtual int SetAction(Action& action);

	int GetAction(int Code);
};

#endif

