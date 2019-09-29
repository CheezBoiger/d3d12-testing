#pragma once

#include <d3d11.h>
#include <d3d11_4.h>


#pragma comment(lib, "d3d11.lib")

#if _DEBUG
#include <cassert>
#include <stdio.h>
#define DX11ASSERT(x) assert(!FAILED(x))
#else
#define DX11ASSERT(x) x
#endif