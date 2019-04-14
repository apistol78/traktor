#pragma once

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#if defined(T_USE_D3DPERF)
#	include <d3d9.h>
#endif
#include <d3d11.h>
#include <d3dcompiler.h>
#include <tchar.h>

