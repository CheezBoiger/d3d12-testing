#pragma once
#include "WinConfigs.h"

namespace jcl {


enum InputStatus {
    INPUT_STATUS_UP,
    INPUT_STATUS_DOWN,
    INPUT_STATUS_STILLDOWN,
    INPUT_STATUS_IDLE
};

class Keyboard {
public:
    static bool isKeyDown(UINT vk);
    static bool isKeyUp(UINT vk);
    static bool isKeyStillDown(UINT vk);
    static void registerInput(UINT vk, InputStatus event);
};
} // jcl