#ifndef __INPUT_HANDLER__
#define __INPUT_HANDLER__

#include "ActionCode.h"
#include "windows.h"
#include "InputSystem.h"

class InputSystem;

class InputHandler {

private:
    // KEYMAPPINGS
    InputSystem * Input;

private:
public:
    InputHandler(InputSystem * Input);
    virtual ~InputHandler();
    // handle messages
    int OnWindowsMessage(int msg, size_t lParam, size_t wParam);
};

#endif 
