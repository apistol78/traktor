/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_WIN32) && !defined(T_STATIC)

#include <wx/wx.h>
#include <wx/msw/private.h>

BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID)
{
	BOOL bResult = TRUE;

	if (dwReason == DLL_PROCESS_ATTACH)
		wxSetInstance((HINSTANCE)hInstance);

	return bResult;
}

#endif
