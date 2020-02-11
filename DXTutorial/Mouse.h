//
#pragma once

#include "WinConfigs.h"

namespace jcl {


class Mouse {
public:
    static void inputMousePos(I32 x, I32 y);
    static R32 kLastXPos;
    static R32 kLastYPos;
    static R32 kXPos;
    static R32 kYPos;
};
}
