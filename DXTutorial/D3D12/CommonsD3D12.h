//
#pragma once


#include "../WinConfigs.h"

#include <d3d12.h>
#include <d3d12shader.h>

// On machines that do not have windows insider preview, this will break!
#include <DirectML.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectML.lib")

#if _DEBUG
#define DX12ASSERT(x) assert(!FAILED(x))
#else
#define DX12ASSERT(x) x
#endif