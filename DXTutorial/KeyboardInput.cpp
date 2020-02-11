
#include "KeyboardInput.h"


namespace jcl {


UINT kKeyboardMap[128];

void Keyboard::registerInput(KeyCode vk, InputStatus event)
{
    kKeyboardMap[vk] = event;
}

bool Keyboard::isKeyDown(KeyCode vk)
{
    return kKeyboardMap[vk] == INPUT_STATUS_DOWN;
}

bool Keyboard::isKeyUp(KeyCode vk)
{
    return kKeyboardMap[vk] == INPUT_STATUS_UP;
}
} // jcl