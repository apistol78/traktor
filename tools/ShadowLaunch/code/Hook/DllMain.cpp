#include <Windows.h>
#include <detours.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Path.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/TString.h>

using namespace traktor;

// {0C8ACD8F-9C7E-4076-B859-CFC0CA4C548C}
static const GUID c_guidShadowPayload = { 0xc8acd8f, 0x9c7e, 0x4076, { 0xb8, 0x59, 0xcf, 0xc0, 0xca, 0x4c, 0x54, 0x8c } };

#pragma pack(1)
struct ShadowPayload
{
	wchar_t sourcePath[MAX_PATH];
	wchar_t sandboxPath[MAX_PATH];
};
#pragma pack()

static ShadowPayload* g_shadowPayload = 0;
static HMODULE (WINAPI *s_LoadLibraryA)(LPCSTR lpLibFileName) = LoadLibraryA;
static HMODULE (WINAPI *s_LoadLibraryW)(LPCWSTR lpLibFileName) = LoadLibraryW;

static BOOL CALLBACK ListFileCallback(PVOID pContext, PCHAR pszOrigFile, PCHAR pszFile, PCHAR *ppszOutFile)
{  
	std::set< std::wstring >& dlls = *(std::set< std::wstring >*)pContext;
	dlls.insert(mbstows(pszFile));
	return TRUE;   
}   

bool shadowCopy(const Path& sourceFile)
{
	Path shadowPath = Path(g_shadowPayload->sandboxPath);
	Path shadowFile = shadowPath + sourceFile.getFileName();

	if (FileSystem::getInstance().get(shadowFile) != 0)
		return true;

	FileSystem::getInstance().makeAllDirectories(shadowPath);

	HANDLE hSourceFile = CreateFile(
		sourceFile.getPathName().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hSourceFile == INVALID_HANDLE_VALUE)
		return false;

	HANDLE hShadowFile = CreateFile(
		shadowFile.getPathName().c_str(),
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if (hShadowFile == INVALID_HANDLE_VALUE)
		return false;

	PDETOUR_BINARY pBinary = DetourBinaryOpen(hSourceFile);
	if (!pBinary)
	{
		CloseHandle(hSourceFile);
		return false;
	}

	// Scan all import DLLs from binary.
	std::set< std::wstring > dlls;
	DetourBinaryEditImports(pBinary, &dlls, NULL, ListFileCallback, NULL, NULL);

	// Append our payload to binary.
	ShadowPayload pl = { 0 };
	wcscpy(pl.sourcePath, sourceFile.getPathOnly().c_str());
	wcscpy(pl.sandboxPath, shadowPath.getPathName().c_str());
	DetourBinarySetPayload(pBinary, c_guidShadowPayload, &pl, sizeof(pl));

	// Write our patch binary.
	DetourBinaryWrite(pBinary, hShadowFile);

	DetourBinaryClose(pBinary);

	CloseHandle(hShadowFile);
	CloseHandle(hSourceFile);

	for (std::set< std::wstring >::const_iterator i = dlls.begin(); i != dlls.end(); ++i)
	{
		if (FileSystem::getInstance().get(sourceFile.getPathOnly() + L"/" + *i) != 0)
			shadowCopy(sourceFile.getPathOnly() + L"/" + *i);
	}

	return true;
}

HMODULE WINAPI HookLoadLibraryA(LPCSTR lpLibFileName)
{
	Path shadowFile = Path(g_shadowPayload->sandboxPath) + Path(mbstows(lpLibFileName));
	if (FileSystem::getInstance().exist(shadowFile))
	{
		// Already sandboxes; load from sandbox.
		return (*s_LoadLibraryW)(shadowFile.getPathName().c_str());
	}

	Path sourceFile = Path(g_shadowPayload->sourcePath) + Path(mbstows(lpLibFileName));
	if (FileSystem::getInstance().exist(sourceFile))
	{
		// File exist is source, need to create sandboxed copy.
		if (!shadowCopy(sourceFile))
			return NULL;

		// Copy successful; load from sandbox.
		return (*s_LoadLibraryW)(shadowFile.getPathName().c_str());
	}

	// Do not exist in source folder, assume external.
	return (*s_LoadLibraryA)(lpLibFileName);
}

HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
	Path shadowFile = Path(g_shadowPayload->sandboxPath) + Path(lpLibFileName);
	if (FileSystem::getInstance().exist(shadowFile))
	{
		// Already sandboxes; load from sandbox.
		return (*s_LoadLibraryW)(shadowFile.getPathName().c_str());
	}

	Path sourceFile = Path(g_shadowPayload->sourcePath) + Path(lpLibFileName);
	if (FileSystem::getInstance().exist(sourceFile))
	{
		// File exist is source, need to create sandboxed copy.
		if (!shadowCopy(sourceFile))
			return NULL;

		// Copy successful; load from sandbox.
		return (*s_LoadLibraryW)(shadowFile.getPathName().c_str());
	}

	// Do not exist in source folder, assume external.
	return (*s_LoadLibraryW)(lpLibFileName);
}

__declspec(dllexport)
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			DWORD pl = 0;
			g_shadowPayload = (ShadowPayload*)DetourFindPayload(NULL, c_guidShadowPayload, &pl);
			if (!g_shadowPayload)
				return FALSE;

			DisableThreadLibraryCalls(hDLL);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)s_LoadLibraryA, HookLoadLibraryA);
			DetourAttach(&(PVOID&)s_LoadLibraryW, HookLoadLibraryW);
			DetourTransactionCommit();
		}
		break;
	}
	return TRUE;
}
