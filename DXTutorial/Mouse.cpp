//
#include "Mouse.h"

namespace jcl {

R32 Mouse::kLastXPos = 0.0f;
R32 Mouse::kLastYPos = 0.0f;
R32 Mouse::kXPos = 0.0f;
R32 Mouse::kYPos = 0.0f;

void Mouse::inputMousePos(I32 x, I32 y)
{
    if (Mouse::kXPos == x && Mouse::kYPos) return;
    Mouse::kXPos = (R32)x;
    Mouse::kYPos = (R32)y;
    
}
}