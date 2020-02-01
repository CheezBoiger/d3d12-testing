
#include "KeyboardInput.h"


namespace jcl {


UINT kKeyboardMap[128];

void Keyboard::registerInput(UINT vk, InputStatus event)
{
    kKeyboardMap[vk] = event;
}

bool Keyboard::isKeyDown(UINT vk)
{
    return kKeyboardMap[vk] == INPUT_STATUS_DOWN;
}

bool Keyboard::isKeyUp(UINT vk)
{
    return kKeyboardMap[vk] == INPUT_STATUS_UP;
}
} // jcl