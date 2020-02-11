
#pragma once

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <windowsx.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <string>
#include <math.h>

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
typedef size_t SIZEB;

typedef float R32;
typedef double R64;


#if _DEBUG
#include <stdio.h>
#include <assert.h>
#define DEBUG(str, ...) printf(str ## "\n", ## __VA_ARGS__)
#define ASSERT(x) assert(x)
#else
#define DEBUG(str, ...)
#define ASSERT(x)
#endif

#define CONST_PI                3.141592653589793238462643383279502884197169399375
#define CONST_PI_HALF           1.57079632679489661923   // pi/2
#define CONST_PI_QUARTER        0.785398163397448309616 // pi/4
#define CONST_2_PI              6.283185307 // 2 * pi
#define CONST_TOLERANCE         0.0001     // 
#define EPSILON                 0.0000001 // 
#define R_E                     2.71828182845904523536   // e
#define ToRads(deg) ((deg) * (static_cast<R32>(CONST_PI) / 180.0f))
