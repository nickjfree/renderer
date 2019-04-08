#include "InputHandler.h"

InputHandler::InputHandler(InputSystem * Input_) : Input(Input_) {
}


InputHandler::~InputHandler() {
}

int InputHandler::OnWindowsMessage(int msg, size_t lParam, size_t wParam) {

    Action action = {};
    int prev_stat;
    switch (msg) {


        // key downs
    case WM_KEYDOWN:
    case WM_KEYUP:
        //printf("key down %x\n", lParam);
        switch (wParam) {
        case 'W':
            action.Code = ACT_FORWARD;
            break;
        case 'S':
            action.Code = ACT_BACK;
            break;
        case 'A':
            action.Code = ACT_STRIF_LEFT;
            break;
        case 'D':
            action.Code = ACT_STRIF_RIGHT;
            break;
        case 'Q':
            action.Code = ACT_TRUN_LEFT;
            break;
        case 'E':
            action.Code = ACT_TURN_RIGHT;
            break;
        case VK_UP:
            action.Code = ACT_TURN_DOWN;
            break;
        case VK_DOWN:
            action.Code = ACT_TURN_UP;
            break;
        case VK_LEFT:
            action.Code = ACT_ROLL_LEFT;
            break;
        case VK_RIGHT:
            action.Code = ACT_ROLL_RIGHT;
            break;
        case VK_SPACE:
        case 'M':
            action.Code = ACT_ASCEND;
            break;
        case VK_CONTROL:
        case 'N':
            action.Code = ACT_DESCEND;
            break;
        }
        prev_stat = lParam & 0x40000000;
        if (msg == WM_KEYDOWN) {
            action.Actived = 1;
            if (!prev_stat) {
                action.Started = 1;
            }
        }
        else if (msg == WM_KEYUP) {
            action.Stoped = 1;
            action.Actived = 0;
        }
        Input->SetAction(action);
        break;

    default:
        break;
    }
    return 0;
}
