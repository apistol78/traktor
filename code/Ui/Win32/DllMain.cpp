/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
