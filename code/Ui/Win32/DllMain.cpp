/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if !defined(T_STATIC)

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>

extern HINSTANCE g_hInstance;

BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID)
{
	BOOL bResult = TRUE;

	if (dwReason == DLL_PROCESS_ATTACH)
		g_hInstance = (HINSTANCE)hInstance;

	return bResult;
}

#endif
