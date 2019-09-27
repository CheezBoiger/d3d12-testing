
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <string>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

typedef unsigned char U8;
typedef char I8;
typedef unsigned short U16;
typedef short I16;
typedef unsigned U32;
typedef int I32;
typedef unsigned long long U64;
typedef long long I64;

typedef I32 B32;
typedef I8 B8;

typedef float R32;
typedef double R64;


#if _DEBUG
#include <stdio.h>
#define DEBUG(str, ...) printf(str ## "\n", ## __VA_ARGS__)
#else
#define DEBUG(str, ...)
#endif
